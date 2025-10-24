# LeiaSR Display API Migration Tool

Automated migration tool for converting legacy LeiaSR Display API code to the modern IDisplayManager interface with graceful fallback support.

## Overview

This tool automatically migrates C++ code from the legacy `SR::Display` API to the modern `SR::IDisplayManager` pattern with fallback to legacy API for backward compatibility.

### What It Does

- **Detects** legacy Display API usage patterns in your C++ codebase
- **Analyzes** migration needs with confidence scoring
- **Transforms** code safely with automatic backups
- **Adds** lazy binding support via `SRDISPLAY_LAZYBINDING`
- **Integrates** the DisplayAccess helper class
- **Provides** rollback capability if needed

## Installation

### From Source

```bash
cd leiasr-migrate
pip install -e .
```

This installs the `leiasr-migrate` command-line tool.

### Requirements

- Python 3.7+
- PyYAML 6.0+

## Usage

### 1. Analyze Your Code

First, analyze your project to see what needs migration:

```bash
leiasr-migrate analyze /path/to/your/project
```

Save the analysis report to a file:

```bash
leiasr-migrate analyze /path/to/your/project --report=analysis.txt
```

### 2. Test Migration (Dry Run)

See what changes would be made without actually modifying files:

```bash
leiasr-migrate migrate /path/to/your/project --dry-run
```

### 3. Perform Migration

Migrate your code (original files will be renamed to .legacy):

```bash
leiasr-migrate migrate /path/to/your/project
```

Specify the path to display_helper.h:

```bash
leiasr-migrate migrate /path/to/your/project --helper-path=../common
```

Skip confirmation prompt:

```bash
leiasr-migrate migrate /path/to/your/project --yes
```

## What Gets Migrated

### Before (Legacy API)

```cpp
#include "sr/world/display/display.h"

SR::Display* display = SR::Display::create(context);
int width = display->getResolutionWidth();
int height = display->getResolutionHeight();
```

### After (Modern API with Fallback)

```cpp
#define SRDISPLAY_LAZYBINDING  // Enable modern DisplayManager with fallback
#include "sr/world/display/display.h"
#include "../common/display_helper.h"

SR::Helper::DisplayAccess display(context);
int width = display.getResolutionWidth();
int height = display.getResolutionHeight();
```

## Transformations Applied

The tool applies these transformations in order:

1. **Add SRDISPLAY_LAZYBINDING**: Enables lazy binding before display.h include
2. **Add Helper Include**: Includes display_helper.h for DisplayAccess class
3. **Replace Display::create()**: Converts `SR::Display*` pointers to `SR::Helper::DisplayAccess` objects
4. **Update Pointer Syntax**: Changes `->` to `.` for DisplayAccess variables

## Detection Patterns

The analyzer detects:

- Legacy display.h includes without lazy binding
- `SR::Display::create()` calls
- Display method calls that need validity checking
- Manual display polling loops
- Manual nullptr checks

## Confidence Levels

- **High**: Safe for automatic migration (includes, Display::create)
- **Medium**: Review recommended (inline creates, method calls)
- **Low**: Manual review required (complex control flow)

## Example Workflow

```bash
# 1. Analyze your project
leiasr-migrate analyze ./my_leiasr_app --report=migration_report.txt

# 2. Review the report
cat migration_report.txt

# 3. Test migration with dry-run
leiasr-migrate migrate ./my_leiasr_app --dry-run

# 4. Perform actual migration
leiasr-migrate migrate ./my_leiasr_app --helper-path=../common

# 5. Test your application
cd my_leiasr_app
cmake --build build
./build/Release/my_app.exe

# 6. If everything works, delete .legacy files
find ./my_leiasr_app -name "*.legacy" -delete
```

## Output Files

- **Original files**: Renamed to `filename.ext.legacy`
- **Migrated files**: Modern code written to original filename
- **Reports**: Analysis and transformation details (optional)

## Safety Features

- ✅ Original files preserved with .legacy extension
- ✅ Dry-run mode to preview changes
- ✅ Confirmation prompts (skip with --yes)
- ✅ Detailed transformation reports
- ✅ Easy to compare migrated vs original
- ✅ No modifications during analysis

## Command Reference

### analyze

```bash
leiasr-migrate analyze <path> [--report FILE] [--no-recursive]
```

- `<path>`: File or directory to analyze
- `--report FILE`: Save analysis report to file
- `--no-recursive`: Don't recurse into subdirectories

### migrate

```bash
leiasr-migrate migrate <path> [--helper-path PATH] [--dry-run] [--yes] [--report FILE] [--no-recursive]
```

- `<path>`: File or directory to migrate
- `--helper-path PATH`: Path to display_helper.h (default: ../common)
- `--dry-run`: Show changes without modifying files
- `--yes`: Skip confirmation prompt
- `--report FILE`: Save transformation report to file
- `--no-recursive`: Don't recurse into subdirectories


## Prerequisites

Before using this tool, ensure you have:

1. **DisplayAccess Helper**: Copy `display_helper.h` to your common includes directory
2. **SDK Version**: LeiaSR SDK 1.34.8-RC1 or later (with IDisplayManager support)
3. **CMake Updates**: Add `-DSRDISPLAY_LAZYBINDING` to your CMakeLists.txt

## Next Steps After Migration

1. **Review Changes**: Check the transformation report
2. **Compare Files**: Diff migrated files against .legacy versions if needed
3. **Update CMakeLists.txt**: Add `add_definitions(-DSRDISPLAY_LAZYBINDING)`
4. **Test Build**: Verify your project compiles
5. **Test Runtime**: Run with both old and new runtimes
6. **Add Validity Checks**: Wrap display usage with `if (display.isDisplayValid())`
7. **Clean Up**: Delete .legacy files once satisfied

## Troubleshooting

### "No files require migration"

- Check that your files use `#include "sr/world/display/display.h"`
- Verify you're targeting the right directory
- Files may already be migrated

### Build Errors After Migration

- Ensure display_helper.h is in the include path
- Add `-DSRDISPLAY_LAZYBINDING` to CMake
- Check that helper_path is correct

### Runtime Errors

- Verify SDK version compatibility
- Add validity checks: `if (display.isDisplayValid())`
- Check console output for API detection messages

### Need to Revert Migration

- Simply delete the migrated files
- Rename .legacy files back to original names: `mv main.cpp.legacy main.cpp`

## Support

For issues or questions:
- Review DisplayManager.md for technical details
- Check MODERNIZED_EXAMPLES.md for working examples
- See sdk-summary.md for SDK overview

## License

Copyright (c) Leia Inc.
