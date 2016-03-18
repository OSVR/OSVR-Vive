#
# HTC Vive plugin for OSVR
#
cmake_minimum_required(VERSION 3.1.0)
project(com_osvr_Vive)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(CopyImportedTarget)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(WIN32 OR ANDROID)
    option(Boost_USE_STATIC_LIBS "Build with Boost's static libraries?" ON)
endif()
find_package(osvr REQUIRED)
find_package(JsonCpp REQUIRED)
find_package(Boost REQUIRED COMPONENTS system iostreams filesystem)
find_package(Eigen3 REQUIRED)

# Interface target for the openvr_driver.h header we'll use to interact with the target driver.
add_library(OpenVRDriver INTERFACE)
target_include_directories(OpenVRDriver INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/vendor/openvr/headers")

add_library(boost_process INTERFACE)
target_include_directories(boost_process INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/vendor/boost-process" ${Boost_INCLUDE_DIRS})
target_link_libraries(boost_process INTERFACE ${Boost_SYSTEM_LIBRARIES} ${Boost_IOSTREAMS_LIBRARIES} ${Boost_FILESYSTEM_LIBRARIES})

add_library(linkable_into_dll INTERFACE)
target_compile_options(linkable_into_dll INTERFACE ${CMAKE_SHARED_LIBRARY_CXX_FLAGS})

# Interface target for some sort of filesystem functionality
if(MSVC AND NOT MSVC_VERSION LESS 1800)
    # We can use the experimental filesystem header here.
    add_library(filesystem_lib INTERFACE)
    target_compile_definitions(filesystem_lib INTERFACE OSVR_USING_FILESYSTEM_TR2)
else()
    # Interface target for boost fs
    find_library(Boost REQUIRED COMPONENTS filesystem system)
    add_library(filesystem_lib INTERFACE)
    target_link_libraries(filesystem_lib
        INTERFACE
        ${Boost_FILESYSTEM_LIBRARIES}
        ${Boost_SYSTEM_LIBRARIES})
    target_include_directories(filesystem_lib INTERFACE ${Boost_INCLUDE_DIRS})
    target_compile_definitions(filesystem_lib INTERFACE BOOST_FILESYSTEM_VERSION=3 OSVR_USING_BOOST_FILESYSTEM)
endif()

set(DISPLAY_SOURCES
    DisplayDescriptor.cpp
    DisplayDescriptor.h
    MonoPoints.cpp
    MonoPoints.h
    PointHelpers.h
    RGBPoints.cpp
    RGBPoints.h)

# Put the shared files into a static library, so we don't recompile them multiple times.
add_library(ViveLoaderLib STATIC
    ChaperoneData.cpp
    ChaperoneData.h
    DeviceHolder.h
    DriverLoader.cpp
    DriverLoader.h
    DriverWrapper.h
    FindDriver.cpp
    FindDriver.h
    GetComponent.h
    GetProvider.h
    InterfaceTraits.h
    SearchPathExtender.h
    ServerDriverHost.cpp
    ServerDriverHost.h
    PropertyHelper.h
    VRSettings.cpp
    VRSettings.h)
target_link_libraries(ViveLoaderLib
    PUBLIC
    OpenVRDriver osvr::osvrUtil linkable_into_dll
    PRIVATE
    filesystem_lib boost_process JsonCpp::JsonCpp)
target_include_directories(ViveLoaderLib PRIVATE ${Boost_INCLUDE_DIRS})

# Build the plugin
osvr_convert_json(com_osvr_Vive_json
    com_osvr_Vive.json
    "${CMAKE_CURRENT_BINARY_DIR}/com_osvr_Vive_json.h")
osvr_convert_json(org_osvr_ViveController_json
    org_osvr_ViveController.json
    "${CMAKE_CURRENT_BINARY_DIR}/org_osvr_ViveController_json.h")
osvr_add_plugin(com_osvr_Vive
    CPP
    com_osvr_Vive.cpp
    OSVRViveTracker.cpp
    OSVRViveTracker.h
    QuickProcessingDeque.h
    VerifyLocked.h
    "${CMAKE_CURRENT_BINARY_DIR}/com_osvr_Vive_json.h"
    "${CMAKE_CURRENT_BINARY_DIR}/org_osvr_ViveController_json.h")

target_link_libraries(com_osvr_Vive ViveLoaderLib JsonCpp::JsonCpp)
target_include_directories(com_osvr_Vive
    PRIVATE
    ${EIGEN3_INCLUDE_DIR})

# Build the executable
add_executable(ViveLoader
    ViveLoader.cpp)

target_link_libraries(ViveLoader PRIVATE ViveLoaderLib)
copy_imported_targets(ViveLoader osvr::osvrUtil)

# Build another tool

osvr_convert_json(viveDisplayInput
    input/HTC_Vive_PRE.json
    "${CMAKE_CURRENT_BINARY_DIR}/viveDisplayInput.h")
add_executable(DisplayExtractor
    DisplayExtractor.cpp
    ${DISPLAY_SOURCES}
    "${CMAKE_CURRENT_BINARY_DIR}/viveDisplayInput.h")
target_link_libraries(DisplayExtractor PRIVATE ViveLoaderLib JsonCpp::JsonCpp)
copy_imported_targets(DisplayExtractor osvr::osvrUtil)

# Install files for the plugin.
install(FILES
    osvr_server_config.vive.sample.json
    README.md
    LICENSE
    DESTINATION .)
