cmake_minimum_required(VERSION 3.10)

project(MyProject)

# 设置可执行文件输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

# 指定源文件和头文件的搜索路径
set(SOURCE_DIR "${PROJECT_SOURCE_DIR}/src")
set(INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include")

# 递归搜索所有源文件(.cpp, .cc等)
file(GLOB_RECURSE SRC_FILES
        ${SOURCE_DIR}/*.cpp
        ${SOURCE_DIR}/*.cc
)


# 包含并构建 yaml-cpp 子项目
add_subdirectory(3rd/yaml-cpp)

# 添加可执行文件
add_executable(netproxy ${SRC_FILES}
        include/log.h
        src/log.cpp
        src/main.cpp
)

# 包含头文件目录
target_include_directories(netproxy PRIVATE ${INCLUDE_DIR})

# 链接 yaml-cpp 库
target_link_libraries(netproxy PRIVATE yaml-cpp::yaml-cpp)

# ========== 复制共享库到 bin 目录（仅当使用共享库时）==========
# 检查是否构建的是共享库（.so 或 .dll）
get_target_property(YAML_LIB_TYPE yaml-cpp::yaml-cpp TYPE)
if(YAML_LIB_TYPE STREQUAL "SHARED_LIBRARY")
    # 获取 yaml-cpp 库文件路径
    get_target_property(YAML_LIB_LOCATION yaml-cpp::yaml-cpp LOCATION)

    # 在构建 netproxy 后，复制 .so 文件到 bin/
    add_custom_command(
            TARGET netproxy POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:yaml-cpp::yaml-cpp>"
            "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<TARGET_FILE_NAME:yaml-cpp::yaml-cpp>"
            COMMENT "Copying yaml-cpp shared library to bin/"
    )
endif()