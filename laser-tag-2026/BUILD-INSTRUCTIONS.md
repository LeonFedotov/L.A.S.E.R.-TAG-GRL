# Build Instructions

This document covers building laser-tag-2026 on macOS, Windows, and Linux.

## Prerequisites

- **openFrameworks 0.12.1** or nightly build
- **C++ compiler** (Xcode on macOS, Visual Studio on Windows, GCC on Linux)
- **Git** for cloning addons

## Quick Start (macOS)

```bash
# 1. Install openFrameworks
mkdir -p lib && cd lib
curl -LO https://github.com/openframeworks/openFrameworks/releases/download/0.12.1/of_v0.12.1_osx_release.tar.gz
tar -xzf of_v0.12.1_osx_release.tar.gz && rm of_v0.12.1_osx_release.tar.gz
mv of_v0.12.1_osx_release of

# 2. Install third-party addons
cd of/addons
git clone https://github.com/frauzufall/ofxGuiExtended.git
git clone https://github.com/kylemcdonald/ofxCv.git

# 3. Build
cd ../../..  # back to laser-tag-2026
make Release

# 4. Run
./bin/laser-tag-2026.app/Contents/MacOS/laser-tag-2026
```

## Platform-Specific Instructions

### macOS

**Requirements:**
- Xcode Command Line Tools
- openFrameworks 0.12.1 macOS release

**Build Options:**
```bash
make           # Debug build
make Release   # Release build
make clean     # Clean build artifacts
```

**Alternative (Xcode):**
```bash
open laser-tag-2026.xcodeproj
```

**Known Issues:**
- Audio/video conflict: ofSoundPlayer and ofVideoPlayer both use AVFoundation on macOS. When video is active, audio may not play. This is an upstream openFrameworks limitation.

### Windows

**Requirements:**
- Visual Studio 2019 or 2022
- openFrameworks 0.12.1 VS release

**Setup:**
1. Download [of_v0.12.1_vs_64_release.zip](https://github.com/openframeworks/openFrameworks/releases/download/0.12.1/of_v0.12.1_vs_64_release.zip)
2. Extract to `C:\of\` or your preferred location
3. Clone addons into `C:\of\addons\`:
   ```cmd
   cd C:\of\addons
   git clone https://github.com/frauzufall/ofxGuiExtended.git
   git clone https://github.com/kylemcdonald/ofxCv.git
   ```
4. Run **Project Generator** (`projectGenerator-vs/projectGenerator.exe`)
5. Select the laser-tag-2026 folder and generate the VS project
6. Open `laser-tag-2026.sln` in Visual Studio
7. Build Release x64

**Audio on Windows:**
Audio should work correctly on Windows since ofSoundPlayer uses DirectShow/FMOD instead of AVFoundation.

### Linux

**Requirements:**
- GCC/G++
- openFrameworks 0.12.1 Linux release
- OpenGL development libraries

**Setup:**
```bash
# 1. Install OF dependencies (Ubuntu/Debian)
cd of/scripts/linux/ubuntu
sudo ./install_dependencies.sh

# 2. Install addons
cd ../../addons
git clone https://github.com/frauzufall/ofxGuiExtended.git
git clone https://github.com/kylemcdonald/ofxCv.git

# 3. Build
cd /path/to/laser-tag-2026
make Release
```

## Required Addons

Listed in `addons.make`:

| Addon | Description | Source |
|-------|-------------|--------|
| ofxOpenCv | OpenCV bindings | Built-in |
| ofxCv | Kyle McDonald's CV utilities | [github](https://github.com/kylemcdonald/ofxCv) |
| ofxGuiExtended | Extended GUI controls | [github](https://github.com/frauzufall/ofxGuiExtended) |
| ofxOsc | OSC networking | Built-in |
| ofxXmlSettings | XML config | Built-in |

## Configuration

The app looks for configuration files in `bin/data/settings/`:
- `settings.xml` - Main settings
- `quad.xml` / `quadProj.xml` - Warping calibration
- `colors.xml` - Color palette

## Troubleshooting

### Addon Errors
If you get errors about missing addon files, ensure:
1. Addons are in the correct location (`of/addons/` or `local_addons/`)
2. The `OF_ROOT` path in `config.make` points to your OF installation

### Template Lookup Errors (ofxGuiExtended)
The upstream ofxGuiExtended now includes OF 0.12 compatibility fixes. If you're using an older version and see errors like "member access into incomplete type", this is a C++ two-phase lookup issue with templates that has been fixed in the current version. Update the submodule or apply the fix:
```cpp
// Change: param->getName()
// To:     this->param->getName()
```

### Audio Not Playing (macOS)
This is the known AVFoundation conflict. Workarounds:
1. Disable video playback when audio is needed
2. Use external audio playback
3. Build for Windows where this conflict doesn't exist

## CI/CD

The project includes GitHub Actions workflows:
- **Build Standalone** - Web version deployment
- **Windows Build** - Windows executable build (tests audio compatibility)

## Pre-built Releases

Pre-built macOS binaries are available in [GitHub Releases](https://github.com/LeonFedotov/L.A.S.E.R.-TAG-GRL/releases).
