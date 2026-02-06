# Copilot Instructions for L.A.S.E.R. TAG

## Project Overview

L.A.S.E.R. TAG is an interactive art installation project by the Graffiti Research Lab that allows users to "paint" on projected surfaces using laser pointers. The project consists of two main components:

1. **laser-tag-2026**: A C++ application built with openFrameworks (v0.12.1) for real-time laser tracking and projection
2. **laser-tag-web**: A browser-based implementation using JavaScript, Vite, and OpenCV.js

## Build & Test Commands

### laser-tag-2026 (C++/openFrameworks)
```bash
# Navigate to the project
cd laser-tag-2026

# Build (Debug)
make

# Build (Release)
make Release

# Clean build artifacts
make clean

# Alternative: Open in Xcode (macOS)
open laser-tag-2026.xcodeproj

# Run the application
./bin/laser-tag-2026.app/Contents/MacOS/laser-tag-2026
```

**Prerequisites:**
- openFrameworks 0.12.1 installed in `lib/of_v0.12.1_osx_release/`
- Required addons: ofxCv, ofxGuiExtended (see BUILD-INSTRUCTIONS.md)

### laser-tag-web (JavaScript/Vite)
```bash
# Navigate to the project
cd laser-tag-web

# Install dependencies
npm ci

# Development server
npm run dev

# Build for production
npm run build

# Run tests
npm run test

# Run tests in watch mode
npm run test:watch
```

## Architecture

### laser-tag-2026 (C++)
```
Camera Input → HSV Color Tracking → Blob Detection → Coordinate Warping → Brush Rendering → Projection Output
                                                                               ↓
                                                                     Network Stream (UDP/TCP)
```

**Key directories:**
- `src/app/` - Main controller (`appController`) orchestrates all components
- `src/dataIn/` - Input processing: laser tracking, coordinate warping
- `src/dataOut/` - Output rendering: projection, drip effects, brushes
- `src/dataOut/brushes/` - Four brush types (vectorBrush, pngBrush, graffLetter, gestureBrush)
- `src/utils/` - Utility classes and helpers

**Key classes:**
- `appController` - Central orchestrator
- `laserTracking` - HSV-based laser detection using OpenCV
- `imageProjection` - Projection output with quadrilateral warping
- `drips` - Paint drip effect with gravity simulation

### laser-tag-web (JavaScript)
**Key directories:**
- `src/app/` - Application setup and initialization
- `src/tracking/` - Laser detection and tracking using OpenCV.js
- `src/rendering/` - Canvas rendering and effects
- `src/brushes/` - Brush implementations
- `src/calibration/` - Calibration utilities
- `src/effects/` - Visual effects (drips, etc.)
- `src/utils/` - Utility functions

## Code Conventions

### C++ (laser-tag-2026)
- **File naming**: camelCase (e.g., `appController.cpp`)
- **Class naming**: PascalCase (e.g., `AppController`)
- **Boolean prefixes**: Use `b` prefix (e.g., `bSetupCamera`, `bInverted`)
- **Constants**: `UPPER_CASE` with underscores (e.g., `NUM_BRUSHES`, `STATUS_SHOW_TIME`)
- **Pattern**: One class per .cpp/.h pair
- **Namespacing**: Minimal use of explicit namespaces; rely on std:: prefix where needed

### JavaScript (laser-tag-web)
- **File naming**: camelCase for modules, PascalCase for classes
- **Module type**: ES6 modules (`type: "module"` in package.json)
- **Formatting**: Standard JavaScript conventions
- **Testing**: Vitest for unit tests, jsdom for DOM testing

## Configuration Files

### laser-tag-2026
Configuration files in `bin/data/settings/`:
- `settings.xml` - Main app settings (tracking params, brush config, network)
- `quad.xml` / `quadProj.xml` - Warping calibration points
- `colors.xml` - Color palette definitions

### laser-tag-web
- `vite.config.js` - Vite build configuration
- `vitest.config.js` - Test configuration
- `package.json` - Dependencies and scripts

## Dependencies & Version Management

### openFrameworks (laser-tag-2026)
- **Version**: 0.12.1 (pinned)
- **Location**: `lib/of_v0.12.1_osx_release/`
- **Addons** (listed in `addons.make`):
  - `ofxOpenCv` - OpenCV bindings (built-in)
  - `ofxCv` - Kyle McDonald's CV utilities (submodule)
  - `ofxGuiExtended` - Extended GUI controls (submodule)
  - `ofxOsc` - OSC networking (built-in)
  - `ofxXmlSettings` - XML config (built-in)

**Note**: ofxGuiExtended now includes OF 0.12 compatibility fixes in upstream (merged from LeonFedotov/ofxGuiExtended PR #80). The submodule uses the upstream frauzufall/ofxGuiExtended repository.

### JavaScript (laser-tag-web)
Key dependencies:
- `@techstark/opencv-js` - OpenCV for browser
- `gl-matrix` - WebGL matrix operations
- `tweakpane` / `lil-gui` - GUI controls
- `kalman-filter` - Smoothing algorithm
- `vite` - Build tool
- `vitest` - Testing framework

## Common Patterns

### When Adding New Features
1. Check existing implementations for similar functionality
2. Follow the architectural pattern (input → processing → output)
3. Add configuration options to XML settings (C++) or appropriate config (JS)
4. Update relevant documentation (BUILD-INSTRUCTIONS.md for build changes)
5. Test with both live camera input and test videos (C++) or in browser (JS)

### Error Handling
- C++: Use ofLog() for logging (ofLogError, ofLogWarning, ofLogNotice)
- JavaScript: Use console methods appropriately

### Git Submodules
This repository uses submodules for third-party addons:
```bash
# Initialize submodules after cloning
git submodule update --init --recursive
```

## CI/CD

GitHub Actions workflows in `.github/workflows/`:
- `build.yml` - Builds laser-tag-web and creates releases
- `deploy.yml` - Deployment workflow
- `release.yml` - Release management

## Platform-Specific Notes

### macOS
- Builds successfully with Xcode Command Line Tools
- Known issue: Audio/video conflict with AVFoundation (see BUILD-INSTRUCTIONS.md)

### Windows
- Use Visual Studio 2019 or 2022
- Audio works correctly (uses DirectShow/FMOD instead of AVFoundation)
- May require Project Generator for initial setup

### Linux
- Requires OpenGL development libraries
- Run `of/scripts/linux/ubuntu/install_dependencies.sh` first

## Testing

### laser-tag-2026
- No automated test suite
- Manual testing required with:
  - Live camera input
  - Test videos from `bin/data/videos/`

### laser-tag-web
```bash
npm run test        # Run all tests
npm run test:watch  # Watch mode for development
```

## Documentation References

- **BUILD-INSTRUCTIONS.md** - Detailed platform-specific build instructions
- **CLAUDE.md** - Additional context for AI assistants (build status, patches, architecture)
- **README.md** - Project overview and quick links
- **laser-tag-web/README.md** - Web version specific documentation

## Common Issues & Solutions

### ofxGuiExtended Template Errors
Apply two-phase lookup fix: Change `param->getName()` to `this->param->getName()`

### Missing Addons
Ensure addons are in correct location (`of/addons/` or `local_addons/`) and `OF_ROOT` in `config.make` points to your OF installation.

### Audio Not Playing (macOS, laser-tag-2026)
Known AVFoundation conflict. Workarounds:
1. Disable video playback when audio is needed
2. Use external audio playback
3. Build for Windows where this conflict doesn't exist

## Security Notes

- Don't commit secrets or sensitive data
- Configuration files may contain sensitive settings - review before committing
- Network streaming features (OSC) should be properly configured for production use
