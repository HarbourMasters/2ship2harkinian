#!/usr/bin/env bash
# Cross-compile the audio codec libraries (ogg, vorbis, opus, opusfile) for iphoneos/arm64
# as static libraries into a single install prefix. These are required by the game's
# custom-audio importer (AudioSampleFactory) and are not provided by libultraship.
#
# Usage: build-ios-deps.sh <install-prefix>
set -euo pipefail

PREFIX="${1:?usage: build-ios-deps.sh <install-prefix>}"
DEPLOYMENT_TARGET="16.0"
WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT

IOS_CMAKE_ARGS=(
  -DCMAKE_SYSTEM_NAME=iOS
  -DCMAKE_OSX_SYSROOT=iphoneos
  -DCMAKE_OSX_ARCHITECTURES=arm64
  -DCMAKE_OSX_DEPLOYMENT_TARGET="$DEPLOYMENT_TARGET"
  -DCMAKE_BUILD_TYPE=Release
  -DBUILD_SHARED_LIBS=OFF
  -DCMAKE_INSTALL_PREFIX="$PREFIX"
  -DCMAKE_PREFIX_PATH="$PREFIX"
  -DCMAKE_FIND_ROOT_PATH="$PREFIX"
  -DCMAKE_C_FLAGS=-fPIC
  # ogg/vorbis/opusfile ship pre-3.5 cmake_minimum_required, removed in CMake 4.x
  -DCMAKE_POLICY_VERSION_MINIMUM=3.5
)

build() {
  local name="$1" url="$2" ref="$3"; shift 3
  echo "==> $name $ref"
  git init -q "$WORK/$name"
  git -C "$WORK/$name" remote add origin "$url"
  git -C "$WORK/$name" fetch -q --depth 1 origin "$ref"
  git -C "$WORK/$name" checkout -q FETCH_HEAD
  if [ "$name" = "opusfile" ]; then
    # opusfile versions itself via `git describe` (impossible in a shallow tagless fetch)
    # or a package_version file — but only consults the file when .git is absent
    # (OpusFilePackageVersion.cmake uses elseif on the .git dir). Drop .git, provide the file.
    rm -rf "$WORK/$name/.git"
    echo 'PACKAGE_VERSION="0.12.1"' > "$WORK/$name/package_version"
  fi
  cmake -S "$WORK/$name" -B "$WORK/$name/build" -GNinja "${IOS_CMAKE_ARGS[@]}" "$@"
  cmake --build "$WORK/$name/build" -j
  cmake --install "$WORK/$name/build"
}

build ogg      https://github.com/xiph/ogg.git      v1.3.5 -DINSTALL_DOCS=OFF -DBUILD_TESTING=OFF
build vorbis   https://github.com/xiph/vorbis.git   v1.3.7
build opus     https://github.com/xiph/opus.git     v1.5.2 -DOPUS_BUILD_TESTING=OFF -DOPUS_BUILD_PROGRAMS=OFF
# opusfile's last release (0.12) predates its CMake support; pin master (dormant since 2024-09)
build opusfile https://github.com/xiph/opusfile.git 3ecc22aa0a4430f61c2403d57370a089536d197a \
  -DOP_DISABLE_HTTP=ON -DOP_DISABLE_EXAMPLES=ON -DOP_DISABLE_DOCS=ON
# game code (ColorPictograph enhancement) includes png.h directly; zlib comes from the iOS SDK
build libpng   https://github.com/pnggroup/libpng.git v1.6.44 \
  -DPNG_SHARED=OFF -DPNG_STATIC=ON -DPNG_TESTS=OFF -DPNG_TOOLS=OFF -DPNG_FRAMEWORK=OFF

echo "==> installed into $PREFIX:"
find "$PREFIX/lib" -name '*.a' -maxdepth 1
