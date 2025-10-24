# LeiaSR Migration Tool - Quick Start Guide

## What's Included

- `leiasr-migrate.exe` - Standalone executable (no Python required)
- `README.md` - Full documentation
- `README.txt` - Basic usage info

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
leiasr-migrate.exe migrate C:\path\to\your\project --helper-path=../common
```

This will:
- Rename original files to `.legacy` extension
- Write migrated code to original filenames
- Add SRDISPLAY_LAZYBINDING support
- Include display_helper.h

### 4. Review Results

Compare the migrated code with originals:

```cmd
fc /n main.cpp.legacy main.cpp
```

Or use any diff tool (WinMerge, Beyond Compare, etc.)

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

### Missing display_helper.h

Make sure you've copied `display_helper.h` to your common includes directory and specify the correct path with `--helper-path`.

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

C:\> leiasr-migrate.exe migrate C:\my_app --helper-path=../common

Proceed with migration? (y/N): y

Migrated main.cpp (9 changes)
Original saved to main.cpp.legacy

Migration Complete!

C:\> cd my_app
C:\my_app> cmake --build build --config Release
C:\my_app> build\Release\my_app.exe

✓ Everything works!

C:\my_app> del *.legacy
```

## Getting Help

Run with `--help` for full options:

```cmd
leiasr-migrate.exe --help
leiasr-migrate.exe analyze --help
leiasr-migrate.exe migrate --help
```

## More Information

See `README.md` for:
- Complete command reference
- Migration patterns explained
- Troubleshooting guide
- Advanced usage

## Version

**1.0.0** - Built from [leiasr-tests](https://github.com/dfattal/leiasr-tests) repository

Generated with GitHub Actions on Windows Server 2022.
