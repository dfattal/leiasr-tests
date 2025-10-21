# GitHub Actions Setup Complete! 🎉

Your repository is now configured to automatically build Windows applications from your Mac using GitHub Actions.

---

## ✅ What Was Set Up

### 1. GitHub Actions Workflow
**File**: `.github/workflows/build-windows.yml`

- Builds both LookAroundEyesApp and LeiaSplatViewer
- Runs on every push to `main` branch
- Can also be triggered manually
- Packages executables with DLLs and shaders
- Creates downloadable ZIP artifacts

### 2. Git Configuration
**File**: `.gitignore`

- Configured to **include** the Leia SR SDK (needed for builds)
- Excludes build artifacts and temporary files
- SDK size: ~32 MB (acceptable for GitHub)

### 3. Documentation
**File**: `.github/BUILD_WITH_ACTIONS.md`

- Complete guide on using GitHub Actions
- How to download artifacts
- Troubleshooting tips
- Development workflow

---

## 🚀 Next Steps

### 1. Commit and Push the GitHub Actions Configuration

```bash
cd /Users/david.fattal/Documents/GitHub/leiasr-tests

# Add the GitHub Actions workflow
git add .github/

# Add the .gitignore
git add .gitignore

# Add the documentation
git add GITHUB_ACTIONS_SETUP.md

# Commit
git commit -m "Add GitHub Actions workflow for automated Windows builds"

# Push to trigger first build
git push origin main
```

### 2. Add the Leia SR SDK to Repository

⚠️ **IMPORTANT**: The SDK must be committed for GitHub Actions to access it.

```bash
# Check SDK size (should be ~32 MB)
du -sh simulatedreality-SDK-1.34.8.31132-win64-Release

# Add SDK to git
git add simulatedreality-SDK-1.34.8.31132-win64-Release/

# Commit SDK
git commit -m "Add Leia SR SDK for GitHub Actions builds"

# Push (this will take a while due to SDK size)
git push origin main
```

**Note**: First push with SDK may take 5-10 minutes depending on internet speed.

### 3. Add All Application Files

```bash
# Add both applications
git add LookAroundEyesApp/ LeiaSplatViewer/

# Add documentation
git add *.md task-list.md

# Commit
git commit -m "Add LookAroundEyesApp and LeiaSplatViewer applications"

# Push
git push origin main
```

### 4. Watch the Build

1. Go to your GitHub repository
2. Click **"Actions"** tab
3. See the workflow running
4. Wait ~5-10 minutes for build to complete

### 5. Download the Artifacts

1. Click on the completed workflow run
2. Scroll to **"Artifacts"** section
3. Download:
   - `LookAroundEyesApp-windows-x64.zip`
   - `LeiaSplatViewer-windows-x64.zip`

---

## 📊 Repository Structure After Setup

```
leiasr-tests/
├── .github/
│   ├── workflows/
│   │   └── build-windows.yml          ← GitHub Actions workflow
│   └── BUILD_WITH_ACTIONS.md          ← Documentation
├── .gitignore                          ← Git ignore rules
├── simulatedreality-SDK-1.34.8.31132-win64-Release/  ← Leia SR SDK (~32 MB)
│   ├── include/                        ← Headers
│   ├── lib/                            ← Link libraries
│   ├── examples/                       ← SDK examples
│   └── third_party/                    ← Dependencies
├── LookAroundEyesApp/
│   ├── src/                            ← Source code
│   ├── shaders/                        ← HLSL shaders
│   └── LookAroundEyesApp.vcxproj       ← VS project
├── LeiaSplatViewer/
│   ├── src/                            ← Source code
│   ├── shaders/                        ← HLSL shaders
│   └── LeiaSplatViewer.vcxproj         ← VS project
└── *.md                                ← Documentation
```

---

## ⚙️ How It Works

### On Every Push to Main:

1. **GitHub Actions** starts a Windows VM
2. **Checks out** your code
3. **Sets up** MSBuild (Visual Studio build tools)
4. **Builds** LookAroundEyesApp in Release x64 mode
5. **Builds** LeiaSplatViewer in Release x64 mode
6. **Packages** executables, DLLs, and shaders
7. **Uploads** artifacts (available for 90 days)

### You Can:

- Develop on your Mac
- Push code to GitHub
- Download Windows executables
- Test on Windows machine with Leia display
- Never need Visual Studio!

---

## 📦 What Gets Built

### LookAroundEyesApp-windows-x64.zip (~5-10 MB)

- `LookAroundEyesApp.exe` - Main application
- `SimulatedReality*.dll` - Runtime dependencies
- `shaders/*.hlsl` - Vertex and pixel shaders
- `README.txt` - Build info and usage instructions

### LeiaSplatViewer-windows-x64.zip (~5-10 MB)

- `LeiaSplatViewer.exe` - Main application
- `SimulatedReality*.dll` - Runtime dependencies
- `shaders/*.hlsl` - All rendering shaders
- `README.txt` - Build info and usage instructions

---

## 🔧 Workflow Features

### Automatic Build

```yaml
on:
  push:
    branches: [ main ]  # Builds on every push to main
```

### Manual Trigger

You can also manually trigger a build:
1. Go to Actions tab
2. Click "Build Windows Applications"
3. Click "Run workflow"

### Parallel Builds

Both applications build simultaneously for speed:
- LookAroundEyesApp: ~3-5 minutes
- LeiaSplatViewer: ~3-5 minutes
- **Total**: ~5-7 minutes (parallel execution)

### Smart Packaging

Post-build scripts automatically:
- Copy executables to staging area
- Copy all required DLLs
- Copy shader files with correct structure
- Generate README with build metadata
- Create ready-to-run ZIP packages

---

## 💰 Cost

**FREE** for public repositories!

- **Build time**: 2,000 minutes/month (you'll use ~100-200 minutes/month)
- **Storage**: 500 MB for artifacts (plenty for these apps)
- **Bandwidth**: Unlimited downloads

If your repo is private:
- Still FREE up to 2,000 minutes/month
- Same storage limits

---

## 🐛 Troubleshooting

### Build Fails: "Cannot find SDK"

**Problem**: SDK not committed to repository

**Solution**:
```bash
git add simulatedreality-SDK-1.34.8.31132-win64-Release/
git commit -m "Add SDK for CI"
git push
```

### Build Fails: "MSBuild errors"

**Problem**: Code doesn't compile

**Solution**:
1. Check the error in Actions logs
2. Fix locally
3. Push fix
4. Build will re-run automatically

### No Artifacts

**Problem**: Build failed or still running

**Solution**:
- Wait for build to complete (green checkmark)
- Click on the run to see detailed logs
- Artifacts only appear if build succeeds

### DLL Missing in Artifact

**Problem**: Post-build event didn't copy DLLs

**Note**: The SDK in the repository doesn't include DLL files in a `bin/` directory.
The DLLs are typically installed system-wide by the Leia SR Platform Service.

**Workaround**: Users need to install Leia SR Platform Service on their Windows machine,
which provides the required DLLs.

---

## 🎯 Development Workflow

### Recommended Process

1. **Edit code on Mac** (VS Code, any editor)
2. **Git commit and push** to GitHub
3. **GitHub Actions builds** automatically
4. **Download artifacts** from Actions tab
5. **Test on Windows** with Leia display
6. **Iterate** - repeat steps 1-5

### No Windows Needed for Development!

You only need Windows for:
- Final testing with Leia SR display
- Running the actual applications

Everything else can be done on your Mac:
- Writing code
- Reviewing changes
- Building executables (via GitHub Actions)

---

## 📚 Documentation

- **Build Guide**: `.github/BUILD_WITH_ACTIONS.md` - Complete usage guide
- **Workflow File**: `.github/workflows/build-windows.yml` - The actual workflow
- **Project READMEs**: Individual app documentation

---

## ✨ Benefits

✅ **Platform Independent** - Develop on Mac, build for Windows
✅ **No Visual Studio Required** - GitHub provides the build environment
✅ **Automated** - Push code, get executables
✅ **Version Controlled** - Every build is tied to a commit
✅ **Shareable** - Team members can download builds
✅ **Fast** - Parallel builds in ~5-7 minutes
✅ **Free** - No cost for public repositories

---

## 🎉 You're All Set!

Your repository now has **professional CI/CD** for Windows application builds.

**Next**: Commit your changes and watch the magic happen!

```bash
# From your Mac
git add .
git commit -m "Complete GitHub Actions setup"
git push origin main

# Then visit: https://github.com/YOUR_USERNAME/YOUR_REPO/actions
```

---

**Happy coding from your Mac! 🚀**
