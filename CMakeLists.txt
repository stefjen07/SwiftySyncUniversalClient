cmake_minimum_required(VERSION 3.19)
project(SwiftySyncUniversalClient)

set(CMAKE_CXX_STANDARD 14)

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

add_executable(test_client main.cpp)
target_link_libraries(test_client LINK_PUBLIC SwiftySyncUniversalClient ${Boost_LIBRARIES})