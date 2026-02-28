include(SAMPPluginToolkit)
sampcmake_configure_amx()

option(LOGCORE_INSTALL_DEV 
	"Generate install target specifically for development." ON)

add_subdirectory(amx)

if(WIN32)
	set(CRASHHANDLER_CPP crashhandler_windows.cpp)
	set(FILECHANGEDETECTOR_CPP FileChangeDetectorWindows.cpp)
else()
	set(CRASHHANDLER_CPP crashhandler_unix.cpp)
	set(FILECHANGEDETECTOR_CPP FileChangeDetectorLinux.cpp)
endif()

set(LOGCORE_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include/samplog)

sampcmake_add_shared_library(log-core
	NO_PREFIX
	SOURCES
	Api.cpp
	AmxDebugManager.cpp
	AmxDebugManager.hpp
	SampConfigReader.cpp
	SampConfigReader.hpp
	Singleton.hpp
	LogConfig.cpp
	LogConfig.hpp
	Logger.cpp
	Logger.hpp
	LogManager.cpp
	LogManager.hpp
	LogRotationManager.cpp
	LogRotationManager.hpp
	utils.cpp
	utils.hpp
	${CRASHHANDLER_CPP}
	crashhandler.hpp
	${FILECHANGEDETECTOR_CPP}
	FileChangeDetector.hpp
	${LOGCORE_INCLUDE_DIR}/LogLevel.hpp
	${LOGCORE_INCLUDE_DIR}/ILogger.hpp
	${LOGCORE_INCLUDE_DIR}/export.h
)

target_include_directories(log-core PRIVATE "../include")

target_compile_features(log-core PUBLIC 
	cxx_auto_type 
	cxx_decltype 
	cxx_defaulted_functions 
	cxx_default_function_template_args
	cxx_delegating_constructors
	cxx_deleted_functions
	cxx_enum_forward_declarations
	cxx_explicit_conversions
	cxx_extended_friend_declarations
	cxx_inheriting_constructors
	cxx_noexcept
	cxx_nonstatic_member_init
	cxx_nullptr
	cxx_range_for
	cxx_reference_qualified_functions
	cxx_right_angle_brackets
	cxx_rvalue_references
	cxx_strong_enums
	cxx_variadic_templates
)

set_target_properties(log-core PROPERTIES OUTPUT_NAME "log-core2")
if(MINGW)
	set_target_properties(log-core PROPERTIES SUFFIX ".dll")
endif()

target_include_directories(log-core PRIVATE 
	${LOGCORE_LIBS_DIR}/tinydir
)

target_compile_definitions(log-core PRIVATE IN_LOGCORE_PLUGIN FMT_HEADER_ONLY)
if(MSVC)
	target_compile_definitions(log-core PRIVATE
		_CRT_SECURE_NO_WARNINGS
		NOMINMAX
		WIN32_LEAN_AND_MEAN
		NOGDI # disables ERROR define (conflicts with log level)
	)
endif()

if(UNIX AND NOT APPLE)
	target_link_libraries(log-core PRIVATE rt)
endif()

if (MSVC)
	target_compile_options(log-core PRIVATE /WX)
endif()

sampcmake_apply_target_profile(log-core plugin-windows-cross-static-runtime)

target_link_libraries(log-core PRIVATE
	amx
	CONAN_PKG::fmt
	CONAN_PKG::yaml-cpp
)

sampcmake_configure_plugin_target_output(log-core
	RUNTIME_DIR "${CMAKE_BINARY_DIR}/artifact"
	LIBRARY_DIR "${CMAKE_BINARY_DIR}/artifact"
	ARCHIVE_DIR "${CMAKE_BINARY_DIR}/lib"
)
