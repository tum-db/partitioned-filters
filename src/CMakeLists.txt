set(library_name filters)

# build library
add_library(${library_name} STATIC)
target_include_directories(${library_name} INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>)
target_link_libraries(${library_name} INTERFACE cityhash libdivide fastfilter impala efficient_cuckoofilter amd_mortonfilter vacuumfilter)

if (NUMA_FOUND)
    target_link_libraries(${library_name} INTERFACE ${NUMA_LIBRARY})
    target_compile_definitions(${library_name} INTERFACE HAVE_NUMA)
endif (NUMA_FOUND)

if (ENABLE_BSD)
    target_compile_definitions(${library_name} INTERFACE ENABLE_BSD)
    target_link_libraries(${library_name} INTERFACE boost bsd)
endif ()

add_library(${library_name}::${library_name} ALIAS ${library_name})
