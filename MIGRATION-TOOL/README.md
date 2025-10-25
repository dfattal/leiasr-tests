# LeiaSR Display API Migration Tool

This directory contains the complete migration tool for converting legacy LeiaSR Display API code to the modern IDisplayManager interface.

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
├── MIGRATION_TOOL.md       # Technical overview of the migration tool
├── TEST_RESULTS.md         # Testing validation results
├── EXECUTABLE_BUILD.md     # Build process documentation
├── RELEASE_NOTES_v1.1.md   # v1.1.0 release notes
└── README.md               # This file
```

## 🚀 Quick Start

### For End Users (Windows)

If you just want to migrate your code:

1. Go to `release/` folder
2. Read `QUICKSTART.txt` or `README.md`
3. Run `leiasr-migrate.exe`

**No Python installation required!**

### For Developers

If you want to build from source or contribute:

1. Go to `python-source/` folder
2. Install: `pip install -r requirements.txt`
3. Run: `python -m leiasr_migrate.cli analyze <path>`

## 📖 Documentation

- **`release/README.md`** - Complete user guide for the Windows executable
- **`release/QUICKSTART.txt`** - Quick reference card (plain text)
- **`MIGRATION_TOOL.md`** - Technical design and architecture
- **`EXECUTABLE_BUILD.md`** - How the executable is built via GitHub Actions
- **`TEST_RESULTS.md`** - Validation test results
- **`RELEASE_NOTES_v1.1.md`** - What's new in v1.1.0

## ✨ Features

### Version 1.1.0 (Current)

- **Auto-generates `display_helper.h`** - No manual file copying needed
- **Self-contained executable** - Everything embedded in the .exe
- **Simplified commands** - Just `analyze` and `migrate`
- **Safe transformations** - Creates `.legacy` backups
- **Dry-run mode** - Preview changes before applying

## 🔧 What It Does

The migration tool automatically:

1. **Detects** legacy `SR::Display*` API usage patterns
2. **Analyzes** your codebase with confidence scoring
3. **Auto-creates** `display_helper.h` with the DisplayAccess helper class
4. **Transforms** code to use modern `IDisplayManager` API
5. **Adds** `SRDISPLAY_LAZYBINDING` for backward compatibility
6. **Converts** pointer syntax (`->`) to direct access (`.`)
7. **Creates** backup files (`.legacy`) for rollback

## 📊 Migration Example

### Before
```cpp
#include "sr/world/display/display.h"

SR::Display* display = SR::Display::create(context);
int width = display->getResolutionWidth();
```

### After
```cpp
#define SRDISPLAY_LAZYBINDING
#include "sr/world/display/display.h"
#include "display_helper.h"  // Auto-generated!

SR::Helper::DisplayAccess display(context);
int width = display.getResolutionWidth();
```

## 🛠️ Building the Executable

The Windows executable is automatically built via GitHub Actions:

1. Workflow: `.github/workflows/build-migration-tool.yml`
2. Uses PyInstaller to create standalone `.exe`
3. Embeds all dependencies including `display_helper.h` template
4. Artifacts available in GitHub Actions runs

See `EXECUTABLE_BUILD.md` for details.

## 🧪 Testing

Automated tests run on every commit:

- Workflow: `.github/workflows/test-migration-tool.yml`
- Tests on real SDK example (`opengl_weaving`)
- Validates all transformations
- Verifies auto-generation of helper file

See `TEST_RESULTS.md` for latest results.

## 📋 Requirements

### For End Users
- Windows 10 or later
- LeiaSR SDK 1.34.8-RC1 or later

### For Developers
- Python 3.7+
- PyYAML 6.0+
- PyInstaller (for building executable)

## 🔗 Related Documentation

In the project root:
- `DisplayManager.md` - Modern IDisplayManager API reference
- `MODERNIZED_EXAMPLES.md` - Example code using the new API
- `sdk-summary.md` - LeiaSR SDK overview

## 📦 Releases

### v1.1.0 (2025-10-24)
- Auto-generation of `display_helper.h`
- Simplified command interface
- Self-contained migration process
- See `RELEASE_NOTES_v1.1.md` for details

### v1.0.0 (2025-10-24)
- Initial release
- Manual helper file setup required
- See `CHANGELOG.md` in release/ folder

## 🐛 Issues & Support

For issues or questions:
- Check documentation in `release/README.md`
- Review `MIGRATION_TOOL.md` for technical details
- See test results in `TEST_RESULTS.md`

## 📝 License

Copyright (c) 2025 Leia Inc.
