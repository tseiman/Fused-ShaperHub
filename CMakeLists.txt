


# set(CMAKE_VERBOSE_MAKEFILE ON)

cmake_minimum_required(VERSION 3.1...3.29)

project(
    fused-shaperhub
    VERSION 1.0
    LANGUAGES C)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_FILE_OFFSET_BITS=64")

#set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} \
#-D_FILE_OFFSET_BITS=64 \
#-DFUSE_USE_VERSION=26 \
#-D_FORTIFY_SOURCE=2  \
#-D_GLIBCXX_ASSERTIONS  \
#-O2 \
#-fasynchronous-unwind-tables  \
#-fexceptions  \
#-fno-omit-frame-pointer \
#")


set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} \
-D_FILE_OFFSET_BITS=64 \
-DFUSE_USE_VERSION=26 \
-D_FORTIFY_SOURCE=2  \
-D_GLIBCXX_ASSERTIONS  \
-O0 -g \
-fasynchronous-unwind-tables  \
-fexceptions  \
-fno-omit-frame-pointer \
")


#set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} \
#-D_FILE_OFFSET_BITS=64 \
#-D_FORTIFY_SOURCE=2  \
#-D_GLIBCXX_ASSERTIONS  \
#-O2 \
#-fasynchronous-unwind-tables  \
#-fexceptions  \
#")

#-fpie  \
#-pie -fpic \
#-shared  \
#-fstack-clash-protection  \
#-fstack-protector-all  \
#-fstack-protector-strong  \
#-pipe \
#-Werror=format-security \
#-z defs -z now -z relro \
#-Wall 


set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake" ${CMAKE_MODULE_PATH})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)


OPTION(IS_MOC "This enables a mocup build - filling the FUSE FS with demo folders and files" OFF)

IF(IS_MOC)
    ADD_DEFINITIONS(-DIS_MOC)
ENDIF(IS_MOC)

set(SOURCES
    src/fused-shaperhub.c
    src/fuse-dataloader.c
    src/http-connector.c
    src/data-container.c
    src/open-file-manager.c
    src/config.c
    src/alloc.c
    src/main.c
)
add_executable(fused-shaperhub ${SOURCES})

find_package(FUSE REQUIRED)
include_directories(${FUSE_INCLUDE_DIRS})
target_link_libraries(fused-shaperhub ${FUSE_LIBRARIES})

find_package(Jansson REQUIRED)
include_directories(${JANSSON_INCLUDE_DIRS})
target_link_libraries(fused-shaperhub ${JANSSON_LIBRARIES})

find_package(CURL REQUIRED)
include_directories(${CURL_INCLUDE_DIRS})
target_link_libraries(fused-shaperhub ${CURL_LIBRARIES})

include_directories(include)

