#!/bin/bash
# Local build script for laser-tag-2026 on Ubuntu 25.10 (Questing Quokka)
# This produces a native deb that works without adding external repos

set -e

echo "=== Building laser-tag-2026 for Ubuntu 25.10 ==="

# Check if we're in the right directory
if [ ! -f "Makefile" ]; then
    echo "Error: Run this script from the laser-tag-2026 directory"
    exit 1
fi

# Step 1: Install system dependencies
echo ""
echo "=== Step 1: Installing system dependencies ==="
# Allow update to fail (e.g., broken PPAs) - packages may still install from cache
sudo apt-get update || echo "Warning: apt-get update had errors, continuing anyway..."
sudo apt-get install -y \
    build-essential \
    binutils-gold \
    pkg-config \
    git \
    curl \
    libunwind-dev \
    libopencv-dev \
    libglfw3-dev \
    libfreeimage-dev \
    libpugixml-dev \
    liburiparser-dev \
    libgstreamer1.0-dev \
    libgstreamer-plugins-base1.0-dev \
    libasound2-dev \
    libpulse-dev \
    libgtk-3-dev \
    libcurl4-openssl-dev \
    libgl-dev \
    libglu1-mesa-dev \
    libopenal-dev \
    libmpg123-dev \
    libsndfile1-dev \
    libboost-filesystem-dev

# Step 2: Download openFrameworks if not present
echo ""
echo "=== Step 2: Setting up openFrameworks ==="
mkdir -p lib
cd lib

if [ ! -d "of_v0.12.1_linux64_gcc6_release" ]; then
    echo "Downloading openFrameworks 0.12.1..."
    curl -sfSL -o of.tar.gz https://github.com/openframeworks/openFrameworks/releases/download/0.12.1/of_v0.12.1_linux64_gcc6_release.tar.gz
    tar -xzf of.tar.gz
    rm of.tar.gz
else
    echo "openFrameworks already present, skipping download"
fi

cd ..

# Step 2b: Setup local_addons (project expects addons here, not in OF directory)
echo ""
echo "=== Step 2b: Setting up local_addons ==="

# Check if ofxGuiExtended needs to be cloned (empty submodule or missing)
if [ ! -f "local_addons/ofxGuiExtended/src/ofxGuiExtended.h" ]; then
    echo "Cloning ofxGuiExtended..."
    rm -rf local_addons/ofxGuiExtended
    git clone --depth 1 https://github.com/LeonFedotov/ofxGuiExtended.git -b of-0.12-compatibility local_addons/ofxGuiExtended
else
    echo "ofxGuiExtended already present"
fi

# Check if ofxCv needs to be cloned (empty submodule or missing)
if [ ! -f "local_addons/ofxCv/libs/ofxCv/include/ofxCv.h" ]; then
    echo "Cloning ofxCv..."
    rm -rf local_addons/ofxCv
    git clone --depth 1 https://github.com/kylemcdonald/ofxCv.git local_addons/ofxCv
else
    echo "ofxCv already present"
fi

# Install OF dependencies
echo "Installing openFrameworks dependencies..."
cd lib/of_v0.12.1_linux64_gcc6_release/scripts/linux/ubuntu
yes | sudo ./install_dependencies.sh || true
cd ../../../../..

# Step 3: Configure for Linux
echo ""
echo "=== Step 3: Configuring build ==="
sed -i 's|OF_ROOT = lib/of_v0.12.1_osx_release|OF_ROOT = lib/of_v0.12.1_linux64_gcc6_release|' config.make

# Step 4: Apply patches
echo ""
echo "=== Step 4: Applying patches ==="

# Patch ofxCv for GCC
if [ -f "local_addons/ofxCv/libs/ofxCv/include/ofxCv/Tracker.h" ]; then
    sed -i 's/Tracker<T>()/Tracker()/' local_addons/ofxCv/libs/ofxCv/include/ofxCv/Tracker.h
fi

# Patch OF for missing memory header
if [ -f "lib/of_v0.12.1_linux64_gcc6_release/libs/openFrameworks/types/ofTypes.h" ]; then
    if ! grep -q "#include <memory>" lib/of_v0.12.1_linux64_gcc6_release/libs/openFrameworks/types/ofTypes.h; then
        sed -i '1i #include <memory>' lib/of_v0.12.1_linux64_gcc6_release/libs/openFrameworks/types/ofTypes.h
    fi
fi

# Patch GLFW for headless/X11 operation (GLFW 3.4+ requires explicit platform hint)
GLFW_WINDOW_FILE="lib/of_v0.12.1_linux64_gcc6_release/libs/openFrameworks/app/ofAppGLFWWindow.cpp"
if [ -f "$GLFW_WINDOW_FILE" ]; then
    if ! grep -q "GLFW_PLATFORM_X11" "$GLFW_WINDOW_FILE"; then
        echo "Patching ofAppGLFWWindow.cpp for GLFW 3.4+ X11 platform support..."
        sed -i '/if (!glfwInit()) {/i\
#if GLFW_VERSION_MAJOR >= 3 \&\& GLFW_VERSION_MINOR >= 4\
        glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);\
#endif' "$GLFW_WINDOW_FILE"
    fi
fi

# Step 5: Build
echo ""
echo "=== Step 5: Building ==="
make Release -j$(nproc)

# Step 6: Create Debian package
echo ""
echo "=== Step 6: Creating Debian package ==="
VERSION=$(date +%Y.%m.%d)
PKG_DIR=laser-tag-2026_${VERSION}_amd64

# Create debian package structure
mkdir -p ${PKG_DIR}/DEBIAN
mkdir -p ${PKG_DIR}/usr/bin
mkdir -p ${PKG_DIR}/usr/share/laser-tag-2026
mkdir -p ${PKG_DIR}/usr/share/applications
mkdir -p ${PKG_DIR}/usr/share/icons/hicolor/256x256/apps

# Copy binary and data
cp bin/laser-tag-2026 ${PKG_DIR}/usr/bin/
cp -r bin/data ${PKG_DIR}/usr/share/laser-tag-2026/

# Create wrapper script
cat > ${PKG_DIR}/usr/bin/laser-tag-2026-launcher << 'LAUNCHER'
#!/bin/bash
cd /usr/share/laser-tag-2026
exec /usr/bin/laser-tag-2026 "$@"
LAUNCHER
chmod +x ${PKG_DIR}/usr/bin/laser-tag-2026-launcher

# Create desktop entry
cat > ${PKG_DIR}/usr/share/applications/laser-tag-2026.desktop << 'DESKTOP'
[Desktop Entry]
Name=Laser Tag 2026
Comment=Interactive laser graffiti installation
Exec=laser-tag-2026-launcher
Terminal=false
Type=Application
Categories=Graphics;Art;
DESKTOP

# Detect OpenCV suffix dynamically (for Ubuntu 25.10 this will be "410")
OPENCV_SUFFIX=$(apt-cache search '^libopencv-core[0-9]' | head -1 | grep -oP 'libopencv-core\K[0-9.d]+')
echo "Detected OpenCV suffix: ${OPENCV_SUFFIX}"

# Create control file with correct dependencies for this system
cat > ${PKG_DIR}/DEBIAN/control << CONTROL
Package: laser-tag-2026
Version: ${VERSION}
Section: graphics
Priority: optional
Architecture: amd64
Depends: libopencv-core${OPENCV_SUFFIX}, libopencv-imgproc${OPENCV_SUFFIX}, libopencv-video${OPENCV_SUFFIX}, libopencv-videoio${OPENCV_SUFFIX}, libglfw3, libfreeimage3t64, libpugixml1v5, liburiparser1, libgstreamer1.0-0, libgstreamer-plugins-base1.0-0, libasound2t64, libpulse0, libgtk-3-0t64, libcurl4t64
Maintainer: GRL <info@graffitiresearchlab.com>
Description: Laser Tag 2026 - Interactive laser graffiti
 An interactive art installation that uses computer vision
 to track laser pointers, allowing users to paint on
 projected surfaces using lasers as brushes.
 Built locally for Ubuntu 25.10 (Questing Quokka).
CONTROL

# Build the package
dpkg-deb --build ${PKG_DIR}
mv ${PKG_DIR}.deb bin/laser-tag-2026-ubuntu25.10.deb

echo ""
echo "=== Build Complete ==="
echo "Debian package: bin/laser-tag-2026-ubuntu25.10.deb"
echo ""
echo "To install:"
echo "  sudo dpkg -i bin/laser-tag-2026-ubuntu25.10.deb"
echo "  sudo apt-get install -f"
echo ""
echo "To run:"
echo "  laser-tag-2026-launcher"
