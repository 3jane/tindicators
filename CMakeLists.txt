cmake_minimum_required(VERSION 3.12.4)
find_package(Python3 COMPONENTS Interpreter)
execute_process(
    COMMAND ${Python3_EXECUTABLE} -c "import version; print(version.version, end='')"
    OUTPUT_VARIABLE version
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

project(tulipindicators VERSION "${version}" LANGUAGES C CXX)

## Library
file(GLOB src indicators/*)
file(GLOB utils utils/*)
add_library(indicators SHARED ${src} ${utils} indicators.h indicators_index.c)
target_link_libraries(indicators PUBLIC
    "$<$<OR:$<C_COMPILER_ID:Clang>,$<C_COMPILER_ID:AppleClang>,$<C_COMPILER_ID:GNU>>:$<$<CONFIG:DEBUG>:asan;ubsan>;m>")

set_target_properties(indicators PROPERTIES PUBLIC_HEADER "indicators.h")
install(TARGETS indicators
    PUBLIC_HEADER DESTINATION "include"
    ARCHIVE DESTINATION "lib"
    LIBRARY DESTINATION "lib")

target_compile_options(indicators
    PUBLIC
    $<$<OR:$<C_COMPILER_ID:Clang>,$<C_COMPILER_ID:AppleClang>,$<C_COMPILER_ID:GNU>>:
        -Wall -Wextra -Wshadow -Wconversion -g3 -ggdb -O2 -Werror=sequence-point
        $<$<COMPILE_LANGUAGE:CXX>:-fpermissive -std=c++17>
        $<$<COMPILE_LANGUAGE:C>:-std=c99 -pedantic>
        $<$<CONFIG:DEBUG>:
            -fsanitize=undefined -fsanitize=address -fsanitize=leak>
        $<$<CONFIG:RELEASE>:
            -ffast-math>>
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
    OUTPUT indicators.h indicators_index.c
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)


# Only test the newly added indicator, based on the branch name

if ("${branch_name}" STREQUAL "")
    execute_process(
        COMMAND git rev-parse --abbrev-ref HEAD
        OUTPUT_VARIABLE branch_name
    )
endif()
string(STRIP ${branch_name} branch_name)

execute_process(
    COMMAND ${CMAKE_COMMAND} -E echo ${branch_name}
    COMMAND ${Python3_EXECUTABLE} -c "pass; \
        import re; \
        m = re.match('[^/]*/[^/]*/([a-zA-Z0-9]+)(-.*)?', input()); \
        import yaml, os.path; \
        indicators = yaml.safe_load(open(os.path.join('${CMAKE_CURRENT_SOURCE_DIR}','indicators.yaml'))); \
        print(m[1], end='') if m and m[1] in indicators else ''; \
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


# Samples
# failed to build on MSVC:
# add_executable(example1 example1.c)
# target_link_libraries(example1 indicators)

add_executable(example2 example2.c)
target_link_libraries(example2 indicators)

add_executable(sample sample.c)
target_link_libraries(sample indicators)

# not ready yet:
# file(GLOB src-h utils/*.h indicators/*.h)
# add_custom_command(
#     OUTPUT tiamalgamation.c
#     COMMAND ${CMAKE_C_COMPILER} ${CMAKE_C_FLAGS} -E -P -DTI_SKIP_SYSTEM_HEADERS -D__TI_INDICATORS_H__ -D__BUFFER_H__ -D__MINMAX_H__ ${src} | cat - ${src-h} > tiamalgamation.c
# )

# CC -E -P $CFLAGS -DTI_SKIP_SYSTEM_HEADERS -D__TI_INDICATORS_H__ -D__BUFFER_H__ -D__MINMAX_H__ |
# cat <(CC -E -P $CFLAGS -DTI_SKIP_SYSTEM_HEADERS -D__TI_INDICATORS_H__ -D__BUFFER_H__ -D__MINMAX_H__) <(**/*.h) > tiamalgamation.c

## Tests
enable_testing()

add_executable(smoke smoke.c ${utils})
target_link_libraries(smoke indicators)
add_test(NAME smoke COMMAND smoke ${indicator_name} WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_executable(fuzzer fuzzer.c)
target_link_libraries(fuzzer indicators)
add_test(NAME fuzzer COMMAND fuzzer ${indicator_name})

add_executable(benchmark benchmark.c)
set_target_properties(benchmark PROPERTIES EXCLUDE_FROM_ALL 1)
target_link_libraries(benchmark indicators ta_lib)
#add_test(NAME benchmark COMMAND benchmark ${indicator_name})

add_executable(benchmark2 benchmark2.c ${utils})
target_link_libraries(benchmark2 indicators)
add_test(NAME benchmark2 COMMAND benchmark2 ${indicator_name})

add_custom_target(check COMMAND CTEST_OUTPUT_ON_FAILURE=1 ${CMAKE_CTEST_COMMAND})

## Bindings
add_custom_command(
    COMMAND echo 'dir=${BINARIES}'
    COMMAND ls -lah ${BINARIES}
    COMMAND ${CMAKE_COMMAND} -E copy ${BINARIES}/libindicators.so ${CMAKE_SOURCE_DIR}/bindings/python/tulipindicators/
    COMMAND ${CMAKE_COMMAND} -E copy ${BINARIES}/indicators.dll ${CMAKE_SOURCE_DIR}/bindings/python/tulipindicators/
    COMMAND ls -lah ${CMAKE_SOURCE_DIR}/bindings/python/tulipindicators/
    COMMAND ${Python3_EXECUTABLE} -m pip install -e .[dev]
    COMMAND ${Python3_EXECUTABLE} setup.py bdist_wheel
    COMMAND ${Python3_EXECUTABLE} -m tox
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/bindings/python
    OUTPUT ${CMAKE_SOURCE_DIR}/bindings/python/dist/tulipindicators-${version}-py3-none-any.whl
)
add_custom_target(tulipindicators-python DEPENDS ${CMAKE_SOURCE_DIR}/bindings/python/dist/tulipindicators-${version}-py3-none-any.whl)
set_target_properties(tulipindicators-python PROPERTIES EXCLUDE_FROM_ALL 1)

add_custom_command(
    COMMAND ${Python3_EXECUTABLE} generate-bindings.py
    #COMMAND dotnet format
    COMMAND msbuild /t:restore
    COMMAND msbuild /t:pack /p:PackageVersion=${version} /p:Configuration=Release
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/bindings/lean
    OUTPUT ${CMAKE_SOURCE_DIR}/bindings/lean/bin/Release/tulipindicators-lean.${version}.nupkg
)
add_custom_target(tulipindicators-lean DEPENDS tulipindicators-python ${CMAKE_SOURCE_DIR}/bindings/lean/bin/Release/tulipindicators-lean.${version}.nupkg)
set_target_properties(tulipindicators-lean PROPERTIES EXCLUDE_FROM_ALL 1)