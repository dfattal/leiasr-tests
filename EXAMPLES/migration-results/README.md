# Migration Results

This directory contains the results from running the migration tool on DirectX 11 example in both migration modes.

## Structure

After running the test workflow, this directory will contain:

```
migration-results/
├── modern/                          # Modern mode migration results
│   ├── main.cpp                     # Migrated code (uses IDisplayManager)
│   └── main.cpp.legacy              # Original code before migration
├── legacy-fallback/                 # Legacy fallback mode migration results
│   ├── main.cpp                     # Migrated code (uses DisplayAccess)
│   ├── main.cpp.legacy              # Original code before migration
│   └── display_helper.h             # Auto-generated helper file
└── README.md                        # This file
```

## Migration Modes

### Modern Mode (DEFAULT)
- Direct IDisplayManager API usage
- No helper files
- Clean, simple code
- Requires SDK 1.34.8-RC1+ runtime

**Example transformation:**
```cpp
// Before
SR::Display* display = SR::Display::create(context);

// After
SR::IDisplayManager* displayMgr_display = SR::IDisplayManager::create(context);
SR::IDisplay* display = displayMgr_display->getPrimaryActiveSRDisplay();
```

### Legacy Fallback Mode
- Uses DisplayAccess helper class
- Auto-generates display_helper.h
- Runtime fallback to legacy Display class
- Compatible with older SDK runtimes

**Example transformation:**
```cpp
// Before
SR::Display* display = SR::Display::create(context);

// After
#define SRDISPLAY_LAZYBINDING
#include "display_helper.h"
SR::Helper::DisplayAccess display(context);
```

## How to Generate These Results

1. Push changes to trigger the test workflow
2. Or manually trigger via GitHub Actions:
   - Go to Actions tab → Test Migration Tool Executable → Run workflow
3. Wait for workflow to complete
4. Download artifacts using `gh run download <run-id>`
5. Organize files into modern/ and legacy-fallback/ directories

## Test Workflow

The test workflow (`.github/workflows/test-migration-tool.yml`) performs:
- Builds migration tool executable
- Tests modern migration mode on directx11_weaving example
- Tests legacy fallback migration mode on directx11_weaving example
- Validates transformations
- Uploads artifacts with migration results
