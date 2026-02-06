# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

laser-tag-2026 is an interactive art installation built with **openFrameworks** (C++). It uses computer vision to track laser pointers in real-time, allowing users to "paint" on projected surfaces using lasers as brushes.

## Build Commands

See **[BUILD-INSTRUCTIONS.md](BUILD-INSTRUCTIONS.md)** for detailed platform-specific instructions (macOS, Windows, Linux).

```bash
# Build (default/debug)
make

# Build release
make Release

# Clean build artifacts
make clean

# Alternative: Open in Xcode
open laser-tag-2026.xcodeproj
```

## Build Status

**✅ Builds successfully** with openFrameworks 0.12.1 and patched ofxGuiExtended addon.

### Patches Applied (ofxGuiExtended)
- C++ template two-phase lookup (added `this->` prefix for dependent name lookup)
- OF 0.12 API changes (`ofPath.setFillColor()` now requires `ofFloatColor`)
- C++20 idiomatic `file_clock::to_sys()` for filesystem time conversion
- Missing `std::` prefix on `noskipws`
- Added missing includes (`ofPath.h`, `ofVboMesh.h`)

### Patches Applied (laser-tag-2026)
- Made `baseBrush::getTexture()` pure virtual

## First-Time Setup

### 1. Clone with Submodules

```bash
git clone --recursive https://github.com/LeonFedotov/L.A.S.E.R.-TAG-GRL.git
cd L.A.S.E.R.-TAG-GRL/laser-tag-2026
```

If already cloned without `--recursive`:
```bash
git submodule update --init --recursive
```

### 2. Download openFrameworks 0.12.1

```bash
mkdir -p lib && cd lib
curl -LO https://github.com/openframeworks/openFrameworks/releases/download/0.12.1/of_v0.12.1_osx_release.tar.gz
tar -xzf of_v0.12.1_osx_release.tar.gz && rm of_v0.12.1_osx_release.tar.gz
cd ..
```

### 3. Build

```bash
make -j16
```

### Quick Copy-Paste Setup

```bash
git clone --recursive https://github.com/LeonFedotov/L.A.S.E.R.-TAG-GRL.git && \
cd L.A.S.E.R.-TAG-GRL/laser-tag-2026 && \
mkdir -p lib && cd lib && \
curl -LO https://github.com/openframeworks/openFrameworks/releases/download/0.12.1/of_v0.12.1_osx_release.tar.gz && \
tar -xzf of_v0.12.1_osx_release.tar.gz && rm of_v0.12.1_osx_release.tar.gz && \
cd .. && make -j16
```

## Version Pinning

| Component | Version | Source |
|-----------|---------|--------|
| openFrameworks | **0.12.1** | [GitHub Release](https://github.com/openframeworks/openFrameworks/releases/tag/0.12.1) |
| ofxGuiExtended | master | [frauzufall/ofxGuiExtended](https://github.com/frauzufall/ofxGuiExtended) (submodule) |
| ofxCv | master | [kylemcdonald/ofxCv](https://github.com/kylemcdonald/ofxCv) (submodule) |

**OF path**: Configured in `config.make` via `OF_ROOT = lib/of_v0.12.1_osx_release`.

## Running

```bash
./bin/laser-tag-2026.app/Contents/MacOS/laser-tag-2026
```

The app creates two windows:
- **Main window** (1280x800): GUI controls and camera preview
- **Projector window** (1280x720): Output for projection

## Architecture

```
Camera Input → HSV Color Tracking → Blob Detection → Coordinate Warping → Brush Rendering → Projection Output
                                                                              ↓
                                                                    Network Stream (UDP/TCP)
```

### Source Structure

- **`src/app/`** - Main controller (`appController`) orchestrates all components
- **`src/dataIn/`** - Input processing: laser tracking via OpenCV blob detection, coordinate warping
- **`src/dataOut/`** - Output rendering: projection, drip effects, brush implementations
- **`src/dataOut/brushes/`** - Four brush types inheriting from `baseBrush`:
  - `vectorBrush` - OpenGL-based drawing modes
  - `pngBrush` - Custom PNG letter stamps
  - `graffLetter` - 3D bubble letters
  - `gestureBrush` - Gesture-based painting
- **`src/utils/`** - Color management utilities

### Key Classes

- `appController` - Central orchestrator, owns all subsystems
- `laserTracking` - HSV-based laser detection using OpenCV
- `imageProjection` - Projection output with quadrilateral warping
- `drips` - Paint drip effect with gravity simulation

## Configuration

Settings persist to XML files in `bin/data/settings/`:
- `settings.xml` - Main app settings (tracking params, brush config, network)
- `quad.xml` / `quadProj.xml` - Warping calibration points
- `colors.xml` - Color palette definitions

## openFrameworks Addons

Listed in `addons.make`:
- ofxOpenCv, ofxCv - Computer vision
- ofxGuiExtended - GUI controls
- ofxOsc - Network streaming
- ofxXmlSettings - Configuration persistence

## Code Conventions

- **Naming**: camelCase files (`appController.cpp`), PascalCase classes
- **Booleans**: `b` prefix (`bSetupCamera`, `bInverted`)
- **Constants**: `UPPER_CASE` (`NUM_BRUSHES`, `STATUS_SHOW_TIME`)
- **Pattern**: One class per .cpp/.h pair

## Testing

No automated test suite. Test using:
- Live camera input
- Test videos from `bin/data/videos/`

## Context Management

**IMPORTANT**: Monitor context usage during sessions. When context reaches 10%:
1. `/create_handoff` - Save current work state
2. `/compact` - Clear context
3. `/resume_handoff` - Continue from saved state

This ensures continuity across context limits.
