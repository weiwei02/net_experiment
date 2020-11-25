cmake_minimum_required(VERSION 3.7)
project(net_exp)

#set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libstdc++ -static-libgcc  -static")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE "Debug")

# add use llvm toolschain
SET (CMAKE_C_COMPILER            "/usr/bin/clang")
SET (CMAKE_CXX_COMPILER            "/usr/bin/clang++")
SET (CMAKE_AR     "/usr/bin/llvm-ar")
SET (CMAKE_LINKER "/usr/bin/lld")
SET (CMAKE_NM     "/usr/bin/llvm-nm")
SET (CMAKE_OBJDUMP"/usr/bin/llvm-objdump")
SET (CMAKE_RANLIB "/usr/bin/llvm-ranlib")

#设置安装目录
set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR}/target)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build/lib)
#设置执行文件存放路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/build/bin)
set(TARGET_DIR ${PROJECT_SOURCE_DIR}/target)
set(EXTERN_LIB_PATH ${PROJECT_SOURCE_DIR}/lib)






function(add_static_lib lname)
    add_library(${lname} STATIC IMPORTED)
    set_property(TARGET ${lname} PROPERTY IMPORTED_LOCATION ${EXTERN_LIB_PATH}/lib${lname}/lib${lname}.a)
endfunction()

#SET(CMAKE_MODULE_PATH ${CMAKE_ROOT}/Modules ${CMAKE_SOURCE_DIR}/cmake/modules)
include_directories(SYSTEM
        ${PROJECT_SOURCE_DIR}/lib/libevent
        ${PROJECT_SOURCE_DIR}/source
        )
add_static_lib(event)

add_subdirectory(source/common)
add_subdirectory(source/exe)
add_subdirectory(source/tcp)
add_subdirectory(test/event)
add_subdirectory(test/tcp)
add_subdirectory(test/zero_copy)
