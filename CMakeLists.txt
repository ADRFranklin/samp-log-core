cmake_minimum_required(VERSION 3.15)
cmake_policy(SET CMP0091 NEW)
project(samp-log-core VERSION 0.4)

if (NOT LOGCORE_VERSION) # allow version override
	set(LOGCORE_VERSION ${PROJECT_VERSION})
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/libs/sampcmake")
include(SAMPPluginToolkit)
sampcmake_apply_global_profile(samp-default)

set(CMAKE_CXX_STANDARD 11) #required for fmt library

if (MSVC)
	set(CMAKE_BUILD_TYPE "" CACHE STRING "" FORCE)
endif()

set(LOGCORE_LIBS_DIR "${PROJECT_SOURCE_DIR}/libs")

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/cmake/conan.cmake" AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/libs/sampcmake/patches/cmake-conan-manual-compiler.patch")
	sampcmake_apply_cmake_conan_manual_compiler_patch(
		WORK_DIR "${CMAKE_CURRENT_SOURCE_DIR}"
		CMAKE_CONAN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/cmake/conan.cmake"
		PATCH_FILE "${CMAKE_CURRENT_SOURCE_DIR}/libs/sampcmake/patches/cmake-conan-manual-compiler.patch"
	)
endif()
include(cmake/conan.cmake)

set(LOGCORE_CONAN_PROFILE_ARGS "")
if(DEFINED DCC_CONAN_PROFILE)
	list(APPEND LOGCORE_CONAN_PROFILE_ARGS PROFILE ${DCC_CONAN_PROFILE})
endif()
if(DEFINED DCC_CONAN_PROFILE_HOST)
	list(APPEND LOGCORE_CONAN_PROFILE_ARGS PROFILE ${DCC_CONAN_PROFILE_HOST})
endif()
if(DEFINED DCC_CONAN_PROFILE_BUILD)
	list(APPEND LOGCORE_CONAN_PROFILE_ARGS PROFILE_BUILD ${DCC_CONAN_PROFILE_BUILD})
endif()

set(LOGCORE_CONAN_SETTINGS_ARGS "")
if(DEFINED DCC_CONAN_SETTINGS)
	list(APPEND LOGCORE_CONAN_SETTINGS_ARGS ${DCC_CONAN_SETTINGS})
endif()

function(add_conan_lib pkg_name conan_pkg)
	conan_check(REQUIRED)

	conan_cmake_run(
		REQUIRES ${conan_pkg}
		ARCH x86
		BASIC_SETUP CMAKE_TARGETS NO_OUTPUT_DIRS
		BUILD missing
		${LOGCORE_CONAN_PROFILE_ARGS}
		SETTINGS ${LOGCORE_CONAN_SETTINGS_ARGS}
	)

	set_target_properties(${pkg_name} PROPERTIES IMPORTED_GLOBAL TRUE)
endfunction()

if(NOT DEFINED CONAN_HAS_FMT)
	add_conan_lib(CONAN_PKG::fmt fmt/7.1.3)
endif()

add_conan_lib(CONAN_PKG::yaml-cpp yaml-cpp/0.7.0)

set(LOGCORE_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include" CACHE INTERNAL "")
add_subdirectory(src)
