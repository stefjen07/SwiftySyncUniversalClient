cmake_minimum_required(VERSION 3.19)
project(SwiftySyncUniversalClient)
project(c_api_connector)

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

add_library(c_api_connector c_api_connector.cpp c_api_connector.h)
target_link_libraries(c_api_connector SwiftySyncUniversalClient)

add_executable(test_client main.cpp)
target_link_libraries(test_client LINK_PUBLIC SwiftySyncUniversalClient ${Boost_LIBRARIES})

set(CMAKE_C_STANDARD 11)

add_executable(test_c_api c_api_test.c)
target_link_libraries(test_c_api LINK_PUBLIC c_api_connector)