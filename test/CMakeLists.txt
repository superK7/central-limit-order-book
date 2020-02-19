cmake_minimum_required(VERSION 3.10)

set(PROJECT_UNIT_TESTS_NAME ${PROJECT_NAME_STR}_test)

file(GLOB UNIT_TESTS_SRC_FILES ${PROJECT_SOURCE_DIR}/test/*.cpp)

find_package(Boost COMPONENTS filesystem system unit_test_framework REQUIRED)

set(UNIT_TESTS_INCLUDES ${PROJECT_INCLUDE_DIR}
        ${PROJECT_SOURCE_DIR}/src
        ${Boost_INCLUDE_DIR})

include_directories(${Boost_INCLUDE_DIRS})
include_directories(${UNIT_TESTS_INCLUDES})

add_definitions(-DBOOST_TEST_DYN_LINK)

add_executable(${PROJECT_UNIT_TESTS_NAME} ${UNIT_TESTS_SRC_FILES} ${SRC_FILES})
target_link_libraries(${PROJECT_UNIT_TESTS_NAME} ${Boost_LIBRARIES})

enable_testing()
add_test(NAME Test COMMAND ${PROJECT_UNIT_TESTS_NAME})