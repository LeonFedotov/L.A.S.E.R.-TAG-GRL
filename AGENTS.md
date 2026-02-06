# AGENTS.md

This file provides guidance to AI coding agents (including GitHub Copilot) when working with this repository.

## Quick Reference

| Component | Language | Build Command | Test Command |
|-----------|----------|---------------|--------------|
| laser-tag-2026 | C++/openFrameworks | `make Release` | Manual testing |
| laser-tag-web | JavaScript/Vite | `npm run build` | `npm run test` |

## Project Context

**L.A.S.E.R. TAG** by Graffiti Research Lab is an interactive art installation that enables laser pointer graffiti on projected surfaces through real-time computer vision tracking.

### Two Implementations

1. **laser-tag-2026** (Desktop): Professional C++ application using openFrameworks for live installations
2. **laser-tag-web** (Browser): Standalone web version using JavaScript and OpenCV.js

## Agent Guidelines

### Before Making Changes

1. **Understand the component**: This repo has two distinct projects. Identify which one you're working on.
2. **Check existing documentation**: Refer to BUILD-INSTRUCTIONS.md and CLAUDE.md for build context.
3. **Review similar code**: Look for existing patterns before implementing new features.
4. **Test your assumptions**: Both projects support test videos/scenarios for validation.

### Making Code Changes

#### For laser-tag-2026 (C++)
- **Always build before committing**: `make clean && make Release`
- **Respect openFrameworks patterns**: Use ofLog(), follow OF lifecycle methods (setup, update, draw)
- **One class per file**: Follow the existing pattern of matching .cpp/.h files
- **Prefix booleans with 'b'**: e.g., `bIsActive`, `bShowDebug`

#### For laser-tag-web (JavaScript)
- **Run tests**: Always run `npm run test` before committing
- **Use ES6 modules**: This project uses modern JavaScript with `type: "module"`
- **Build to verify**: Run `npm run build` to catch build-time issues
- **Follow existing structure**: Match the directory organization (tracking/, rendering/, brushes/, etc.)

### Version Constraints

**Critical**: Don't upgrade openFrameworks beyond 0.12.1 without extensive testing. The project uses:
- Specific OF 0.12.1 APIs
- ofxGuiExtended with OF 0.12 compatibility (now upstream)
- Version-pinned submodules (ofxCv, ofxGuiExtended)

For JavaScript dependencies, follow semantic versioning but test thoroughly due to OpenCV.js compatibility requirements.

### Configuration Management

Both projects use persistent configuration:
- **C++**: XML files in `bin/data/settings/` (settings.xml, quad.xml, colors.xml)
- **JavaScript**: Runtime configuration via GUI (Tweakpane/lil-gui)

When adding configurable parameters:
1. Add to the appropriate settings class/object
2. Wire up to GUI controls
3. Ensure persistence (XML save/load for C++, localStorage for JS)
4. Document in relevant config files

### Debugging & Logging

**C++ (laser-tag-2026)**:
```cpp
ofLogNotice("ComponentName") << "Info message";
ofLogWarning("ComponentName") << "Warning message";
ofLogError("ComponentName") << "Error message";
```

**JavaScript (laser-tag-web)**:
```javascript
console.log('[ComponentName]', 'Info message');
console.warn('[ComponentName]', 'Warning message');
console.error('[ComponentName]', 'Error message');
```

### Architecture Respect

Both implementations follow a similar flow:
```
Input (Camera) → Detection (HSV/Blob) → Tracking → Warping → Rendering → Output (Projection)
```

When modifying any stage:
1. Understand dependencies on upstream/downstream components
2. Maintain the data flow contracts
3. Test the entire pipeline, not just your component
4. Consider performance implications (these are real-time systems)

### Common Pitfalls

1. **Breaking the build**: Always test your build before committing
   - C++: `make clean && make Release` (takes a few minutes)
   - JS: `npm run build && npm run test`

2. **Ignoring submodules**: C++ project uses git submodules
   ```bash
   git submodule update --init --recursive
   ```

3. **Platform assumptions**: C++ code must work on macOS, Windows, and Linux
   - Test with CI or document platform-specific limitations
   - Known issue: macOS AVFoundation audio/video conflict

4. **Missing OF setup**: C++ requires openFrameworks installed in `lib/` directory
   - See BUILD-INSTRUCTIONS.md for setup
   - `OF_ROOT` in config.make must point to correct location

5. **OpenCV.js loading**: Browser version requires proper async loading of OpenCV
   - Check existing initialization patterns in `src/app/`

### Testing Strategy

**laser-tag-2026 (C++)**:
- Manual testing required (no automated test suite)
- Test with live camera if available
- Use test videos from `bin/data/videos/`
- Verify both windows: GUI window and Projector window
- Test calibration/warping functionality

**laser-tag-web (JavaScript)**:
- Automated tests with Vitest: `npm run test`
- Manual browser testing: `npm run dev`
- Test in multiple browsers (Chrome, Firefox, Safari)
- Verify camera permissions and OpenCV loading

### File Organization Rules

**Don't commit**:
- Build artifacts (`bin/`, `obj/`, `dist/`, `node_modules/`)
- IDE-specific files (except those already tracked)
- Temporary test files
- Personal configuration overrides

**Always commit**:
- Source code changes
- Documentation updates
- Build script modifications
- Configuration schema changes (not personal settings)

### When Stuck

1. **Check CLAUDE.md**: Contains build status, known patches, and troubleshooting
2. **Check BUILD-INSTRUCTIONS.md**: Platform-specific setup and build help
3. **Review git history**: See how similar changes were made before
4. **Ask for clarification**: If requirements are unclear, ask before implementing

## Code Style Summary

### C++ Style
- camelCase files: `appController.cpp`
- PascalCase classes: `AppController`
- Boolean prefix: `bIsRunning`, `bDebugMode`
- Constants: `MAX_BRUSHES`, `DEFAULT_WIDTH`
- Indentation: Tabs (match OF convention)

### JavaScript Style  
- camelCase files and functions: `laserTracking.js`
- PascalCase classes: `LaserTracking`
- Modern ES6+: Use const/let, arrow functions, async/await
- Indentation: 2 spaces

## Contribution Workflow

1. **Explore**: Understand the change required
2. **Plan**: Identify minimal changes needed
3. **Build**: Verify build works before changes
4. **Implement**: Make focused, surgical changes
5. **Test**: Verify changes work as expected
6. **Build again**: Ensure build still succeeds
7. **Commit**: With clear, descriptive commit message

## Key Resources

- [openFrameworks Documentation](https://openframeworks.cc/documentation/)
- [OpenCV.js Documentation](https://docs.opencv.org/3.4/d5/d10/tutorial_js_root.html)
- [Vite Documentation](https://vitejs.dev/)
- [Vitest Documentation](https://vitest.dev/)

## Remember

This is an **art installation project** used in live performances. Code quality, reliability, and real-time performance are critical. Test thoroughly and maintain backward compatibility whenever possible.
