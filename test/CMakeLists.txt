# Use files from tests
file(GLOB_RECURSE TEST_CPP ${CMAKE_CURRENT_LIST_DIR}/*.cpp)
file(GLOB_RECURSE TEST_HPP ${CMAKE_CURRENT_LIST_DIR}/*.hpp)

enable_testing()

message(STATUS "[TEST] targets")
foreach (ONE_TEST_CPP ${TEST_CPP})

    get_filename_component(ONE_TEST_EXEC ${ONE_TEST_CPP} NAME_WE)
    set(TARGET_NAME ${ONE_TEST_EXEC})

    add_executable(${TARGET_NAME} ${ONE_TEST_CPP} ${TEST_HPP})
    set_target_properties(${TARGET_NAME} PROPERTIES OUTPUT_NAME ${ONE_TEST_EXEC})
    target_link_libraries(${TARGET_NAME} PRIVATE gtest gmock Threads::Threads filters::filters perfevent::perfevent)

    add_test(filters ${TARGET_NAME})
    message(STATUS "    ${TARGET_NAME}")
endforeach ()
