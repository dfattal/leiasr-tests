"""
Code transformer for migrating legacy Display API to modern IDisplayManager
"""

import re
import os
from pathlib import Path
from typing import List, Dict, Tuple, Set
from dataclasses import dataclass
import yaml

# Embedded display_helper.h content
DISPLAY_HELPER_H = '''/*!
 * Copyright (C) 2025 Leia, Inc.
 *
 * Modern Display Manager Helper
 * Provides version-safe display access with automatic fallback to legacy Display class
 */

#pragma once

#define SRDISPLAY_LAZYBINDING  // Enable lazy binding for version compatibility
#include "sr/world/display/display.h"
#include <iostream>

namespace SR {
namespace Helper {

/**
 * \\brief Wrapper class that provides unified display access with version fallback
 *
 * This class automatically uses IDisplayManager when available (SDK 1.34.8+)
 * and falls back to the legacy Display class for older runtimes.
 */
class DisplayAccess
{
private:
    SRContext& m_context;
    IDisplayManager* m_displayManager;
    IDisplay* m_modernDisplay;
    Display* m_legacyDisplay;
    bool m_usingModernAPI;

public:
    DisplayAccess(SRContext& context)
        : m_context(context)
        , m_displayManager(nullptr)
        , m_modernDisplay(nullptr)
        , m_legacyDisplay(nullptr)
        , m_usingModernAPI(false)
    {
        // Try to get modern DisplayManager first
        m_displayManager = TryGetDisplayManagerInstance(context);
        if (m_displayManager) {
            m_modernDisplay = m_displayManager->getPrimaryActiveSRDisplay();
            m_usingModernAPI = true;
            std::cout << "[DisplayAccess] Using modern IDisplayManager API" << std::endl;
        } else {
            // Fallback to legacy Display
            m_legacyDisplay = Display::create(context);
            m_usingModernAPI = false;
            std::cout << "[DisplayAccess] Falling back to legacy Display API" << std::endl;
        }
    }

    ~DisplayAccess() {
        // Context owns IDisplayManager and IDisplay, no cleanup needed
        // Display is also context-owned, no cleanup needed
    }

    /**
     * \\brief Check if display is valid and available
     *
     * For modern API: checks IDisplay::isValid()
     * For legacy API: checks if display parameters are non-zero
     */
    bool isDisplayValid() {
        if (m_usingModernAPI) {
            return m_modernDisplay && m_modernDisplay->isValid();
        } else {
            // Legacy API doesn't have isValid(), check if we got meaningful data
            if (!m_legacyDisplay) return false;
            SR_recti loc = m_legacyDisplay->getLocation();
            int64_t width = loc.right - loc.left;
            int64_t height = loc.bottom - loc.top;
            return (width != 0) && (height != 0);
        }
    }

    /**
     * \\brief Get display resolution height
     */
    int getResolutionHeight() {
        if (m_usingModernAPI) {
            return m_modernDisplay ? m_modernDisplay->getResolutionHeight() : 1440;
        } else {
            return m_legacyDisplay ? m_legacyDisplay->getResolutionHeight() : 1440;
        }
    }

    /**
     * \\brief Get display resolution width
     */
    int getResolutionWidth() {
        if (m_usingModernAPI) {
            return m_modernDisplay ? m_modernDisplay->getResolutionWidth() : 2560;
        } else {
            return m_legacyDisplay ? m_legacyDisplay->getResolutionWidth() : 2560;
        }
    }

    /**
     * \\brief Get physical resolution height
     */
    int getPhysicalResolutionHeight() {
        if (m_usingModernAPI) {
            return m_modernDisplay ? m_modernDisplay->getPhysicalResolutionHeight() : 1440;
        } else {
            return m_legacyDisplay ? m_legacyDisplay->getPhysicalResolutionHeight() : 1440;
        }
    }

    /**
     * \\brief Get physical resolution width
     */
    int getPhysicalResolutionWidth() {
        if (m_usingModernAPI) {
            return m_modernDisplay ? m_modernDisplay->getPhysicalResolutionWidth() : 2560;
        } else {
            return m_legacyDisplay ? m_legacyDisplay->getPhysicalResolutionWidth() : 2560;
        }
    }

    /**
     * \\brief Get physical size height in cm
     */
    float getPhysicalSizeHeight() {
        if (m_usingModernAPI) {
            return m_modernDisplay ? m_modernDisplay->getPhysicalSizeHeight() : 40.0f;
        } else {
            return m_legacyDisplay ? m_legacyDisplay->getPhysicalSizeHeight() : 40.0f;
        }
    }

    /**
     * \\brief Get physical size width in cm
     */
    float getPhysicalSizeWidth() {
        if (m_usingModernAPI) {
            return m_modernDisplay ? m_modernDisplay->getPhysicalSizeWidth() : 70.0f;
        } else {
            return m_legacyDisplay ? m_legacyDisplay->getPhysicalSizeWidth() : 70.0f;
        }
    }

    /**
     * \\brief Get dot pitch in cm
     */
    float getDotPitch() {
        if (m_usingModernAPI) {
            return m_modernDisplay ? m_modernDisplay->getDotPitch() : 0.0233f;
        } else {
            return m_legacyDisplay ? m_legacyDisplay->getDotPitch() : 0.0233f;
        }
    }

    /**
     * \\brief Get display location in virtual screen coordinates
     */
    SR_recti getLocation() {
        SR_recti defaultLoc = {0, 0, 2560, 1440};
        if (m_usingModernAPI) {
            return m_modernDisplay ? m_modernDisplay->getLocation() : defaultLoc;
        } else {
            return m_legacyDisplay ? m_legacyDisplay->getLocation() : defaultLoc;
        }
    }

    /**
     * \\brief Get recommended views texture width
     */
    int getRecommendedViewsTextureWidth() {
        if (m_usingModernAPI) {
            return m_modernDisplay ? m_modernDisplay->getRecommendedViewsTextureWidth() : 2560;
        } else {
            return m_legacyDisplay ? m_legacyDisplay->getRecommendedViewsTextureWidth() : 2560;
        }
    }

    /**
     * \\brief Get recommended views texture height
     */
    int getRecommendedViewsTextureHeight() {
        if (m_usingModernAPI) {
            return m_modernDisplay ? m_modernDisplay->getRecommendedViewsTextureHeight() : 1440;
        } else {
            return m_legacyDisplay ? m_legacyDisplay->getRecommendedViewsTextureHeight() : 1440;
        }
    }

    /**
     * \\brief Get default viewing position (only available in modern API)
     *
     * \\return true if modern API and successfully retrieved, false for legacy API
     */
    bool getDefaultViewingPosition(float& x_mm, float& y_mm, float& z_mm) {
        if (m_usingModernAPI && m_modernDisplay) {
            m_modernDisplay->getDefaultViewingPosition(x_mm, y_mm, z_mm);
            return true;
        } else {
            // Legacy API doesn't support this, provide sensible defaults
            x_mm = 0.0f;
            y_mm = 0.0f;
            z_mm = 600.0f; // Default viewing distance
            return false;
        }
    }

    /**
     * \\brief Check if using modern DisplayManager API
     */
    bool isUsingModernAPI() const { return m_usingModernAPI; }

    /**
     * \\brief Get unique display identifier (only modern API)
     */
    uint64_t getIdentifier() {
        if (m_usingModernAPI && m_modernDisplay) {
            return m_modernDisplay->identifier();
        }
        return 0; // Legacy API doesn't have identifiers
    }

    /**
     * \\brief Wait for display to become ready
     *
     * Polls display validity with timeout
     * \\param maxWaitSeconds Maximum time to wait in seconds
     * \\return true if display became ready, false if timeout
     */
    bool waitForDisplay(double maxWaitSeconds = 5.0) {
        double startTime = (double)GetTickCount64() / 1000.0;

        while (true) {
            if (isDisplayValid()) {
                std::cout << "[DisplayAccess] Display is ready" << std::endl;
                return true;
            }

            double currentTime = (double)GetTickCount64() / 1000.0;
            if ((currentTime - startTime) > maxWaitSeconds) {
                std::cout << "[DisplayAccess] Display wait timeout" << std::endl;
                return false;
            }

            std::cout << "[DisplayAccess] Waiting for display..." << std::endl;
            Sleep(100);
        }
    }
};

} // namespace Helper
} // namespace SR
'''


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

    def __init__(self, patterns_file: str = None, legacy_fallback: bool = False):
        if patterns_file is None:
            patterns_file = Path(__file__).parent / 'patterns' / 'display_patterns.yaml'

        with open(patterns_file, 'r') as f:
            self.patterns = yaml.safe_load(f)

        self.transformations = []
        self.legacy_fallback = legacy_fallback

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

        # Apply transformations based on migration mode
        transformations_config = self.patterns.get('transformations', {})
        sorted_transforms = sorted(
            transformations_config.items(),
            key=lambda x: x[1].get('priority', 999)
        )

        if self.legacy_fallback:
            # Legacy fallback mode: Use DisplayAccess helper with runtime fallback
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

                    # After converting pointers to objects, fix nullptr checks
                    modified_content, transforms = self._replace_nullptr_checks(
                        file_path, modified_content, transform_config
                    )
                    file_transformations.extend(transforms)
        else:
            # Modern mode: Direct IDisplayManager usage
            for transform_name, transform_config in sorted_transforms:
                if transform_name == 'replace_display_create':
                    # Use modern transformation
                    modified_content, transforms = self._replace_display_create_modern(
                        file_path, modified_content, transform_config
                    )
                    file_transformations.extend(transforms)
                # Skip lazy binding, helper include, and pointer arrow transformations

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
            helper_include = f'#include "display_helper.h"\n'
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

    def _replace_nullptr_checks(self, file_path: str, content: str,
                                config: Dict) -> Tuple[str, List[Transformation]]:
        """Replace nullptr checks with isDisplayValid() for DisplayAccess variables"""
        transforms = []

        # First, find all DisplayAccess variables
        display_access_vars = set()
        for match in re.finditer(r'SR::Helper::DisplayAccess\s+(\w+)', content):
            display_access_vars.add(match.group(1))

        if not display_access_vars:
            return content, transforms

        modified_content = content
        offset = 0

        # Build pattern for nullptr checks: var != nullptr or var == nullptr
        for var_name in display_access_vars:
            # Pattern for != nullptr
            pattern_ne = rf'{var_name}\s*!=\s*nullptr'
            for match in re.finditer(pattern_ne, content):
                replacement = f'{var_name}.isDisplayValid()'

                start = match.start() + offset
                end = match.end() + offset
                modified_content = modified_content[:start] + replacement + modified_content[end:]

                offset += len(replacement) - (match.end() - match.start())

                line_num = content[:match.start()].count('\n') + 1
                transform = Transformation(
                    file_path=file_path,
                    line_number=line_num,
                    original_text=match.group(0),
                    new_text=replacement,
                    transformation_type='replace_nullptr_check',
                    description=f'Changed {var_name} != nullptr to {var_name}.isDisplayValid()'
                )
                transforms.append(transform)

            # Pattern for == nullptr
            pattern_eq = rf'{var_name}\s*==\s*nullptr'
            for match in re.finditer(pattern_eq, modified_content):
                replacement = f'!{var_name}.isDisplayValid()'

                # Recalculate positions in modified content
                start = match.start()
                end = match.end()
                modified_content = modified_content[:start] + replacement + modified_content[end:]

                line_num = content[:match.start()].count('\n') + 1
                transform = Transformation(
                    file_path=file_path,
                    line_number=line_num,
                    original_text=f'{var_name} == nullptr',
                    new_text=replacement,
                    transformation_type='replace_nullptr_check',
                    description=f'Changed {var_name} == nullptr to !{var_name}.isDisplayValid()'
                )
                transforms.append(transform)

        return modified_content, transforms

    def _replace_display_create_modern(self, file_path: str, content: str,
                                       config: Dict) -> Tuple[str, List[Transformation]]:
        """Replace SR::Display::create() with direct IDisplayManager usage (modern mode)"""
        transforms = []

        # Pattern to match: SR::Display* varName = SR::Display::create(context);
        pattern = r'SR::Display\s*\*\s*(\w+)\s*=\s*SR::Display::create\s*\(([^)]+)\)\s*;?'

        modified_content = content
        offset = 0

        for match in re.finditer(pattern, content):
            var_name = match.group(1)
            context_arg = match.group(2).strip()

            # Generate modern replacement
            # Creates two lines:
            # 1. SR::IDisplayManager* displayMgr_varName = SR::GetDisplayManagerInstance(context);
            # 2. SR::IDisplay* varName = displayMgr_varName->getPrimaryActiveSRDisplay();
            mgr_var_name = f'displayMgr_{var_name}'
            replacement = f'SR::IDisplayManager* {mgr_var_name} = SR::GetDisplayManagerInstance({context_arg});\n    SR::IDisplay* {var_name} = {mgr_var_name}->getPrimaryActiveSRDisplay();'

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
                transformation_type='replace_display_create_modern',
                description=f'Converted Display::create() to IDisplayManager (modern)'
            )
            transforms.append(transform)

        return modified_content, transforms

    def _create_helper_file(self, directory: Path, dry_run: bool = False) -> bool:
        """Create display_helper.h if it doesn't exist"""
        helper_file = directory / 'display_helper.h'

        if helper_file.exists():
            print(f"  display_helper.h already exists at {helper_file}")
            return False

        if not dry_run:
            helper_file.write_text(DISPLAY_HELPER_H, encoding='utf-8')
            print(f"  Created display_helper.h at {helper_file}")
        else:
            print(f"  Would create display_helper.h at {helper_file}")

        return True

    def transform_directory(self, directory: str, file_list: List[str] = None,
                           dry_run: bool = False) -> List[Transformation]:
        """Transform all files in a directory or specific files"""
        if file_list:
            files = [Path(f) for f in file_list]
        else:
            directory = Path(directory)
            files = list(directory.glob('**/*.cpp')) + list(directory.glob('**/*.h'))

        # Create display_helper.h only in legacy fallback mode
        directory_path = Path(directory) if isinstance(directory, str) else directory
        if self.legacy_fallback:
            self._create_helper_file(directory_path, dry_run)
        else:
            if not dry_run:
                print("  Skipping display_helper.h creation (modern mode)")
            else:
                print("  Would skip display_helper.h creation (modern mode)")

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
