cmake_minimum_required(VERSION 3.12)
project(FFGLPlayer VERSION 1.0)

# 设置 C++ 标准为 C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 配置 FFmpeg 源码和安装路径
set(FFMPEG_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/ffmpeg)
set(FFMPEG_INSTALL_DIR ${FFMPEG_SOURCE_DIR}/dist)

# FFmpeg 编译选项（只编译动态库，不编译文档和工具）
set(FFMPEG_CONFIGURE_OPTIONS
    --prefix=${FFMPEG_INSTALL_DIR}
    --disable-doc
    --disable-programs
    --enable-shared
)

# 使用 ExternalProject 自动下载和编译 FFmpeg
include(ExternalProject)
ExternalProject_Add(ffmpeg
    SOURCE_DIR ${FFMPEG_SOURCE_DIR}
    BINARY_DIR ${FFMPEG_SOURCE_DIR}/build
    CONFIGURE_COMMAND ${FFMPEG_SOURCE_DIR}/configure ${FFMPEG_CONFIGURE_OPTIONS}
    BUILD_COMMAND make -j8
    INSTALL_COMMAND make install
    BUILD_IN_SOURCE 0
)

# 设置 FFmpeg 头文件和库路径
set(FFMPEG_INCLUDE_DIR ${FFMPEG_INSTALL_DIR}/include)
set(FFMPEG_LIBRARY_DIR ${FFMPEG_INSTALL_DIR}/lib)

# 根据平台设置 FFmpeg 动态库后缀
if(APPLE)
    set(FFMPEG_LIB_EXT "dylib")
elseif(UNIX)
    set(FFMPEG_LIB_EXT "so")
endif()

# FFmpeg 相关库文件列表
set(FFMPEG_LIBRARIES
    ${FFMPEG_LIBRARY_DIR}/libavcodec.${FFMPEG_LIB_EXT}
    ${FFMPEG_LIBRARY_DIR}/libavformat.${FFMPEG_LIB_EXT}
    ${FFMPEG_LIBRARY_DIR}/libavutil.${FFMPEG_LIB_EXT}
    ${FFMPEG_LIBRARY_DIR}/libswscale.${FFMPEG_LIB_EXT}
    ${FFMPEG_LIBRARY_DIR}/libavdevice.${FFMPEG_LIB_EXT}
    ${FFMPEG_LIBRARY_DIR}/libswresample.${FFMPEG_LIB_EXT}
)

# 查找 glfw3 库（用于 OpenGL 窗口）
find_package(glfw3 REQUIRED)

# 启用测试
enable_testing()

# 添加源码子目录
add_subdirectory(src)

# 添加测试子目录
add_subdirectory(tests)

