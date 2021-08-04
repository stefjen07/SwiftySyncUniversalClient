export SYSTEM_NAME="iOS"
export INCLUDE_DIR="$(PWD)/vcpkg_installed/arm64-ios/include"
export LIB_DIR="$(PWD)/vcpkg_installed/arm64-ios/lib"
export LIBS="crypto ssl"
SRC_DIR=$(PWD)
mkdir ios_build
cd ios_build
cmake .. -G Xcode -DCMAKE_TOOLCHAIN_FILE=$SRC_DIR/ios.toolchain/ios.toolchain.cmake -DPLATFORM=OS64
cmake --build . --config Release