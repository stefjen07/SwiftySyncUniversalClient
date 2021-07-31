cmake -B build -G "Xcode" -DSCAPIX_BRIDGE=objc -DCMAKE_TOOLCHAIN_FILE=${PWD}/vcpkg/scripts/buildsystems/vcpkg.cmake
read -p "Bridge generated"