cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(mount-octave VERSION 0.0.1 LANGUAGES C)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_FILE_OFFSET_BITS=64")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall --pedantic -g")
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake" ${CMAKE_MODULE_PATH})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

find_package(FUSE REQUIRED)
find_package(CURL REQUIRED)
find_package(Jansson REQUIRED)

include_directories(${FUSE_INCLUDE_DIR} ${CURL_INCLUDE_DIR} ${JANSSON_INCLUDE_DIR})
add_executable(mount-octave fuse-octave.c fuse-dataloader.c http-connector.c json-handler.c data-container.c config.c main.c)
target_link_libraries(mount-octave ${FUSE_LIBRARIES} ${CURL_LIBRARIES} ${JANSSON_LIBRARIES})
