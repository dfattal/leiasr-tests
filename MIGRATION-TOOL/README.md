# LeiaSR Display API Migration Tool

**Version 1.2.0** - Two Migration Modes: Modern (Default) & Legacy Fallback

This directory contains the complete migration tool for converting legacy LeiaSR Display API code to the modern IDisplayManager interface. The tool now offers two migration strategies to suit different deployment scenarios.

## 📁 Directory Structure

```
MIGRATION-TOOL/
├── python-source/          # Python source code for the migration tool
│   ├── leiasr_migrate/     # Main package
│   ├── setup.py            # Python package setup
│   ├── requirements.txt    # Python dependencies
│   └── leiasr-migrate.spec # PyInstaller build configuration
├── release/                # Windows executable release package
│   ├── leiasr-migrate.exe  # Standalone Windows executable (7.9 MB)
│   ├── README.md           # Complete user documentation
│   ├── QUICKSTART.txt      # Quick start guide
│   └── CHANGELOG.md        # Version history
└── README.md               # This file
```

## 🚀 Quick Start

### For End Users (Windows)

If you just want to migrate your code:

1. Navigate to `release/` folder
2. Read `QUICKSTART.txt` or `README.md`
3. Run the migration tool:

```cmd
REM Modern mode (default - recommended)
leiasr-migrate.exe migrate C:\path\to\your\project

REM Legacy fallback mode (backward compatible)
leiasr-migrate.exe migrate C:\path\to\your\project --legacy-fallback
```

**No Python installation required!** The executable is completely standalone.

### For Developers

If you want to build from source or contribute:

1. Go to `python-source/` folder
2. Install: `pip install -r requirements.txt`
3. Run: `python -m leiasr_migrate.cli analyze <path>`

## 📖 Documentation

### User Documentation
- **`release/README.md`** - Complete user guide (14KB, comprehensive)
- **`release/QUICKSTART.txt`** - Quick reference card (5KB, plain text)
- **`release/CHANGELOG.md`** - Version history and release notes

### Developer Documentation
- See `.github/workflows/` for automated build and test workflows
- Python source in `python-source/` with inline documentation

## ✨ Features

### Version 1.2.0 (Current - 2025-10-24)

#### Two Migration Modes
- **Modern Mode (DEFAULT)** - Clean, direct IDisplayManager API migration
  - Uses `SR::GetDisplayManagerInstance()`
  - No helper files needed
  - Requires SDK 1.34.8-RC1+ runtime

- **Legacy Fallback Mode** - Backward-compatible migration
  - Auto-generates `display_helper.h` with runtime fallback
  - Works with older SDK runtimes
  - Use `--legacy-fallback` flag

#### Core Features
- **Self-contained executable** - No dependencies, runs anywhere on Windows
- **Intelligent transformations** - Handles pointers, nullptr checks, API calls
- **Safe migrations** - Creates `.legacy` backups automatically
- **Dry-run mode** - Preview changes before applying
- **Detailed reporting** - Shows all transformations with line numbers

## 🔧 What It Does

### Modern Mode (Default)
The migration tool automatically:

1. **Detects** legacy `SR::Display*` API usage patterns
2. **Analyzes** your codebase with confidence scoring
3. **Transforms** to use `SR::GetDisplayManagerInstance()` and `IDisplay*`
4. **Creates** backup files (`.legacy`) for safe rollback
5. **Reports** all changes with line numbers

### Legacy Fallback Mode (`--legacy-fallback`)
Additionally performs:

1. **Auto-generates** `display_helper.h` with the DisplayAccess helper class
2. **Adds** `SRDISPLAY_LAZYBINDING` define for runtime fallback
3. **Converts** pointer syntax (`->`) to direct access (`.`)
4. **Transforms** nullptr checks to `isDisplayValid()` calls
5. **Enables** backward compatibility with older SDK runtimes

## 📊 Migration Examples

### Before (Legacy API)
```cpp
#include "sr/world/display/display.h"

SR::Display* display = SR::Display::create(context);
if (display != nullptr) {
    int width = display->getResolutionWidth();
}
```

### After - Modern Mode (Default)
```cpp
#include "sr/world/display/display.h"

SR::IDisplayManager* displayMgr_display = SR::GetDisplayManagerInstance(context);
SR::IDisplay* display = displayMgr_display->getPrimaryActiveSRDisplay();
if (display != nullptr) {
    int width = display->getResolutionWidth();
}
```

### After - Legacy Fallback Mode (`--legacy-fallback`)
```cpp
#define SRDISPLAY_LAZYBINDING  // Auto-added
#include "sr/world/display/display.h"
#include "display_helper.h"  // Auto-generated!

SR::Helper::DisplayAccess display(context);
if (display.isDisplayValid()) {  // Auto-converted from nullptr check
    int width = display.getResolutionWidth();
}
```

## 🛠️ Building the Executable

The Windows executable is automatically built via GitHub Actions:

**Build Workflow**: `.github/workflows/build-migration-tool.yml`
- Uses PyInstaller to create standalone `.exe` (7.9 MB)
- Embeds all dependencies including `display_helper.h` template
- Runs on every push to `MIGRATION-TOOL/python-source/`
- Artifacts uploaded with 90-day retention

**Test Workflow**: `.github/workflows/test-migration-tool.yml`
- Tests both modern and legacy fallback modes
- Uses DirectX 11 weaving example from SDK
- Validates all transformations
- Verifies executables build successfully

**Example Build Workflow**: `.github/workflows/build-migrated-examples.yml`
- Builds complete migrated executables
- Tests both migration modes end-to-end
- Results available in `EXAMPLES/migration-results/`

## 📋 Requirements

### For End Users
- **Windows 10 or later**
- **LeiaSR SDK 1.34.8-RC1 or later** (for modern mode)
- Older SDK versions supported with legacy fallback mode

### For Developers
- **Python 3.11+**
- **PyYAML 6.0+**
- **PyInstaller 6.0+** (for building executable)

## 📦 Version History

### v1.2.0 (2025-10-24) - Current
**Two Migration Modes**
- ✨ Modern mode (default): Direct `GetDisplayManagerInstance()` API
- ✨ Legacy fallback mode: Backward-compatible with `DisplayAccess` helper
- ✨ Automatic nullptr → `isDisplayValid()` conversion
- ✨ Complete end-to-end testing with built executables
- 🐛 Fixed: Uses `GetDisplayManagerInstance()` instead of `create()`
- 🐛 Fixed: Proper handling of DisplayAccess object comparisons

### v1.1.0 (2025-10-24)
- Auto-generation of `display_helper.h`
- Simplified command interface
- Self-contained migration process

### v1.0.0 (2025-10-24)
- Initial release
- Manual helper file setup required

See `release/CHANGELOG.md` for complete history.

## 🎯 Migration Results

Validated migration examples available in `EXAMPLES/migration-results/`:

**Modern Mode:**
- ✅ DirectX 11 example migrated and built (226 KB executable)
- Uses `GetDisplayManagerInstance()` + `getPrimaryActiveSRDisplay()`
- Clean, modern code without helper files

**Legacy Fallback Mode:**
- ✅ DirectX 11 example migrated and built (228 KB executable)
- Uses `DisplayAccess` helper with runtime fallback
- Includes auto-generated `display_helper.h` (8 KB)

## 🐛 Troubleshooting & Support

### Common Issues

**Build Errors After Migration?**
- Modern mode: Ensure SDK 1.34.8-RC1+ is installed
- Legacy fallback: Check that `display_helper.h` was created
- Verify include paths in your build system

**Migration Didn't Detect Files?**
- Ensure files use `#include "sr/world/display/display.h"`
- Check that `SR::Display::create()` patterns exist
- Run analyze mode first: `leiasr-migrate.exe analyze <path>`

**Need to Revert?**
- Original files saved as `*.legacy`
- Simply delete migrated files and rename `.legacy` back
- Or use version control: `git restore .`

### Getting Help
- **User Guide**: See `release/README.md` (comprehensive)
- **Quick Start**: See `release/QUICKSTART.txt` (fast reference)
- **Examples**: Check `EXAMPLES/migration-results/` for working code
- **Workflows**: Review `.github/workflows/` for automation examples

## 📝 License

Copyright (c) 2025 Leia Inc.
