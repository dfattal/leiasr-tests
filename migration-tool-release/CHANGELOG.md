# Changelog

All notable changes to the LeiaSR Display API Migration Tool will be documented in this file.

## [1.1.0] - 2025-10-24

### ✨ Added
- **Auto-generation of display_helper.h**: The tool now automatically creates the helper file in the target directory during migration
- Embedded 252-line display_helper.h template directly in the executable
- Simplified include path: now uses `#include "display_helper.h"` instead of `#include "../common/display_helper.h"`

### 🎯 Changed
- **Removed --helper-path argument**: No longer needed since file is auto-generated
- Simplified command syntax: just `leiasr-migrate migrate <path>`
- Updated CLI help text and examples

### 🔧 Fixed
- Excluded display_helper.h from migration analysis to prevent migrating the helper file itself
- Fixed issue where tool would try to process helper file (reduced false detections from 44 to 12 patterns)

### 📝 Improved
- More user-friendly workflow - no manual file copying required
- Self-contained migration process
- Helper file always matches the tool version

## [1.0.0] - 2025-10-24

### 🎉 Initial Release

#### Features
- **Analyze Command**: Detect legacy Display API patterns in C++ code
  - Pattern detection using regex and YAML configuration
  - Confidence scoring (high/medium/low)
  - Detailed analysis reports

- **Migrate Command**: Transform code to modern IDisplayManager API
  - Adds `#define SRDISPLAY_LAZYBINDING` for version compatibility
  - Converts `SR::Display*` to `SR::Helper::DisplayAccess`
  - Changes pointer syntax (`->`) to direct access (`.`)
  - Creates `.legacy` backup files

- **Safety Features**:
  - Dry-run mode to preview changes
  - Automatic backups (rename to .legacy)
  - Confirmation prompts
  - Detailed transformation reports

- **Distribution**:
  - Standalone Windows executable (7.9 MB)
  - No Python installation required
  - Built with PyInstaller
  - Automated GitHub Actions builds

#### Transformations
- Detection patterns: 7 types
- Transformation rules: 4 priorities
- Supported files: .cpp, .h, .hpp, .cc

#### Testing
- End-to-end testing on GitHub Actions
- Verified on real SDK examples (opengl_weaving)
- 12/12 tests passing

---

## Version Numbering

This project follows [Semantic Versioning](https://semver.org/):
- **Major**: Breaking changes
- **Minor**: New features, backwards compatible
- **Patch**: Bug fixes, backwards compatible

## Links

- [Repository](https://github.com/dfattal/leiasr-tests)
- [Latest Release](https://github.com/dfattal/leiasr-tests/actions/workflows/build-migration-tool.yml)
- [Documentation](https://github.com/dfattal/leiasr-tests/leiasr-migrate/README.md)
