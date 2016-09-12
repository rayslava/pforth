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
  src/pforth.c
  src/dict.c
  src/words.c
  )

add_custom_target(FORTH_CORE
  COMMAND  perl -pe 's|^\\\\.*\$$||\;s|\\n| |\;s|\\x28.*?\\x29||\;s|\\x20+| |' core.fs > core_compressed.fs && xxd -i core_compressed.fs | awk '{ if (/_len/) {$$5=$$5+1\"\;\"} ;if (/[0-9a-f]$$/) { print $$0 \", 0x00\" } else print ; }' > core_fs.h
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/src
  SOURCES ${CMAKE_SOURCE_DIR}/src/core.fs)

set_directory_properties(PROPERTIES
  ADDITIONAL_MAKE_CLEAN_FILES
  "${CMAKE_SOURCE_DIR}/src/core_fs.h;${CMAKE_SOURCE_DIR}/src/core_compressed.fs"
  )

set(CMAKE_EXPORT_COMPILE_COMMANDS On)

macro(create_test name files)
  message("-- Creating test '${name}' of ${files}")

  set(${name}_TEST_FILES
    ${files}
    )

  add_executable(
    ${name}_test
    ${SOURCE_FILES}
    ${CHECK_SOURCES}
    ${${name}_TEST_FILES}
    )

  add_dependencies(${name}_test FORTH_CORE)

  target_link_libraries(
    ${name}_test
    ${CHECK_LIBRARIES}
    ${AUX_LIBS}
    )

  add_test(${name} ${name}_test)
endmacro(create_test)

include(ConfigSafeGuards)
find_package(PkgConfig REQUIRED)
pkg_check_modules(CHECK REQUIRED check>=0.9.10)

if (DEFINED CHECK_INCLUDE_DIRS)
  include_directories(${CHECK_INCLUDE_DIRS})
endif()

if (TRAVIS_BUILD)
  add_definitions(-DTRAVIS_BUILD)
endif()

set(STANDARD "-std=c11")

set(CMAKE_C_FLAGS         "${CMAKE_C_FLAGS} -Wall -Wno-unknown-pragmas -Wno-sign-compare -Wwrite-strings -Wno-unused ${STANDARD} -Wextra -pedantic -Werror")
set(CMAKE_C_FLAGS_PROFILED "${CMAKE_C_FLAGS} -pg")
set(AUX_LIBS "-lpthread")

if (STATIC)
  set(CMAKE_C_FLAGS         "${CMAKE_C_FLAGS} -static -Wl,--whole-archive -lpthread -Wl,--no-whole-archive -fPIC -fPIE")
  if ("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
    set(CMAKE_C_FLAGS         "${CMAKE_C_FLAGS} -static-libgcc")
  endif()
endif()
if (SANITIZED)
  set(CMAKE_C_FLAGS         "${CMAKE_C_FLAGS} -fsanitize=address")
endif()

set(CMAKE_C_FLAGS_DEBUG   "-O0 -ggdb -DDEBUG ${STANDARD}")
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
add_dependencies(${CMAKE_PROJECT_NAME} FORTH_CORE)
enable_testing()

include_directories(
  ${PROJECT_SOURCE_DIR}/src
  )

include(CTest)
enable_testing()

create_test(stack test/stack.c)
create_test(math test/math.c)
create_test(interp test/interp.c)
create_test(words test/words.c)

if (COVERAGE)
  setup_target_for_coverage(math_coverage math_test src coverage)
  setup_target_for_coverage(stack_coverage stack_test src coverage)
  setup_target_for_coverage(interp_coverage interp_test src coverage)
  setup_target_for_coverage(words_coverage words_test src coverage)
  ADD_CUSTOM_TARGET(coverage
    DEPENDS math_coverage stack_coverage interp_coverage words_coverage FORTH_CORE)
endif()

if (COVERALLS)
    setup_target_for_coveralls(math_coveralls math_test src coveralls)
    setup_target_for_coveralls(stack_coveralls stack_test src coveralls)
    setup_target_for_coveralls(interp_coveralls interp_test src coveralls)
    setup_target_for_coverage(words_coverage words_test src coveralls)
    ADD_CUSTOM_TARGET(coveralls
      DEPENDS math_coveralls stack_coveralls interp_coveralls words_coverage FORTH_CORE)
endif()
