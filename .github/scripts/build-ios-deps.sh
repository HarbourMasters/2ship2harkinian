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
)

build() {
  local name="$1" url="$2" tag="$3"; shift 3
  echo "==> $name $tag"
  git -C "$WORK" clone --depth 1 --branch "$tag" "$url" "$name"
  cmake -S "$WORK/$name" -B "$WORK/$name/build" -GNinja "${IOS_CMAKE_ARGS[@]}" "$@"
  cmake --build "$WORK/$name/build" -j
  cmake --install "$WORK/$name/build"
}

build ogg      https://github.com/xiph/ogg.git      v1.3.5 -DINSTALL_DOCS=OFF -DBUILD_TESTING=OFF
build vorbis   https://github.com/xiph/vorbis.git   v1.3.7
build opus     https://github.com/xiph/opus.git     v1.5.2 -DOPUS_BUILD_TESTING=OFF -DOPUS_BUILD_PROGRAMS=OFF
build opusfile https://github.com/xiph/opusfile.git v0.12  -DOP_DISABLE_HTTP=ON -DOP_DISABLE_EXAMPLES=ON -DOP_DISABLE_DOCS=ON

echo "==> installed into $PREFIX:"
find "$PREFIX/lib" -name '*.a' -maxdepth 1
