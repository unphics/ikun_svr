cmake_minimum_required(VERSION 3.22) # cmake最低版本, 非必须填写

# 配置项目
project(last, # 项目名
        LANGUAGES CXX # 项目使用的语言
        VERSION 0.1.1 # 项目版本
        DESCRIPTION "game server" # 项目描述
        HOMEPAGE_URL "no url")
set(CMAKE_CXX_STANDARD 23) # 增加-std=c++23
# set(EXECUTABLE_OUTPUT_PATH ) # 指定输出路径

# 打印出上面的信息
message("Project Name: ${PROJECT_NAME}")
message("Project Version: ${PROJECT_VERSION}")
message("Project Description: ${PROJECT_DESCRIPTION}")

# 链接项目文件
set(src_dir ${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${src_dir})
include_directories(${src_dir}/public/) # 添加包含路径(不用写全路径, 添加直达路径)
include_directories(${src_dir}/proto/)
# aux_source_directory(${PROJECT_SOURCE_DIR} src_file) # PROJECT_SOURCE_DIR就是cmake命令后面跟的路径就是..的路径
file(GLOB_RECURSE src_file ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc) # CMAKE_CURRENT_SOURCE_DIR是cmakelists所在路径

# 链接依赖库protobuf
find_package(Protobuf REQUIRED)
if (PROTOBUF_FOUND)
    message(STATUS "protobuf lib found")
else()
    message(FATAL_ERROR "protobuf lib is essential but not found")
endif()
include_directories(${PROTOBUF_INCLUDE_DIRS})

# 链接静态库
# include_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib/so_2/include/)
# link_libraries(so_2)
# link_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib/so_2/build/)

# 动态库的路径(写在生成exec之前)
# link_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib/so_1/build/)
# include_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib/so_1/src/)

# 生成
# add_library(so_2 STATIC ${src_file}) # SHARED是动态库, 动态库是有可执行权限的
add_executable(last ${src_file}) # 程序生成的可执行程序
set_target_properties(last PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build)

# 定义宏
add_definitions(-DDEBUG)

# add_subdirectory() # 

# 链接动态库(写在生成exec之后, 因为要保证链接的对象存在)
# 动态库的链接具有传递性, 链接了abc则bc的数据会传递给前面的a
# 在物理内存中, 动态库有且仅有一份, 所以可能被多个程序使用
# target_link_libraries(last so_1)

target_link_libraries(last ${PROTOBUF_LIBRARIES})