set(library_name filters)

set(CLANGED_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/reference/peter/peter_bloom_filter.cpp)
set(CLANGED_OBJECT ${CMAKE_CURRENT_BINARY_DIR}/peter_bloom_filter_clang.o)

add_custom_command(
        OUTPUT ${CLANGED_OBJECT}
        COMMAND clang++-20  -I${CMAKE_CURRENT_SOURCE_DIR} -I${CMAKE_CURRENT_SOURCE_DIR}/../lib/libdivide -I${CMAKE_CURRENT_SOURCE_DIR}/../lib/
                            -std=c++20 -O3 -mavx512f -c ${CLANGED_SOURCE} -o ${CLANGED_OBJECT}
        DEPENDS ${CLANGED_SOURCE}
)

# Wrap the object in a static library
add_library(peter_vectorized_bloom STATIC ${CLANGED_OBJECT})
target_link_libraries(peter_vectorized_bloom PRIVATE cityhash)
set_target_properties(peter_vectorized_bloom PROPERTIES LINKER_LANGUAGE CXX)
set_source_files_properties(${CLANGED_OBJECT} PROPERTIES EXTERNAL_OBJECT TRUE)
add_custom_target(peter_bloom_filter_clang_obj DEPENDS ${CLANGED_OBJECT})
add_dependencies(peter_vectorized_bloom peter_bloom_filter_clang_obj)

# Add vectorized function file with custom compile flags
#add_library(peter_vectorized_bloom STATIC reference/peter/peter_bloom_filter.cpp)

# Set custom compile flags for this file only (enable vectorization)
#target_compile_options(peter_vectorized_bloom PRIVATE -O3 -ftree-vectorize -mavx512f -g)

#get_target_property(_flags peter_vectorized_bloom COMPILE_OPTIONS)
#message(STATUS "Compile flags for peter_vectorized_bloom: ${_flags}")

# Optional: include directories for the file
#target_include_directories(peter_vectorized_bloom PRIVATE $<BUILD_INTERFACE:/home/schmidt/filters/src>)
#target_link_libraries(peter_vectorized_bloom PRIVATE cityhash libdivide)


# build library
add_library(${library_name} INTERFACE)
target_include_directories(${library_name} INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>)
target_link_libraries(${library_name} INTERFACE cityhash libdivide fastfilter impala efficient_cuckoofilter amd_mortonfilter vacuumfilter peter_vectorized_bloom)

if (NUMA_FOUND)
    target_link_libraries(${library_name} INTERFACE ${NUMA_LIBRARY})
    target_compile_definitions(${library_name} INTERFACE HAVE_NUMA)
endif (NUMA_FOUND)

if (ENABLE_BSD)
    target_compile_definitions(${library_name} INTERFACE ENABLE_BSD)
    target_link_libraries(${library_name} INTERFACE boost bsd)
endif ()

add_library(${library_name}::${library_name} ALIAS ${library_name})
