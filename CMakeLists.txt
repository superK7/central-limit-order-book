cmake_minimum_required(VERSION 3.10)

set(PROJECT_NAME_STR order_book)
project(${PROJECT_NAME_STR})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-std=c++17")

file(GLOB_RECURSE INC_FILES src/*/*.hpp)
file(GLOB_RECURSE SRC_FILES src/*/*.cpp)
configure_file(${PROJECT_SOURCE_DIR}/src/input.txt ${CMAKE_CURRENT_BINARY_DIR}/input.txt COPYONLY)

find_package(Boost COMPONENTS filesystem system unit_test_framework REQUIRED)

if(NOT Boost_FOUND)
    message("Not found Boost")
endif()

add_subdirectory(test)

include_directories(${Boost_INCLUDE_DIRS})

set(SOURCE_FILES ${PROJECT_SOURCE_DIR}/src/main.cpp ${INC_FILES} ${SRC_FILES})
add_executable(${PROJECT_NAME_STR} ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME_STR} ${Boost_LIBRARIES})
install(TARGETS ${PROJECT_NAME_STR} RUNTIME DESTINATION bin)
