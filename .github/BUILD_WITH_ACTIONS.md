# Building with GitHub Actions

This repository uses GitHub Actions to automatically build Windows applications **from any platform** (Mac, Linux, Windows).

---

## Overview

The GitHub Actions workflow automatically builds both applications:
- **LookAroundEyesApp** - Eye tracking comparison tool
- **LeiaSplatViewer** - Gaussian Splat viewer

Every push to `main` triggers a build, and you can download the resulting executables as ZIP files.

---

## Quick Start

### 1. Push Your Code (from Mac/Linux/Windows)

```bash
# From your Mac terminal
cd /path/to/leiasr-tests
git add .
git commit -m "Update applications"
git push origin main
```

### 2. Monitor the Build

1. Go to your GitHub repository
2. Click the **"Actions"** tab at the top
3. Click on the most recent workflow run
4. Watch the build progress (takes ~5-10 minutes)

### 3. Download the Artifacts

Once the build completes successfully:

1. Scroll down to the **"Artifacts"** section
2. Download the ZIP files:
   - `LookAroundEyesApp-windows-x64.zip`
   - `LeiaSplatViewer-windows-x64.zip`

3. Extract on your Windows machine
4. Run the `.exe` files

---

## Artifact Contents

### LookAroundEyesApp-windows-x64.zip

```
LookAroundEyesApp-windows-x64/
├── LookAroundEyesApp.exe        ← Main executable
├── SimulatedRealityCore.dll
├── SimulatedRealityDisplays.dll
├── SimulatedRealityWeaving.dll
├── SimulatedRealityFaceTrackers.dll
├── SimulatedRealitySense.dll
├── shaders/
│   ├── VertexShader.hlsl
│   └── PixelShader.hlsl
└── README.txt                    ← Build info and instructions
```

### LeiaSplatViewer-windows-x64.zip

```
LeiaSplatViewer-windows-x64/
├── LeiaSplatViewer.exe          ← Main executable
├── SimulatedRealityCore.dll
├── SimulatedRealityDisplays.dll
├── SimulatedRealityWeaving.dll
├── shaders/
│   ├── PointCloud_VS.hlsl
│   ├── PointCloud_PS.hlsl
│   ├── Splat_VS.hlsl
│   ├── Splat_GS.hlsl
│   ├── Splat_PS.hlsl
│   └── SplatSort_CS.hlsl
└── README.txt                    ← Build info and instructions
```

---

## Running the Built Applications

### Prerequisites on Windows Machine

1. **Leia SR Platform Service** must be installed and running
2. **Leia SR display** must be connected
3. **Windows 10/11 x64** operating system

### Steps

1. Download and extract the artifact ZIP
2. Verify **all DLL files** are present
3. Ensure **Leia SR Platform Service** is running
4. Double-click the `.exe` file
5. **LookAroundEyesApp**: Press Tab to switch tracking methods
6. **LeiaSplatViewer**: Press O to load a `.ply` file

---

## Manual Trigger

You can also trigger a build manually without pushing code:

1. Go to **Actions** tab
2. Click **"Build Windows Applications"** workflow
3. Click **"Run workflow"** button (top right)
4. Select branch (usually `main`)
5. Click **"Run workflow"**

---

## Workflow Details

### When It Runs

- ✅ Automatically on every push to `main`
- ✅ Manually via "Run workflow" button
- ❌ Does NOT run on pull requests (to save build minutes)

### Build Process

1. **Checkout code** - Download repository
2. **Setup MSBuild** - Configure Visual Studio build tools
3. **Build LookAroundEyesApp** - Compile x64 Release
4. **Build LeiaSplatViewer** - Compile x64 Release
5. **Package artifacts** - Copy .exe, DLLs, shaders to ZIP
6. **Upload artifacts** - Make available for download

### Build Time

- **Typical**: 5-10 minutes
- **First run**: May take longer (caching dependencies)

---

## Artifact Retention

- **Retention period**: 90 days
- **Storage**: Free (within GitHub's limits)
- **Auto-deletion**: Artifacts older than 90 days are automatically removed

If you need permanent storage, download and archive the ZIPs elsewhere.

---

## Troubleshooting

### Build Failed - "Cannot find SDK"

**Cause**: Leia SR SDK not committed to repository

**Solution**:
```bash
# Check if SDK is tracked
git status simulatedreality-SDK-1.34.8.31132-win64-Release/

# If untracked, add it:
git add simulatedreality-SDK-1.34.8.31132-win64-Release/
git commit -m "Add Leia SR SDK for CI builds"
git push
```

### Build Failed - "MSBuild errors"

**Check the workflow logs:**
1. Click on the failed workflow run
2. Click on the "Build LookAroundEyesApp" or "Build LeiaSplatViewer" step
3. Expand the logs to see detailed error messages
4. Fix the code issue locally
5. Push the fix

### No Artifacts Appear

**Possible causes:**
- Build failed (check logs)
- Still building (wait for completion)
- Artifacts expired (rebuild)

**Solution**: Re-run the workflow or push a new commit

### Downloaded ZIP Won't Extract

**Cause**: Browser may have blocked download

**Solution**:
- Use a different browser
- Check your downloads folder for `.zip` file
- Use 7-Zip or WinRAR if Windows Extractor fails

---

## GitHub Free Tier Limits

GitHub Actions is free for public repositories with these limits:

- **Storage**: 500 MB artifact storage
- **Bandwidth**: Unlimited downloads
- **Build time**: 2,000 minutes/month (plenty for this project)

For private repositories:
- **Build time**: 2,000 minutes/month (free tier)
- **Storage**: 500 MB

Each build uses ~5-10 minutes, so you can do **200-400 builds per month** on the free tier.

---

## Advanced: Customizing the Workflow

### Build Only One App

Edit `.github/workflows/build-windows.yml`:

```yaml
# Comment out the app you don't want to build:
# - name: Build LookAroundEyesApp (x64 Release)
#   run: ...
```

### Change Artifact Retention

```yaml
- name: Upload LookAroundEyesApp build
  uses: actions/upload-artifact@v4
  with:
    name: LookAroundEyesApp-windows-x64
    path: artifacts/LookAroundEyesApp/
    retention-days: 30  # Change this (1-90 days)
```

### Build on Pull Requests

```yaml
on:
  push:
    branches: [ main ]
  pull_request:  # Add this
    branches: [ main ]
```

---

## Comparison: GitHub Actions vs Local Build

| Aspect | GitHub Actions | Local Visual Studio |
|--------|----------------|---------------------|
| **Cost** | Free (public repos) | Requires Windows license |
| **Platform** | Any (Mac/Linux/Windows) | Windows only |
| **Setup** | One-time workflow | Install VS, SDK |
| **Build time** | 5-10 min | 1-2 min |
| **Automation** | Automatic on push | Manual |
| **Testing** | Can't run (no SR display) | Full testing |

**Best practice**: Use GitHub Actions for **building**, local Windows for **testing**.

---

## Integration with Development Workflow

### Typical Workflow

1. **Develop on Mac** (edit code in VS Code, etc.)
2. **Push to GitHub** (git push origin main)
3. **GitHub Actions builds** (automatic)
4. **Download artifacts** (from Actions tab)
5. **Test on Windows** (with Leia SR display)
6. **Iterate** (repeat 1-5)

### No Need for Windows During Development!

You can:
- Write all code on Mac
- Test compilation via GitHub Actions
- Only use Windows for final testing with hardware

---

## Status Badge

Add build status to your README:

```markdown
![Build Status](https://github.com/USERNAME/REPO/actions/workflows/build-windows.yml/badge.svg)
```

Replace `USERNAME` and `REPO` with your GitHub username and repository name.

---

## Frequently Asked Questions

### Q: Can I build Debug instead of Release?

**A**: Yes, edit the workflow:
```yaml
/p:Configuration=Debug `
```

### Q: How do I know which commit was built?

**A**: Each artifact includes a `README.txt` with build timestamp and commit SHA.

### Q: Can I download artifacts via command line?

**A**: Yes, using GitHub CLI:
```bash
# Install GitHub CLI
brew install gh

# Download latest artifact
gh run download
```

### Q: What if my repository is private?

**A**: GitHub Actions still works! You get 2,000 free minutes per month.

### Q: Can I build for other platforms?

**A**: No, these are Windows-specific DirectX applications. They can only be built for Windows x64.

---

## Next Steps

1. ✅ Push your code to GitHub
2. ✅ Watch the build in Actions tab
3. ✅ Download the artifacts
4. ✅ Test on Windows machine with Leia display
5. ✅ Iterate and improve!

---

**You now have fully automated Windows builds from your Mac! 🎉**

For issues, check the workflow logs in the Actions tab or review the MSBuild output.
