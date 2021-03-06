# iOS support in this file is based off of the Platform/Darwin.cmake and Platform/UnixPaths.cmake
# files which are included with CMake 2.8.4

cmake_minimum_required(VERSION 3.3)
include(../common/CMakeLists.txt)

IF(NOT DEFINED TARGET_BUILD_PLATFORM) # Not defined, default to ios
	SET(TARGET_BUILD_PLATFORM "ios")
ENDIF()

SET(PLATFORM_LIST ios)

IF (NOT ${TARGET_BUILD_PLATFORM} IN_LIST PLATFORM_LIST)
	MESSAGE(FATAL_ERROR "Invalid platform:" ${TARGET_BUILD_PLATFORM})
ENDIF()

SET(CMAKE_CXX_FLAGS "-std=c++11 -fno-rtti -fno-exceptions -ffast-math -ffunction-sections -fdata-sections -Werror -ferror-limit=0 -Wall -Wextra -fstrict-aliasing -Wstrict-aliasing=2 -Weverything -Wno-documentation-deprecated-sync -Wno-documentation-unknown-command -Wno-float-equal -Wno-padded -Wno-weak-vtables -Wno-cast-align -Wno-conversion -Wno-missing-noreturn -Wno-missing-variable-declarations -Wno-shift-sign-overflow -Wno-covered-switch-default -Wno-exit-time-destructors -Wno-global-constructors -Wno-missing-prototypes -Wno-unreachable-code -Wno-unused-macros -Wno-unused-member-function -Wno-used-but-marked-unused -Wno-weak-template-vtables -Wno-deprecated -Wno-non-virtual-dtor -Wno-invalid-noreturn -Wno-return-type-c-linkage -Wno-reserved-id-macro -Wno-c++98-compat-pedantic -Wno-unused-local-typedef -Wno-old-style-cast -Wno-newline-eof -Wno-unused-private-field -Wno-undefined-reinterpret-cast -Wno-invalid-offsetof -Wno-zero-as-null-pointer-constant -Wno-shadow-field -Wno-unused-template -gdwarf-2")

SET(CMAKE_SHARED_LINKER_FLAGS "")

# Build debug info for all configurations
SET(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
SET(CMAKE_CXX_FLAGS_CHECKED "-O3 -g")
SET(CMAKE_CXX_FLAGS_PROFILE "-O3 -g")
SET(CMAKE_CXX_FLAGS_RELEASE "-O3 -g")

SET(CMAKE_OSX_DEPLOYMENT_TARGET "")

SET(CMAKE_OSX_ARCHITECTURES "armv7 armv7s arm64")

SET(IOS_PLATFORM_LOCATION "iPhoneOS.platform")

# Setup iOS developer location unless specified manually with CMAKE_IOS_DEVELOPER_ROOT
EXEC_PROGRAM(/usr/bin/xcode-select ARGS -print-path OUTPUT_VARIABLE CMAKE_XCODE_DEVELOPER_DIR)
SET(XCODE_ROOT "${CMAKE_XCODE_DEVELOPER_DIR}/Platforms/${IOS_PLATFORM_LOCATION}/Developer")
IF(NOT DEFINED CMAKE_IOS_DEVELOPER_ROOT)
	IF(EXISTS ${XCODE_ROOT})
		SET(CMAKE_IOS_DEVELOPER_ROOT ${XCODE_ROOT})
	ENDIF(EXISTS ${XCODE_ROOT})
ENDIF(NOT DEFINED CMAKE_IOS_DEVELOPER_ROOT)
SET(CMAKE_IOS_DEVELOPER_ROOT ${CMAKE_IOS_DEVELOPER_ROOT} CACHE PATH "Location of iOS Platform")

# Find and use the most recent iOS sdk unless specified manually with CMAKE_IOS_SDK_ROOT
IF(NOT DEFINED CMAKE_IOS_SDK_ROOT)
	FILE(GLOB _CMAKE_IOS_SDKS "${CMAKE_IOS_DEVELOPER_ROOT}/SDKs/*")
	IF(_CMAKE_IOS_SDKS) 
		LIST(SORT _CMAKE_IOS_SDKS)
		LIST(REVERSE _CMAKE_IOS_SDKS)
		LIST(GET _CMAKE_IOS_SDKS 0 CMAKE_IOS_SDK_ROOT)
	ELSE(_CMAKE_IOS_SDKS)
		MESSAGE(FATAL_ERROR "No iOS SDK's found in default search path ${CMAKE_IOS_DEVELOPER_ROOT}. Manually set CMAKE_IOS_SDK_ROOT or install the iOS SDK.")
	ENDIF(_CMAKE_IOS_SDKS)
	MESSAGE(STATUS "Toolchain using default iOS SDK: ${CMAKE_IOS_SDK_ROOT}")
ENDIF(NOT DEFINED CMAKE_IOS_SDK_ROOT)
SET(CMAKE_IOS_SDK_ROOT ${CMAKE_IOS_SDK_ROOT} CACHE PATH "Location of the selected iOS SDK")

# Set the sysroot default to the most recent SDK
SET(CMAKE_OSX_SYSROOT ${CMAKE_IOS_SDK_ROOT} CACHE PATH "Sysroot used for iOS support")

SET(CMAKE_XCODE_ATTRIBUTE_SUPPORTED_PLATFORMS "iphoneos")
SET(CMAKE_XCODE_ATTRIBUTE_SDKROOT ${CMAKE_IOS_SDK_ROOT})
SET(CMAKE_XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET "7.0")

# Controls PX_NVTX for all projects on iOS
SET(PHYSX_IOS_ENABLE_NVTX 0)

# Disable cuda and dx for all projects on iOS
SET(PHYSX_IOS_COMPILE_DEFS DISABLE_COMPUTE_PHYSX)
SET(PHYSX_IOS_DEBUG_COMPILE_DEFS _DEBUG;PX_DEBUG=1;PX_CHECKED=1;PX_NVTX=${PHYSX_IOS_ENABLE_NVTX};PX_SUPPORT_PVD=1)
SET(PHYSX_IOS_CHECKED_COMPILE_DEFS NDEBUG;PX_CHECKED=1;PX_NVTX=${PHYSX_IOS_ENABLE_NVTX};PX_SUPPORT_PVD=1)
SET(PHYSX_IOS_PROFILE_COMPILE_DEFS NDEBUG;PX_PROFILE=1;PX_NVTX=${PHYSX_IOS_ENABLE_NVTX};PX_SUPPORT_PVD=1)
SET(PHYSX_IOS_RELEASE_COMPILE_DEFS NDEBUG;PX_SUPPORT_PVD=0)

# Include the PxShared-exports.cmake file that PxShared created. There's other ways to do this, but there's a bunch of issues with it in a CI environment
#  such as - we don't want to be installing any built artifacts on this server, etc. So for now we hack it.

# Add PxShared as a dependency so that we can use project references
ADD_SUBDIRECTORY(${PXSHARED_ROOT_DIR}/src/compiler/cmake/ios "${CMAKE_CURRENT_BINARY_DIR}/pxshared_bin")

# Include all of the projects
INCLUDE(LowLevel.cmake)
INCLUDE(LowLevelAABB.cmake)
INCLUDE(LowLevelCloth.cmake)
INCLUDE(LowLevelDynamics.cmake)
INCLUDE(LowLevelParticles.cmake)
INCLUDE(PhysX.cmake)
INCLUDE(PhysXCharacterKinematic.cmake)
INCLUDE(PhysXCommon.cmake)
INCLUDE(PhysXCooking.cmake)
INCLUDE(PhysXExtensions.cmake)
INCLUDE(PhysXVehicle.cmake)
INCLUDE(SceneQuery.cmake)
INCLUDE(SimulationController.cmake)

