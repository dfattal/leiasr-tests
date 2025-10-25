#!/usr/bin/env python3
"""
LeiaSR Display API Migration Tool
Command-line interface for analyzing and migrating legacy Display code
"""

import sys
import os
import argparse
from pathlib import Path

# Add parent directory to path for imports
sys.path.insert(0, str(Path(__file__).parent.parent))

from leiasr_migrate.analyzer import CodeAnalyzer
from leiasr_migrate.transformer import CodeTransformer


def cmd_analyze(args):
    """Analyze codebase for migration needs"""
    print("=" * 80)
    print("LeiaSR Display API Migration Analyzer")
    print("=" * 80)
    print()

    analyzer = CodeAnalyzer()

    # Analyze the target
    if os.path.isfile(args.path):
        analyzer.analyze_file(args.path)
    else:
        analyzer.analyze_directory(args.path, recursive=not args.no_recursive)

    # Generate report
    report_file = args.report if args.report else None
    report = analyzer.generate_report(report_file)

    if not report_file:
        print(report)

    # Print statistics
    stats = analyzer.get_statistics()
    print()
    print("=" * 80)
    print("Summary")
    print("=" * 80)
    print(f"Files requiring migration: {stats['files_to_migrate']}")
    print(f"Total patterns detected: {stats['total_detections']}")
    print(f"  High confidence: {stats['by_confidence']['high']}")
    print(f"  Medium confidence: {stats['by_confidence']['medium']}")
    print(f"  Needs review: {stats['by_confidence']['low']}")
    print()

    if stats['files_to_migrate'] > 0:
        print("Files to migrate:")
        for file in stats['files']:
            print(f"  - {file}")
        print()
        print("Next steps:")
        print(f"  1. Review the analysis report")
        if report_file:
            print(f"     Report saved to: {report_file}")
        print(f"  2. Run migration:")
        print(f"     leiasr-migrate migrate {args.path}")
    else:
        print("No migration needed - no legacy Display API usage detected!")

    return 0


def cmd_migrate(args):
    """Perform the migration"""
    print("=" * 80)
    print("LeiaSR Display API Migration Tool")
    print("=" * 80)
    print()

    if args.dry_run:
        print("DRY RUN MODE - No files will be modified")
        print()

    # First analyze
    analyzer = CodeAnalyzer()
    if os.path.isfile(args.path):
        analyzer.analyze_file(args.path)
    else:
        analyzer.analyze_directory(args.path, recursive=not args.no_recursive)

    files_to_migrate = analyzer.get_files_to_migrate()

    if not files_to_migrate:
        print("No files require migration!")
        return 0

    print(f"Found {len(files_to_migrate)} files to migrate:")
    for file in files_to_migrate:
        print(f"  - {file}")
    print()

    # Confirm unless --yes flag
    if not args.yes and not args.dry_run:
        response = input("Proceed with migration? (y/N): ")
        if response.lower() != 'y':
            print("Migration cancelled.")
            return 1

    # Perform transformation
    transformer = CodeTransformer(legacy_fallback=args.legacy_fallback)

    # Print migration mode
    if args.legacy_fallback:
        print("Migration mode: Legacy Fallback (backward compatible)")
        print("  - Creates display_helper.h with runtime fallback logic")
        print("  - Compatible with older SDK runtimes")
    else:
        print("Migration mode: Modern (IDisplayManager)")
        print("  - Direct IDisplayManager API usage")
        print("  - Requires SDK 1.34.8-RC1 or later runtime")
    print()

    transformations = transformer.transform_directory(
        args.path,
        file_list=files_to_migrate,
        dry_run=args.dry_run
    )

    # Generate report
    report_file = args.report if args.report else None
    report = transformer.generate_diff_report(report_file)

    if not report_file and transformations:
        print()
        print(report)

    print()
    print("=" * 80)
    print("Migration Complete!")
    print("=" * 80)
    print(f"Files modified: {len(set(t.file_path for t in transformations))}")
    print(f"Total transformations: {len(transformations)}")

    if not args.dry_run:
        print()
        print("Original files renamed with .legacy extension")
        print()
        print("Next steps:")
        print("  1. Review the migrated code")
        print("  2. Test your application")
        print("  3. Run build to verify compilation")
        print("  4. If issues occur, use .legacy files as reference")
        print("  5. Once satisfied, you can delete .legacy files")

    return 0




def main():
    """Main entry point"""
    parser = argparse.ArgumentParser(
        description='LeiaSR Display API Migration Tool',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  # Analyze a project
  leiasr-migrate analyze /path/to/project --report=analysis.txt

  # Migrate with dry-run first
  leiasr-migrate migrate /path/to/project --dry-run

  # Perform migration
  leiasr-migrate migrate /path/to/project
        """
    )

    subparsers = parser.add_subparsers(dest='command', help='Command to execute')

    # Analyze command
    analyze_parser = subparsers.add_parser('analyze',
                                          help='Analyze code for migration needs')
    analyze_parser.add_argument('path', help='File or directory to analyze')
    analyze_parser.add_argument('--report', '-r', help='Output report file')
    analyze_parser.add_argument('--no-recursive', action='store_true',
                               help='Do not recurse into subdirectories')

    # Migrate command
    migrate_parser = subparsers.add_parser('migrate',
                                          help='Perform migration')
    migrate_parser.add_argument('path', help='File or directory to migrate')
    migrate_parser.add_argument('--dry-run', action='store_true',
                               help='Show what would be changed without modifying files')
    migrate_parser.add_argument('--yes', '-y', action='store_true',
                               help='Skip confirmation prompt')
    migrate_parser.add_argument('--legacy-fallback', action='store_true',
                               help='Enable backward compatibility with older runtimes (creates display_helper.h with fallback logic)')
    migrate_parser.add_argument('--report', '-r', help='Output transformation report file')
    migrate_parser.add_argument('--no-recursive', action='store_true',
                               help='Do not recurse into subdirectories')

    # Parse arguments
    args = parser.parse_args()

    if not args.command:
        parser.print_help()
        return 1

    # Execute command
    if args.command == 'analyze':
        return cmd_analyze(args)
    elif args.command == 'migrate':
        return cmd_migrate(args)
    else:
        parser.print_help()
        return 1


if __name__ == '__main__':
    sys.exit(main())
