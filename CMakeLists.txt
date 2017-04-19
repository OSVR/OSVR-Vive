#
# HTC Vive plugin for OSVR
#
cmake_minimum_required(VERSION 3.1.0)
project(com_osvr_Vive)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(CopyImportedTarget)
include(GNUInstallDirs)

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

#
# Third-party libraries
#
add_subdirectory(vendor)

option(BUILD_EXTRA_TOOLS "Whether the extra, optional tools should also be built." OFF)

# Interface target for the openvr_driver.h header we'll use to interact with the target driver.
add_library(OpenVRDriver INTERFACE)
target_include_directories(OpenVRDriver INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/vendor/openvr/headers")

add_library(boost_filesystem_v3 INTERFACE)
target_link_libraries(boost_filesystem_v3
    INTERFACE
    ${Boost_FILESYSTEM_LIBRARY}
    ${Boost_SYSTEM_LIBRARY})
target_compile_definitions(boost_filesystem_v3 INTERFACE BOOST_FILESYSTEM_VERSION=3)
target_include_directories(boost_filesystem_v3 INTERFACE ${Boost_INCLUDE_DIRS})

add_library(linkable_into_dll INTERFACE)
target_compile_options(linkable_into_dll INTERFACE ${CMAKE_SHARED_LIBRARY_CXX_FLAGS})

# Interface target for some sort of filesystem functionality
if(MSVC AND NOT MSVC_VERSION LESS 1800)
    # We can use the experimental filesystem header here.
    add_library(filesystem_lib INTERFACE)
    target_compile_definitions(filesystem_lib INTERFACE OSVR_USING_FILESYSTEM_HEADER)
    if(MSVC12)
        target_compile_definitions(filesystem_lib INTERFACE OSVR_USING_FILESYSTEM_TR2)
    elseif(MSVC14)
        target_compile_definitions(filesystem_lib INTERFACE OSVR_USING_FILESYSTEM_EXPERIMENTAL)
    endif()
else()
    # Interface target for boost fs
    add_library(filesystem_lib INTERFACE)
    target_link_libraries(filesystem_lib
        INTERFACE
        boost_filesystem_v3)
    target_compile_definitions(filesystem_lib INTERFACE OSVR_USING_BOOST_FILESYSTEM)
endif()

# Figure out where config files, like display descriptors, should go.
if(OSVR_CACHED_CONFIG_ROOT)
    # Using a new enough core that it indicates this for us, hooray!
    set(OSVR_CONFIG_ROOT "${OSVR_CACHED_CONFIG_ROOT}")
else()
    # TODO remove this once enough time has passed after merging
    # https://github.com/OSVR/OSVR-Core/pull/402
    if(WIN32)
        set(OSVR_CONFIG_ROOT "${CMAKE_INSTALL_BINDIR}")
    else()
        set(OSVR_CONFIG_ROOT "${CMAKE_INSTALL_DATAROOTDIR}/osvrcore")
    endif()
endif()
set(OSVRVIVE_BIN_DIR ${CMAKE_INSTALL_BINDIR})
# Generate configured header with this information.
configure_file(PathConfig.h.in "${CMAKE_CURRENT_BINARY_DIR}/PathConfig.h")

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
    ReturnValue.h
    SearchPathExtender.h
    ServerDriverHost.cpp
    ServerDriverHost.h
    ServerPropertyHelper.h
    PropertyHelper.h
    PropertyTraits.h
	Properties.h
	Properties.cpp
    Settings.cpp
    Settings.h
    DriverLog.h
    DriverLog.cpp
	DriverContext.h
	DriverContext.cpp
	ValveStrCpy.h)
target_link_libraries(ViveLoaderLib
    PUBLIC
    OpenVRDriver osvr::osvrUtil linkable_into_dll
    PRIVATE
    filesystem_lib JsonCpp::JsonCpp ${CMAKE_DL_LIBS}) # ${CMAKE_DL_LIBS} is set to empty string, when system doesn't provide dlfcn. For example, Windows.
target_include_directories(ViveLoaderLib PUBLIC ${CMAKE_CURRENT_BINARY_DIRECTORY} PRIVATE ${Boost_INCLUDE_DIRS})

# Build the plugin
osvr_convert_json(com_osvr_Vive_json
    com_osvr_Vive.json
    "${CMAKE_CURRENT_BINARY_DIR}/com_osvr_Vive_json.h")
osvr_convert_json(com_osvr_ViveSync_json
    com_osvr_ViveSync.json
    "${CMAKE_CURRENT_BINARY_DIR}/com_osvr_ViveSync_json.h")
osvr_add_plugin(com_osvr_Vive
    CPP
    com_osvr_Vive.cpp
    OSVRViveTracker.cpp
    OSVRViveTracker.h
    QuickProcessingDeque.h
    VerifyLocked.h
    "${CMAKE_CURRENT_BINARY_DIR}/com_osvr_Vive_json.h"
	"${CMAKE_CURRENT_BINARY_DIR}/com_osvr_ViveSync_json.h")

target_link_libraries(com_osvr_Vive ViveLoaderLib)
target_include_directories(com_osvr_Vive
    PRIVATE
    ${EIGEN3_INCLUDE_DIR})

if(BUILD_EXTRA_TOOLS)
    # Build the executable
    add_executable(ViveLoader
        ViveLoader.cpp)
    target_link_libraries(ViveLoader PRIVATE ViveLoaderLib)
    copy_imported_targets(ViveLoader osvr::osvrUtil)
	target_include_directories(ViveLoader PRIVATE ${Boost_INCLUDE_DIRS})

    add_executable(GenerateTypedPropertyEnums
        GenerateTypedPropertyEnums.cpp)
    target_link_libraries(GenerateTypedPropertyEnums PRIVATE JsonCpp::JsonCpp osvr::osvrUtil)
    target_include_directories(GenerateTypedPropertyEnums PRIVATE ${Boost_INCLUDE_DIRS})
endif()

# Build another tool
osvr_convert_json(viveDisplayInput
    input/HTC_Vive_PRE.json
    "${CMAKE_CURRENT_BINARY_DIR}/viveDisplayInput.h")
add_executable(ViveDisplayExtractor
    DisplayExtractor.cpp
    ComputeOutputFiles.cpp
    ComputeOutputFiles.h
    ${DISPLAY_SOURCES}
    "${CMAKE_CURRENT_BINARY_DIR}/viveDisplayInput.h")
target_link_libraries(ViveDisplayExtractor PRIVATE ViveLoaderLib JsonCpp::JsonCpp boost_filesystem_v3)
copy_imported_targets(ViveDisplayExtractor osvr::osvrUtil)
install(TARGETS ViveDisplayExtractor
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# Install files for the plugin.
install(FILES
    osvr_server_config.vive.sample.json
    README.md
    LICENSE
    DESTINATION .)
