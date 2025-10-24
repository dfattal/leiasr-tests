"""
Code analyzer for detecting legacy Display API usage
"""

import re
import os
from pathlib import Path
from typing import List, Dict, Tuple
from dataclasses import dataclass
import yaml


@dataclass
class Detection:
    """Represents a detected pattern in code"""
    file_path: str
    line_number: int
    pattern_name: str
    matched_text: str
    context_before: str
    context_after: str
    confidence: str  # 'high', 'medium', 'low'
    capture_groups: Dict[str, str] = None


class CodeAnalyzer:
    """Analyzes C++ code for legacy Display API patterns"""

    def __init__(self, patterns_file: str = None):
        if patterns_file is None:
            patterns_file = Path(__file__).parent / 'patterns' / 'display_patterns.yaml'

        with open(patterns_file, 'r') as f:
            self.patterns = yaml.safe_load(f)

        self.detections = []

    def analyze_file(self, file_path: str) -> List[Detection]:
        """Analyze a single C++ file"""
        if not file_path.endswith(('.cpp', '.h', '.hpp', '.cc')):
            return []

        # Skip helper files - don't try to migrate the migration helper itself!
        if 'display_helper.h' in file_path:
            return []

        try:
            with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
                content = f.read()
                lines = content.splitlines()
        except Exception as e:
            print(f"Error reading {file_path}: {e}")
            return []

        file_detections = []

        # Check each detection pattern
        for pattern_name, pattern_config in self.patterns['detection_patterns'].items():
            pattern_type = pattern_config.get('type', 'regex')

            if pattern_type == 'regex':
                detections = self._detect_regex(
                    file_path, content, lines,
                    pattern_name, pattern_config
                )
                file_detections.extend(detections)

            elif pattern_type == 'regex_list':
                for sub_pattern in pattern_config['patterns']:
                    sub_config = {
                        'pattern': sub_pattern,
                        'description': pattern_config['description']
                    }
                    detections = self._detect_regex(
                        file_path, content, lines,
                        f"{pattern_name}_{sub_pattern[:20]}", sub_config
                    )
                    file_detections.extend(detections)

        # Determine confidence levels
        for detection in file_detections:
            detection.confidence = self._get_confidence(detection.pattern_name)

        self.detections.extend(file_detections)
        return file_detections

    def _detect_regex(self, file_path: str, content: str, lines: List[str],
                      pattern_name: str, pattern_config: Dict) -> List[Detection]:
        """Detect pattern using regex"""
        pattern = pattern_config['pattern']
        multiline = pattern_config.get('multiline', False)
        flags = re.MULTILINE | re.DOTALL if multiline else re.MULTILINE

        detections = []

        try:
            for match in re.finditer(pattern, content, flags):
                # Find line number
                line_num = content[:match.start()].count('\n') + 1

                # Extract context
                context_start = max(0, line_num - 3)
                context_end = min(len(lines), line_num + 3)
                context_before = '\n'.join(lines[context_start:line_num-1])
                context_after = '\n'.join(lines[line_num:context_end])

                # Extract capture groups if specified
                capture_groups = {}
                if 'capture_groups' in pattern_config:
                    for i, group_name in enumerate(pattern_config['capture_groups'], 1):
                        if i <= len(match.groups()):
                            capture_groups[group_name] = match.group(i)

                detection = Detection(
                    file_path=file_path,
                    line_number=line_num,
                    pattern_name=pattern_name,
                    matched_text=match.group(0),
                    context_before=context_before,
                    context_after=context_after,
                    confidence='medium',
                    capture_groups=capture_groups if capture_groups else None
                )
                detections.append(detection)

        except re.error as e:
            print(f"Regex error in pattern '{pattern_name}': {e}")

        return detections

    def _get_confidence(self, pattern_name: str) -> str:
        """Determine confidence level for a pattern"""
        risk_levels = self.patterns.get('risk_levels', {})

        if pattern_name in risk_levels.get('high_confidence', []):
            return 'high'
        elif pattern_name in risk_levels.get('medium_confidence', []):
            return 'medium'
        elif pattern_name in risk_levels.get('needs_review', []):
            return 'low'
        else:
            return 'medium'

    def analyze_directory(self, directory: str, recursive: bool = True) -> List[Detection]:
        """Analyze all C++ files in a directory"""
        directory = Path(directory)

        if not directory.exists():
            raise FileNotFoundError(f"Directory not found: {directory}")

        # Find all C++ files
        patterns = ['**/*.cpp', '**/*.h', '**/*.hpp', '**/*.cc'] if recursive else \
                   ['*.cpp', '*.h', '*.hpp', '*.cc']

        files = []
        for pattern in patterns:
            files.extend(directory.glob(pattern))

        print(f"Analyzing {len(files)} files in {directory}...")

        all_detections = []
        for file_path in files:
            detections = self.analyze_file(str(file_path))
            all_detections.extend(detections)
            if detections:
                print(f"  Found {len(detections)} pattern(s) in {file_path.name}")

        return all_detections

    def generate_report(self, output_file: str = None) -> str:
        """Generate a detailed analysis report"""
        report_lines = []
        report_lines.append("=" * 80)
        report_lines.append("LeiaSR Display API Migration Analysis Report")
        report_lines.append("=" * 80)
        report_lines.append("")

        # Summary
        total = len(self.detections)
        by_confidence = {
            'high': len([d for d in self.detections if d.confidence == 'high']),
            'medium': len([d for d in self.detections if d.confidence == 'medium']),
            'low': len([d for d in self.detections if d.confidence == 'low'])
        }

        report_lines.append(f"Total patterns detected: {total}")
        report_lines.append(f"  High confidence (auto-migrate safe): {by_confidence['high']}")
        report_lines.append(f"  Medium confidence (review recommended): {by_confidence['medium']}")
        report_lines.append(f"  Low confidence (manual review required): {by_confidence['low']}")
        report_lines.append("")

        # Group by file
        by_file = {}
        for detection in self.detections:
            if detection.file_path not in by_file:
                by_file[detection.file_path] = []
            by_file[detection.file_path].append(detection)

        report_lines.append(f"Files requiring migration: {len(by_file)}")
        report_lines.append("")

        # Detailed findings
        for file_path, detections in sorted(by_file.items()):
            report_lines.append("-" * 80)
            report_lines.append(f"File: {file_path}")
            report_lines.append(f"  Patterns found: {len(detections)}")
            report_lines.append("")

            for detection in sorted(detections, key=lambda d: d.line_number):
                report_lines.append(f"  Line {detection.line_number}: {detection.pattern_name}")
                report_lines.append(f"    Confidence: {detection.confidence.upper()}")
                report_lines.append(f"    Matched: {detection.matched_text[:100]}")

                if detection.capture_groups:
                    report_lines.append(f"    Captures: {detection.capture_groups}")

                report_lines.append("")

        report = '\n'.join(report_lines)

        if output_file:
            with open(output_file, 'w') as f:
                f.write(report)
            print(f"\nReport saved to: {output_file}")

        return report

    def get_files_to_migrate(self) -> List[str]:
        """Get list of unique file paths that need migration"""
        return sorted(set(d.file_path for d in self.detections))

    def get_statistics(self) -> Dict:
        """Get migration statistics"""
        files = self.get_files_to_migrate()

        return {
            'total_detections': len(self.detections),
            'files_to_migrate': len(files),
            'by_confidence': {
                'high': len([d for d in self.detections if d.confidence == 'high']),
                'medium': len([d for d in self.detections if d.confidence == 'medium']),
                'low': len([d for d in self.detections if d.confidence == 'low'])
            },
            'by_pattern': self._count_by_pattern(),
            'files': files
        }

    def _count_by_pattern(self) -> Dict[str, int]:
        """Count detections by pattern type"""
        counts = {}
        for detection in self.detections:
            counts[detection.pattern_name] = counts.get(detection.pattern_name, 0) + 1
        return counts


if __name__ == '__main__':
    # Simple test
    import sys

    if len(sys.argv) < 2:
        print("Usage: python analyzer.py <directory>")
        sys.exit(1)

    analyzer = CodeAnalyzer()
    analyzer.analyze_directory(sys.argv[1])
    print(analyzer.generate_report())
