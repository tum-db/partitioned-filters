get_filename_component(library_name fastfilter NAME)
add_library(${library_name} INTERFACE)
target_include_directories(${library_name} INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>)
