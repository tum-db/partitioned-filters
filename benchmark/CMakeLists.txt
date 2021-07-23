add_library(filters_benchmark INTERFACE)
target_include_directories(filters_benchmark INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/>)
add_library(filters::benchmark ALIAS filters_benchmark)

file(GLOB_RECURSE BENCHMARK_JSON RELATIVE ${CMAKE_CURRENT_LIST_DIR} paper/**/*.json)

execute_process(COMMAND bash -c "rm -rf ${CMAKE_CURRENT_BINARY_DIR}/paper")

message(STATUS "[Benchmark] generate")
foreach (ONE_BENCHMARK_JSON ${BENCHMARK_JSON})
    get_filename_component(ONE_BENCHMARK_DIR ${ONE_BENCHMARK_JSON} DIRECTORY)
    get_filename_component(ONE_BENCHMARK_NAME ${ONE_BENCHMARK_JSON} NAME_WE)
    message(STATUS "    ${ONE_BENCHMARK_JSON}")
    execute_process(COMMAND bash -c "
                        cd  ${PROJECT_SOURCE_DIR}
                        source venv/bin/activate
                        python3 python/benchmark_generator/benchmark_generator.py \
                            ${CMAKE_CURRENT_LIST_DIR}/${ONE_BENCHMARK_JSON} \
                            ${CMAKE_CURRENT_BINARY_DIR}/${ONE_BENCHMARK_DIR}/${ONE_BENCHMARK_NAME}.cpp")
endforeach ()

file(GLOB_RECURSE BENCHMARK_CPP ${CMAKE_CURRENT_BINARY_DIR}/paper/*.cpp)

message(STATUS "[Benchmark] targets")
foreach (ONE_BENCHMARK_CPP ${BENCHMARK_CPP})

    get_filename_component(ONE_BENCHMARK_EXEC ${ONE_BENCHMARK_CPP} NAME_WE)

    # Avoid name collision
    set(TARGET_NAME "benchmark_${ONE_BENCHMARK_EXEC}")

    add_executable(${TARGET_NAME} ${ONE_BENCHMARK_CPP})
    set_target_properties(${TARGET_NAME} PROPERTIES OUTPUT_NAME ${TARGET_NAME})
    target_link_libraries(${TARGET_NAME} PRIVATE benchmark Threads::Threads filters::filters filters::benchmark perfevent::perfevent)
    if (NUMA_FOUND)
        target_link_libraries(${TARGET_NAME} INTERFACE ${NUMA_LIBRARY})
        target_compile_definitions(${TARGET_NAME} INTERFACE HAVE_NUMA)
    endif (NUMA_FOUND)


    message(STATUS "    ${TARGET_NAME}")
    list(APPEND benchmark_drivers ${TARGET_NAME})
endforeach ()
