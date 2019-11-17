cmake_minimum_required(VERSION 2.8)

project(slob)

find_package(ICU COMPONENTS uc io REQUIRED)

file(GLOB SOURCES src/*.cpp)

include_directories(include)

set(CMAKE_BUILD_TYPE Release)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_link_libraries(slob lzma z ICU::uc ICU::io)
add_definitions(-DU_CHARSET_IS_UTF8)

install(TARGETS ${PROJECT_NAME} DESTINATION lib)

file(GLOB HEADERS include/*.h)
install(FILES ${HEADERS} DESTINATION include/${PROJECT_NAME})
