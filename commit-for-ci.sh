#!/bin/bash

# Helper script to commit everything for GitHub Actions
# Run this from your Mac after reviewing the changes

set -e

echo "=================================================="
echo "GitHub Actions Setup - Initial Commit"
echo "=================================================="
echo ""

# Change to repo directory
cd "$(dirname "$0")"

echo "Current directory: $(pwd)"
echo ""

# Check git status
echo "📊 Checking git status..."
git status --short
echo ""

# Show what will be added
echo "📦 Files to be committed:"
echo ""
echo "✅ GitHub Actions workflow (.github/workflows/build-windows.yml)"
echo "✅ Git ignore configuration (.gitignore)"
echo "✅ Documentation (*.md files)"
echo "✅ LookAroundEyesApp (source, shaders, project)"
echo "✅ LeiaSplatViewer (source, shaders, project)"
echo "✅ Leia SR SDK (~32 MB)"
echo ""

# Ask for confirmation
read -p "❓ Do you want to commit and push these files? (y/n): " -n 1 -r
echo ""

if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    echo "❌ Aborted. No changes made."
    exit 1
fi

echo ""
echo "📝 Committing files..."
echo ""

# Add GitHub Actions
git add .github/

# Add git ignore
git add .gitignore

# Add documentation
git add *.md

# Add applications
git add LookAroundEyesApp/
git add LeiaSplatViewer/

# Add SDK (this might take a while)
echo "⏳ Adding Leia SR SDK (this may take a minute)..."
git add simulatedreality-SDK-1.34.8.31132-win64-Release/

# Commit
git commit -m "Add GitHub Actions CI/CD for Windows builds

- Add workflow for building LookAroundEyesApp and LeiaSplatViewer
- Configure automatic builds on push to main
- Include Leia SR SDK for CI builds
- Add comprehensive documentation for GitHub Actions usage

Applications:
- LookAroundEyesApp: Eye tracking comparison tool
- LeiaSplatViewer: Gaussian Splat viewer

Build artifacts will be available as downloadable ZIPs from Actions tab."

echo ""
echo "✅ Committed successfully!"
echo ""
echo "🚀 Pushing to GitHub..."
git push origin main

echo ""
echo "=================================================="
echo "✅ SUCCESS!"
echo "=================================================="
echo ""
echo "🎉 Your code is now on GitHub!"
echo ""
echo "Next steps:"
echo "  1. Go to: https://github.com/$(git config --get remote.origin.url | sed 's/.*github.com[:/]\(.*\)\.git/\1/')"
echo "  2. Click the 'Actions' tab"
echo "  3. Watch your build run (~5-10 minutes)"
echo "  4. Download artifacts when complete"
echo ""
echo "📚 For more info, see: .github/BUILD_WITH_ACTIONS.md"
echo ""
