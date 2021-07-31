cmake_minimum_required(VERSION 3.19)
project(SwiftySyncUniversalClient)

set(CMAKE_CXX_STANDARD 14)

add_compile_definitions(CLIENT)

add_subdirectory(SwiftySyncAuthorization)
add_subdirectory(SwiftySyncCommon)
add_subdirectory(SwiftySyncStorage)

include_directories(SwiftySyncAuthorization)
include_directories(SwiftySyncCommon)
include_directories(SwiftySyncStorage)

include_directories(/)
include_directories(SwiftySyncAuthorization/cpp-httplib)
include_directories(websocketpp)
include_directories(SwiftySyncCommon/CodablePP)

find_package(Boost REQUIRED random)
include_directories(${Boost_INCLUDE_DIR})

add_library(SwiftySyncUniversalClient SwiftySyncUniversalClient.cpp SwiftySyncUniversalClient.h)
target_link_libraries(SwiftySyncUniversalClient SwiftySyncAuthorization SwiftySyncCommon SwiftySyncStorage ${Boost_LIBRARIES})

include(FetchContent)
FetchContent_Declare(
        cmodule
        URL "https://github.com/scapix-com/cmodule/archive/v1.0.29.tar.gz"
        URL_HASH SHA256=b49019b355423aebacd927e99541b146c900ef416ae1da6a8343a2a274dd4876
)
FetchContent_MakeAvailable(cmodule)

find_package(Scapix REQUIRED)

if (DEFINED SCAPIX_BRIDGE)
project(universal_scapix)

set(UNIVERSAL_CLIENT_HEADER SwiftySyncUniversalClient.h)
scapix_add_target(universal_scapix "com.stefjen07.swiftysyncuniversalclient" "${UNIVERSAL_CLIENT_HEADER}")
target_sources(universal_scapix PRIVATE SwiftySyncUniversalClient.cpp)
target_include_directories(universal_scapix PUBLIC /)
target_link_libraries(universal_scapix PRIVATE SwiftySyncAuthorization SwiftySyncCommon SwiftySyncStorage ${Boost_LIBRARIES})
endif()

add_executable(test_client main.cpp)
target_link_libraries(test_client LINK_PUBLIC SwiftySyncUniversalClient ${Boost_LIBRARIES})