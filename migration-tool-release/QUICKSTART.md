# LeiaSR Migration Tool - Quick Start Guide

## What's New in Version 1.1.0

🎉 **display_helper.h is now automatically created!**
- No manual file copying required
- Simplified command (no --helper-path argument)
- Self-contained migration process

## What's Included

- `leiasr-migrate.exe` - Standalone executable (7.9 MB, no Python required)
- `FULL-README.md` - Complete documentation
- `README.txt` - Basic usage info
- `QUICKSTART.md` - This file

## Requirements

- Windows 10 or later
- No Python installation needed!

## Quick Start

### 1. Analyze Your Project

Open Command Prompt or PowerShell and run:

```cmd
leiasr-migrate.exe analyze C:\path\to\your\project
```

This will scan your code and tell you what needs migration.

### 2. Preview Changes (Dry Run)

```cmd
leiasr-migrate.exe migrate C:\path\to\your\project --dry-run
```

This shows what will be changed without modifying files.

### 3. Perform Migration

```cmd
leiasr-migrate.exe migrate C:\path\to\your\project
```

This will:
- **Automatically create** `display_helper.h` in your project
- Rename original files to `.legacy` extension
- Write migrated code to original filenames
- Add SRDISPLAY_LAZYBINDING support

### 4. Review Results

Compare the migrated code with originals:

```cmd
fc /n main.cpp.legacy main.cpp
```

Or use any diff tool (WinMerge, Beyond Compare, VS Code, etc.)

### 5. Test Your Build

```cmd
cd your\project
cmake --build build --config Release
```

### 6. Clean Up (Once Satisfied)

Delete `.legacy` files:

```cmd
del /s *.legacy
```

## Common Issues

### "leiasr-migrate.exe is not recognized"

Make sure you're in the same directory as the executable, or add it to your PATH.

### Build Errors After Migration

Add to your CMakeLists.txt:
```cmake
add_definitions(-DSRDISPLAY_LAZYBINDING)
```

## Example Session

```cmd
C:\> leiasr-migrate.exe analyze C:\my_app

Files requiring migration: 1
Total patterns detected: 12
  High confidence: 4
  Medium confidence: 8

C:\> leiasr-migrate.exe migrate C:\my_app

Proceed with migration? (y/N): y

Created display_helper.h at C:\my_app\display_helper.h
Migrated main.cpp (9 changes)
Original saved to main.cpp.legacy

Migration Complete!

C:\> cd my_app
C:\my_app> cmake --build build --config Release
C:\my_app> build\Release\my_app.exe

✓ Everything works!

C:\my_app> del *.legacy
```

## What Gets Migrated

### Before Migration
```cpp
#include "sr/world/display/display.h"

SR::Display* display = SR::Display::create(context);
SR_recti displayLocation = display->getLocation();
```

### After Migration
```cpp
#define SRDISPLAY_LAZYBINDING  // Enable modern DisplayManager with fallback
#include "sr/world/display/display.h"
#include "display_helper.h"    // <-- Automatically created!

SR::Helper::DisplayAccess display(context);
SR_recti displayLocation = display.getLocation();
```

## Getting Help

Run with `--help` for full options:

```cmd
leiasr-migrate.exe --help
leiasr-migrate.exe analyze --help
leiasr-migrate.exe migrate --help
```

## Version Information

**Version**: 1.1.0 (Auto-Generation Update)
**Build Date**: October 24, 2025
**Built From**: [leiasr-tests](https://github.com/dfattal/leiasr-tests) repository
**Changes**:
- ✨ Auto-generates display_helper.h
- ✅ Removed --helper-path argument
- ✅ Simplified workflow
- ✅ Self-contained migration

## More Information

See `FULL-README.md` for:
- Complete command reference
- Migration patterns explained
- Troubleshooting guide
- Advanced usage
- File structure details

## Support

For issues or questions, see documentation at:
https://github.com/dfattal/leiasr-tests
