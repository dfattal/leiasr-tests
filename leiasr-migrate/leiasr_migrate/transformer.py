"""
Code transformer for migrating legacy Display API to modern IDisplayManager
"""

import re
import os
from pathlib import Path
from typing import List, Dict, Tuple
from dataclasses import dataclass
import yaml


@dataclass
class Transformation:
    """Represents a code transformation"""
    file_path: str
    line_number: int
    original_text: str
    new_text: str
    transformation_type: str
    description: str


class CodeTransformer:
    """Transforms legacy Display API code to modern patterns"""

    def __init__(self, patterns_file: str = None, helper_path: str = "../common"):
        if patterns_file is None:
            patterns_file = Path(__file__).parent / 'patterns' / 'display_patterns.yaml'

        with open(patterns_file, 'r') as f:
            self.patterns = yaml.safe_load(f)

        self.helper_path = helper_path
        self.transformations = []

    def transform_file(self, file_path: str, dry_run: bool = False) -> List[Transformation]:
        """Transform a single file"""
        try:
            with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
                original_content = f.read()
        except Exception as e:
            print(f"Error reading {file_path}: {e}")
            return []

        modified_content = original_content
        file_transformations = []

        # Apply transformations in priority order
        transformations_config = self.patterns.get('transformations', {})
        sorted_transforms = sorted(
            transformations_config.items(),
            key=lambda x: x[1].get('priority', 999)
        )

        for transform_name, transform_config in sorted_transforms:
            if transform_name == 'add_lazy_binding':
                modified_content, transforms = self._add_lazy_binding(
                    file_path, modified_content
                )
                file_transformations.extend(transforms)

            elif transform_name == 'add_helper_include':
                modified_content, transforms = self._add_helper_include(
                    file_path, modified_content
                )
                file_transformations.extend(transforms)

            elif transform_name == 'replace_display_create':
                modified_content, transforms = self._replace_display_create(
                    file_path, modified_content, transform_config
                )
                file_transformations.extend(transforms)

            elif transform_name == 'replace_pointer_arrow':
                modified_content, transforms = self._replace_pointer_to_direct(
                    file_path, modified_content, transform_config
                )
                file_transformations.extend(transforms)

        # Write modified content if not dry run
        if not dry_run and file_transformations:
            # Rename original to .legacy
            legacy_path = file_path + '.legacy'
            import shutil
            shutil.move(file_path, legacy_path)

            # Write migrated file in original location
            with open(file_path, 'w', encoding='utf-8') as f:
                f.write(modified_content)

            print(f"  Migrated {file_path} ({len(file_transformations)} changes)")
            print(f"  Original saved to {legacy_path}")

        self.transformations.extend(file_transformations)
        return file_transformations

    def _add_lazy_binding(self, file_path: str, content: str) -> Tuple[str, List[Transformation]]:
        """Add SRDISPLAY_LAZYBINDING define before display.h include"""
        transforms = []

        # Check if already has lazy binding
        if re.search(r'#define\s+SRDISPLAY_LAZYBINDING', content):
            return content, transforms

        # Find display.h include
        pattern = r'(#include\s+"sr/world/display/display\.h")'
        match = re.search(pattern, content)

        if match:
            original = match.group(0)
            replacement = f'#define SRDISPLAY_LAZYBINDING  // Enable modern DisplayManager with fallback\n{original}'

            modified_content = content[:match.start()] + replacement + content[match.end():]

            line_num = content[:match.start()].count('\n') + 1
            transform = Transformation(
                file_path=file_path,
                line_number=line_num,
                original_text=original,
                new_text=replacement,
                transformation_type='add_lazy_binding',
                description='Added SRDISPLAY_LAZYBINDING define'
            )
            transforms.append(transform)

            return modified_content, transforms

        return content, transforms

    def _add_helper_include(self, file_path: str, content: str) -> Tuple[str, List[Transformation]]:
        """Add display_helper.h include after display.h"""
        transforms = []

        # Check if already has helper include
        if 'display_helper.h' in content:
            return content, transforms

        # Find display.h include (it should now have SRDISPLAY_LAZYBINDING before it)
        pattern = r'(#include\s+"sr/world/display/display\.h"\s*\n)'
        match = re.search(pattern, content)

        if match:
            helper_include = f'#include "{self.helper_path}/display_helper.h"\n'
            insertion_point = match.end()

            modified_content = content[:insertion_point] + helper_include + content[insertion_point:]

            line_num = content[:match.start()].count('\n') + 1
            transform = Transformation(
                file_path=file_path,
                line_number=line_num + 1,
                original_text='',
                new_text=helper_include.strip(),
                transformation_type='add_helper_include',
                description=f'Added helper include: {helper_include.strip()}'
            )
            transforms.append(transform)

            return modified_content, transforms

        return content, transforms

    def _replace_display_create(self, file_path: str, content: str,
                                config: Dict) -> Tuple[str, List[Transformation]]:
        """Replace SR::Display::create() with DisplayAccess"""
        transforms = []

        pattern = config['pattern']
        replacement_template = config['replacement']

        modified_content = content
        offset = 0  # Track offset due to previous replacements

        for match in re.finditer(pattern, content):
            var_name = match.group(1) if match.lastindex >= 1 else 'display'
            context_arg = match.group(2) if match.lastindex >= 2 else 'context'

            # Generate replacement
            replacement = re.sub(r'\\1', var_name, replacement_template)
            replacement = re.sub(r'\\2', context_arg, replacement)

            # Apply replacement with offset
            start = match.start() + offset
            end = match.end() + offset
            modified_content = modified_content[:start] + replacement + modified_content[end:]

            # Update offset
            offset += len(replacement) - (match.end() - match.start())

            line_num = content[:match.start()].count('\n') + 1
            transform = Transformation(
                file_path=file_path,
                line_number=line_num,
                original_text=match.group(0),
                new_text=replacement,
                transformation_type='replace_display_create',
                description=f'Converted Display* {var_name} to DisplayAccess'
            )
            transforms.append(transform)

        return modified_content, transforms

    def _replace_pointer_to_direct(self, file_path: str, content: str,
                                   config: Dict) -> Tuple[str, List[Transformation]]:
        """Replace -> with . for DisplayAccess variables"""
        transforms = []

        # First, find all DisplayAccess variables
        display_access_vars = set()
        for match in re.finditer(r'SR::Helper::DisplayAccess\s+(\w+)', content):
            display_access_vars.add(match.group(1))

        if not display_access_vars:
            return content, transforms

        # Replace -> with . for these variables
        modified_content = content
        offset = 0

        # Build pattern for known DisplayAccess variables
        var_pattern = r'(' + '|'.join(display_access_vars) + r')->'

        for match in re.finditer(var_pattern, content):
            var_name = match.group(1)
            replacement = f'{var_name}.'

            start = match.start() + offset
            end = match.end() + offset
            modified_content = modified_content[:start] + replacement + modified_content[end:]

            offset += len(replacement) - (match.end() - match.start())

            line_num = content[:match.start()].count('\n') + 1
            transform = Transformation(
                file_path=file_path,
                line_number=line_num,
                original_text=f'{var_name}->',
                new_text=replacement,
                transformation_type='replace_pointer_arrow',
                description=f'Changed {var_name}-> to {var_name}.'
            )
            transforms.append(transform)

        return modified_content, transforms

    def transform_directory(self, directory: str, file_list: List[str] = None,
                           dry_run: bool = False) -> List[Transformation]:
        """Transform all files in a directory or specific files"""
        if file_list:
            files = [Path(f) for f in file_list]
        else:
            directory = Path(directory)
            files = list(directory.glob('**/*.cpp')) + list(directory.glob('**/*.h'))

        print(f"Transforming {len(files)} files...")
        if dry_run:
            print("(DRY RUN - no files will be modified)")

        all_transformations = []
        for file_path in files:
            transformations = self.transform_file(str(file_path), dry_run)
            all_transformations.extend(transformations)

        return all_transformations

    def generate_diff_report(self, output_file: str = None) -> str:
        """Generate a diff-style report of all transformations"""
        report_lines = []
        report_lines.append("=" * 80)
        report_lines.append("LeiaSR Display API Migration Transformations")
        report_lines.append("=" * 80)
        report_lines.append("")

        # Group by file
        by_file = {}
        for transform in self.transformations:
            if transform.file_path not in by_file:
                by_file[transform.file_path] = []
            by_file[transform.file_path].append(transform)

        report_lines.append(f"Files modified: {len(by_file)}")
        report_lines.append(f"Total transformations: {len(self.transformations)}")
        report_lines.append("")

        # Detailed transformations
        for file_path, transforms in sorted(by_file.items()):
            report_lines.append("-" * 80)
            report_lines.append(f"File: {file_path}")
            report_lines.append(f"  Changes: {len(transforms)}")
            report_lines.append("")

            for transform in sorted(transforms, key=lambda t: t.line_number):
                report_lines.append(f"  Line {transform.line_number}: {transform.description}")
                report_lines.append(f"    - {transform.original_text}")
                report_lines.append(f"    + {transform.new_text}")
                report_lines.append("")

        report = '\n'.join(report_lines)

        if output_file:
            with open(output_file, 'w') as f:
                f.write(report)
            print(f"\nTransformation report saved to: {output_file}")

        return report


if __name__ == '__main__':
    # Simple test
    import sys

    if len(sys.argv) < 2:
        print("Usage: python transformer.py <file_or_directory> [--dry-run]")
        sys.exit(1)

    dry_run = '--dry-run' in sys.argv
    path = sys.argv[1]

    transformer = CodeTransformer()

    if os.path.isfile(path):
        transformer.transform_file(path, dry_run=dry_run)
    else:
        transformer.transform_directory(path, dry_run=dry_run)

    print(transformer.generate_diff_report())
