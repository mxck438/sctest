cmake_minimum_required(VERSION 3.15.0)
project(sctest VERSION 1.0.1 LANGUAGES C )

#set(CMAKE_VERBOSE_MAKEFILE ON)
set(USE_OPT2 OFF)
set(USE_GPROF OFF) 
set(USE_ASAN OFF)
set(USE_TSAN OFF)
if (USE_TSAN)
    set(USE_ASAN OFF)
endif()

include_directories(include)
include_directories(test)

include(sct_tests.cmake)

add_executable(sctest    
  src/sctest_main.c
  src/sct_core.c
  src/sct_commands.c
  src/sct_example_plugin.c
  src/sct_utils.c 
)
               
#-------------------------------------------------------------------------------
#        project version management
find_package(Git QUIET REQUIRED)
execute_process(
        COMMAND "${GIT_EXECUTABLE}" describe --tags
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        RESULT_VARIABLE res
        OUTPUT_VARIABLE out
        ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
    if(res EQUAL 0)
        set(GIT_DESCRIBE ${out})
    else()
        set(GIT_DESCRIBE "1.0-git_describe_failed")
    endif()

message("${GIT_DESCRIBE}") 
string(REGEX REPLACE "^([0-9]+)\.([0-9]+)\.([0-9]+)\-([0-9]+)\-(.*)$"
"\\1;\\2;\\3;\\4;\\5" _ver_parts "${GIT_DESCRIBE}")
list(GET _ver_parts 0 TAG_VERSION_MAJOR)
list(GET _ver_parts 1 TAG_VERSION_MINOR)   
list(GET _ver_parts 2 TAG_VERSION_PATCH)
list(GET _ver_parts 3 TAGS_SINCE) 
list(GET _ver_parts 4 COMMIT_HASH)

MATH(EXPR TAG_CURR_PATCH "${TAG_VERSION_PATCH}+${TAGS_SINCE}")
set(GIT_DESCRIBE ${TAG_VERSION_MAJOR}.${TAG_VERSION_MINOR}.${TAG_CURR_PATCH}-${COMMIT_HASH})

execute_process(
        COMMAND "${GIT_EXECUTABLE}" diff-index --quiet HEAD --
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        RESULT_VARIABLE res
        OUTPUT_VARIABLE out
        ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
    if(res EQUAL 0)
        set(COMMIT_STATUS "-CLEAN")
    else()
        set(COMMIT_STATUS "-DIRTY")
    endif()
set(SCTEST_VERSION ${GIT_DESCRIBE}${COMMIT_STATUS})


target_include_directories(sctest PUBLIC "${PROJECT_BINARY_DIR}")
# SCTEST_BUILD_CONFIG is defined in CMakePresets.json
configure_file(src/sctest_build_config.h.in sctest_build_config.h) 

#-------------------------------------------------------------------------------

if (CMAKE_C_COMPILER_ID MATCHES "Clang")
    message("CLANG")
    target_compile_options(sctest PRIVATE -Wno-ignored-qualifiers)
    target_compile_options(sctest PRIVATE -Wno-incompatible-pointer-types-discards-qualifiers)
    target_compile_options(sctest PRIVATE -Wno-pointer-sign)
else()
    message("GCC")
    target_compile_options(sctest PRIVATE -Wno-discarded-qualifiers)
endif()
if(USE_OPT2)
    target_compile_options(sctest PRIVATE -O3 -DNDEBUG)
endif()      
if(USE_GPROF)
    target_compile_options(sctest PRIVATE -pg)
    target_link_options(sctest PRIVATE -pg) 
endif()    
if(USE_ASAN)
    target_compile_options(sctest PRIVATE -fsanitize=address)
    target_link_options(sctest PRIVATE -fsanitize=address -static-libasan)  #-static-libasan
endif()
if(USE_TSAN)
    target_compile_options(sctest PRIVATE -fsanitize=thread)
    target_link_options(sctest PRIVATE -fsanitize=thread)
endif()

if (USE_TSAN)
    target_link_libraries(sctest tsan)
endif()

target_link_libraries(sctest rt m ${CMAKE_DL_LIBS} readline)

configure_file(grep_test_file grep_test_file) 

#----------------------------------------------------------
# INSTALL
#----------------------------------------------------------

# not implemented

