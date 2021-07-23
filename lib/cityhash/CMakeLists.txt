get_filename_component(library_name ${CMAKE_CURRENT_SOURCE_DIR} NAME)
add_library(${library_name} STATIC city.cc)
target_include_directories(${library_name} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/..)
