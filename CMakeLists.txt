# This file is part of tindicators, licensed under GNU LGPL v3.
# Author: Ilya Pikulin <ilya.pikulin@gmail.com>, 2019-2020
# Author: Anatoly <elengar17@gmail.com>, 2019


cmake_minimum_required(VERSION 3.12.4)
find_package(Python3 COMPONENTS Interpreter)
execute_process(
    COMMAND ${Python3_EXECUTABLE} -c "import version; print(version.version, end='')"
    OUTPUT_VARIABLE version
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

project(tindicators VERSION "${version}" LANGUAGES C CXX)

## Library
file(GLOB src indicators/*)
file(GLOB utils utils/*)
add_library(indicators SHARED ${src} ${utils} indicators.h indicators_index.c)
# target_link_libraries(indicators PUBLIC
    # "$<$<OR:$<C_COMPILER_ID:GNU>>:$<$<CONFIG:DEBUG>:asan;ubsan>;m>")
    # "$<$<OR:$<C_COMPILER_ID:Clang>,$<C_COMPILER_ID:AppleClang>,$<C_COMPILER_ID:GNU>>:$<$<CONFIG:DEBUG>:asan;ubsan>;m>")

target_link_libraries(indicators PUBLIC "$<$<C_COMPILER_ID:GNU>:$<$<CONFIG:DEBUG>:asan;ubsan>>")
target_link_libraries(indicators PUBLIC
    "$<$<OR:$<C_COMPILER_ID:Clang>,$<C_COMPILER_ID:AppleClang>,$<C_COMPILER_ID:GNU>>:m>")
target_link_options(indicators PUBLIC
    "$<$<OR:$<C_COMPILER_ID:Clang>,$<C_COMPILER_ID:AppleClang>,$<C_COMPILER_ID:GNU>>:$<$<CONFIG:DEBUG>:-fsanitize=undefined;-fsanitize=address;$<$<NOT:$<C_COMPILER_ID:AppleClang>>:-fsanitize=leak>>>")


# -fsanitize=address

set_target_properties(indicators PROPERTIES PUBLIC_HEADER "indicators.h")
install(TARGETS indicators
    PUBLIC_HEADER DESTINATION "include"
    ARCHIVE DESTINATION "lib"
    LIBRARY DESTINATION "lib")

target_compile_options(indicators
    PUBLIC
    $<$<OR:$<C_COMPILER_ID:Clang>,$<C_COMPILER_ID:AppleClang>,$<C_COMPILER_ID:GNU>>:
        -Wall -Wextra -Wshadow -g3 -ggdb -O2 -Werror=sequence-point -Werror=vla
        -Wno-unused-variable -Wno-float-conversion -Wno-unused-value -Wno-unused-parameter -Wno-newline-eof 
        $<$<C_COMPILER_ID:GNU>:-Wno-maybe-uninitialized>
        $<$<COMPILE_LANGUAGE:CXX>:-std=c++17 -pedantic>
        $<$<COMPILE_LANGUAGE:C>:-std=c99 -pedantic>
        $<$<CONFIG:DEBUG>:
            -fsanitize=undefined -fsanitize=address $<$<NOT:$<C_COMPILER_ID:AppleClang>>:-fsanitize=leak>>
        $<$<CONFIG:RELEASE>:
            -ffast-math -fno-finite-math-only>>
    $<$<C_COMPILER_ID:MSVC>:
        /std:c++17
        $<$<CONFIG:RELEASE>:
            /fp:fast>>
    PRIVATE
    $<$<C_COMPILER_ID:MSVC>:
        /DBUILDING>
)

execute_process(
    COMMAND ${Python3_EXECUTABLE} -m pip install -r requirements.txt
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
execute_process(
    COMMAND ${Python3_EXECUTABLE} codegen.py
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

# Only test the newly added indicator, based on the branch name

if ("${branch_name}" STREQUAL "")
    execute_process(
        COMMAND git rev-parse --abbrev-ref HEAD
        OUTPUT_VARIABLE branch_name
    )
endif()
string(STRIP "${branch_name}" branch_name)

execute_process(
    COMMAND ${CMAKE_COMMAND} -E echo ${branch_name}
    COMMAND ${Python3_EXECUTABLE} -c "pass; \
        s = input(); \
        candidate = s.split('/')[-1].split('-')[0]; \
        import yaml, os.path; \
        indicators = yaml.safe_load(open(os.path.join('${CMAKE_CURRENT_SOURCE_DIR}','indicators.yaml'))); \
        print(candidate, end='') if candidate in indicators else '' \
    "
    OUTPUT_VARIABLE indicator_name
)

if (${disable_single_testing})
    set(indicator_name "")
endif()

if ("${indicator_name}" STREQUAL "")
    message("no specific indicator")
else()
    message("current indicator: ${indicator_name}")
endif()


find_program(VALGRIND_CMD valgrind)
if (NOT "${VALGRIND_CMD}" STREQUAL "VALGRIND_CMD-NOTFOUND")
    set(VALGRIND_CMD ${VALGRIND_CMD} --error-exitcode=1)
else()
    set(VALGRIND_CMD "")
endif()
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(VALGRIND_CMD "")
endif()
    
message("${VALGRIND_CMD}")


## Tests
enable_testing()

add_executable(smoke tests/smoke.c ${utils})
target_link_libraries(smoke indicators)
file(TO_NATIVE_PATH "${CMAKE_BINARY_DIR}/smoke" path)
add_test(NAME smoke COMMAND ${VALGRIND_CMD} ${path} ${indicator_name} WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_executable(fuzzer tests/fuzzer.cc)
target_link_libraries(fuzzer indicators)
file(TO_NATIVE_PATH "${CMAKE_BINARY_DIR}/fuzzer" path)
add_test(NAME fuzzer COMMAND ${VALGRIND_CMD} ${path} ${indicator_name})

add_executable(benchmark2 tests/benchmark2.c ${utils})
target_link_libraries(benchmark2 indicators)
file(TO_NATIVE_PATH "${CMAKE_BINARY_DIR}/benchmark2" path)
if (NOT "${indicator_name}" STREQUAL "")
    add_test(NAME benchmark2 COMMAND ${VALGRIND_CMD} ${path} ${indicator_name})
else()
    add_test(NAME benchmark2 COMMAND                 ${path} ${indicator_name})
endif()

add_custom_target(check COMMAND ${VALGRIND_CMD} CTEST_OUTPUT_ON_FAILURE=1 ${CMAKE_CTEST_COMMAND})

## Bindings
add_custom_command(
    DEPENDS indicators
    COMMAND echo 'dir=${CMAKE_BINARY_DIR}'
    COMMAND ls -lah ${CMAKE_BINARY_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/libindicators.so ${CMAKE_SOURCE_DIR}/bindings/python/tindicators/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/libindicators.dylib ${CMAKE_SOURCE_DIR}/bindings/python/tindicators/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/indicators.dll ${CMAKE_SOURCE_DIR}/bindings/python/tindicators/
    COMMAND ls -lah ${CMAKE_SOURCE_DIR}/bindings/python/tindicators/
    COMMAND ${Python3_EXECUTABLE} -m pip install -e .[dev]
    COMMAND ${Python3_EXECUTABLE} setup.py bdist_wheel
    COMMAND ${Python3_EXECUTABLE} -m tox
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/bindings/python
    OUTPUT ${CMAKE_SOURCE_DIR}/bindings/python/dist/tindicators-${version}-py3-none-any.whl
)
add_custom_target(tindicators-python DEPENDS ${CMAKE_SOURCE_DIR}/bindings/python/dist/tindicators-${version}-py3-none-any.whl)
set_target_properties(tindicators-python PROPERTIES EXCLUDE_FROM_ALL 1)

add_custom_command(
    COMMAND ${Python3_EXECUTABLE} generate-bindings.py
    #COMMAND dotnet format
    COMMAND msbuild /t:restore
    COMMAND msbuild /t:pack /p:PackageVersion=${version} /p:Configuration=Release
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/bindings/lean
    OUTPUT ${CMAKE_SOURCE_DIR}/bindings/lean/bin/Release/tindicators-lean.${version}.nupkg
)
add_custom_target(tindicators-lean DEPENDS tindicators-python ${CMAKE_SOURCE_DIR}/bindings/lean/bin/Release/tindicators-lean.${version}.nupkg)
set_target_properties(tindicators-lean PROPERTIES EXCLUDE_FROM_ALL 1)


## Docs
find_package(Git)

add_custom_command(
    COMMAND ${GIT_EXECUTABLE} rev-parse --symbolic-full-name --abbrev-ref HEAD
    COMMAND ${GIT_EXECUTABLE} rev-parse --symbolic-full-name --abbrev-ref HEAD | xargs -I % -n 1 sh -c '[[ x"%" == x"master" ]]'
    COMMAND ${Python3_EXECUTABLE} docsgen.py docs.md
    COMMAND ${GIT_EXECUTABLE} checkout gh-pages
    COMMAND ${CMAKE_COMMAND} -E rename docs.md index.markdown
    COMMAND ${GIT_EXECUTABLE} add index.markdown
    COMMAND ${GIT_EXECUTABLE} commit -m 'docs updated'
    COMMAND ${GIT_EXECUTABLE} checkout -
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT ${CMAKE_SOURCE_DIR}/index.markdown
)
add_custom_target(docs DEPENDS ${CMAKE_SOURCE_DIR}/index.markdown)
set_target_properties(tindicators-lean PROPERTIES EXCLUDE_FROM_ALL 1)