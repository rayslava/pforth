cmake_minimum_required(VERSION 3.2)
project(pforth)
cmake_policy(SET CMP0049 OLD)

enable_language(C)

set(CMAKE_MODULE_PATH
  ${CMAKE_MODULE_PATH}
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake
  )

set(SOURCE_FILES
  src/interp.c
  )

set(CMAKE_EXPORT_COMPILE_COMMANDS On)

macro(create_test name files)
  message("-- Creating test '${name}' of ${files}")

  set(${name}_TEST_FILES
    ${files}
    )

  set(test_files
    "\${${name}_TEST_FILES}"
    )

  add_executable(
    ${name}_test
    ${SOURCE_FILES}
    ${CHECK_SOURCES}
    ${test_files}
    )

  target_link_libraries(
    ${name}_test
    ${CHECK_LIBRARIES}
    ${AUX_LIBS}
    )

  add_test(${name} ${name}_test)
endmacro(create_test)

include(ConfigSafeGuards)
find_package(PkgConfig REQUIRED)
pkg_check_modules(CHECK REQUIRED check>=0.10.0)

if (DEFINED CHECK_INCLUDE_DIRS)
  include_directories(${CHECK_INCLUDE_DIRS})
endif()

if (TRAVIS_BUILD)
  add_definitions(-DTRAVIS_BUILD)
endif()

set(STANDARD "-std=c11")

set(CMAKE_C_FLAGS         "${CMAKE_C_FLAGS} -Wall -Wno-unknown-pragmas -Wno-sign-compare -Wwrite-strings -Wno-unused ${STANDARD} -Wextra -pedantic -Werror -pthread ")
set(CMAKE_C_FLAGS_PROFILED "${CMAKE_C_FLAGS} -pg")

if (STATIC)
  set(CMAKE_C_FLAGS         "${CMAKE_C_FLAGS} -static -Wl,--whole-archive -lpthread -Wl,--no-whole-archive -fPIC -fPIE")
  if ("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
    set(CMAKE_C_FLAGS         "${CMAKE_C_FLAGS} -static-libgcc")
  endif()
endif()
if (SANITIZED)
  set(CMAKE_C_FLAGS         "${CMAKE_C_FLAGS} -fsanitize=address")
endif()

set(CMAKE_C_FLAGS_DEBUG   "-O0 -ggdb ${STANDARD}")
set(CMAKE_C_FLAGS_RELEASE "-Ofast ${STANDARD}")

if ("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
  set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -flto -fwhole-program")
endif()

if ("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-error=unknown-warning-option -Wno-error=braced-scalar-init")
endif()

if ((COVERALLS OR COVERAGE) AND CMAKE_BUILD_TYPE STREQUAL "Debug")
  message(STATUS "will generate coverage data")
  include(CodeCoverage)
  include(Coveralls)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -ggdb -O0 -Wall -W -fprofile-arcs -ftest-coverage --coverage")
  set(CMAKE_SHARED_LINKER_FLAGS="${CMAKE_SHARED_LINKER_FLAGS} -fprofile-arcs -ftest-coverage -lgcov")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-arcs -ftest-coverage -lgcov")
endif()

if (STATIC)
  set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")
  set (AUX_LIBS "-lrt -lm -lc")
  set (CHECK_LIBRARIES ${CHECK_LIBRARIES_STATIC})
endif()

add_executable(${CMAKE_PROJECT_NAME} src/main.c ${SOURCE_FILES})

enable_testing()

include_directories(
  ${PROJECT_SOURCE_DIR}/src
  )

include(CTest)
enable_testing()

create_test(zero test/zero.c)

if (COVERAGE)
  setup_target_for_coverage(coverage zero_test src coverage)
endif()

if (COVERALLS)
  setup_target_for_coveralls(coveralls unit_tests src)
endif()
