cmake_minimum_required(VERSION 3.0)
project(SwiftySyncUniversalClient)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_C_STANDARD 11)

IF(APPLE)
    set(CMAKE_THREAD_LIBS_INIT "-lpthread")
    set(CMAKE_HAVE_THREADS_LIBRARY 1)
    set(CMAKE_USE_WIN32_THREADS_INIT 0)
    set(CMAKE_USE_PTHREADS_INIT 1)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
ENDIF()

#set(SYSTEM_NAME "iOS")
if("${SYSTEM_NAME}" STREQUAL "iOS")
    set(IOS 1)

    include(CheckCXXSymbolExists)
    check_cxx_symbol_exists(kqueue sys/event.h HAVE_KQUEUE)
    if(NOT HAVE_KQUEUE)
        message(STATUS "kqueue NOT found!")
    else()
        message(STATUS "kqueue found!")
    endif()

    find_library(APPKIT_LIBRARY AppKit)
    if (NOT APPKIT_LIBRARY)
        message(STATUS "AppKit.framework NOT found!")
    else()
        message(STATUS "AppKit.framework found! ${APPKIT_LIBRARY}")
    endif()

    find_library(UIKIT_LIBRARY UIKit)
    if (NOT UIKIT_LIBRARY)
        message(STATUS "UIKit.framework NOT found!")
    else()
        message(STATUS "UIKit.framework found! ${UIKIT_LIBRARY}")
    endif()

    include(CheckTypeSize)
    check_type_size(time_t SIZEOF_TIME_T)
endif()

add_compile_definitions(CLIENT)
add_compile_definitions(_WEBSOCKETPP_CPP11_THREAD_)
add_compile_definitions(_WEBSOCKETPP_CPP11_RANDOM_DEVICE_)
add_compile_definitions(ASIO_STANDALONE)

add_subdirectory(SwiftySyncAuthorization)
add_subdirectory(SwiftySyncCommon)
add_subdirectory(SwiftySyncStorage)

include_directories(include)
include_directories(SwiftySyncAuthorization/include)
include_directories(SwiftySyncCommon/include)
include_directories(SwiftySyncStorage/include)

include_directories(/)
include_directories(SwiftySyncAuthorization/cpp-httplib)
include_directories(websocketpp)
include_directories(SwiftySyncCommon/CodablePP/include)
include_directories(asio/asio/include)

set(OPENSSL_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/openssl-cmake/include)
set(OPENSSL_LIBRARIES ssl crypto)

include_directories(${OPENSSL_INCLUDE_DIR})

add_library(SwiftySyncUniversalClient src/SwiftySyncUniversalClient.cpp include/SwiftySyncUniversalClient.hpp)
target_link_libraries(SwiftySyncUniversalClient SwiftySyncAuthorization SwiftySyncCommon SwiftySyncStorage)

add_library(c_api_connector src/c_api_connector.cpp include/c_api_connector.h)
target_link_libraries(c_api_connector SwiftySyncUniversalClient c_api_storage)

if("${IOS}" EQUAL "1")
    install(TARGETS c_api_connector
            LIBRARY DESTINATION lib
            ARCHIVE DESTINATION lib/static)

    install (FILES c_api_connector.h SwiftySyncStorage/c_api_storage.h DESTINATION include)
endif()

add_executable(test_client test/test.cpp)
target_link_libraries(test_client LINK_PUBLIC SwiftySyncUniversalClient)

add_executable(test_c_api test/test_c_api.c)
target_link_libraries(test_c_api LINK_PUBLIC c_api_connector)