# src/CMakeLists.txt

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# utils源文件
set(UTILS_SOURCES
    utils/logger.cpp
)

set(DEMUXER_SOURCES
    demuxer/demuxer.cpp
)

# 创建utils静态库
add_library(utils STATIC ${UTILS_SOURCES})

# 创建demuxer静态库
add_library(demuxer STATIC ${DEMUXER_SOURCES})

# 设置demuxer的include目录
target_include_directories(demuxer PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/demuxer
    ${FFMPEG_INSTALL_DIR}/include
)

# 链接FFmpeg库到demuxer
target_link_libraries(demuxer
    ${FFMPEG_INSTALL_DIR}/lib/libavformat.a
    ${FFMPEG_INSTALL_DIR}/lib/libavcodec.a
    ${FFMPEG_INSTALL_DIR}/lib/libavutil.a
    ${FFMPEG_INSTALL_DIR}/lib/libswscale.a
    ${FFMPEG_INSTALL_DIR}/lib/libswresample.a
)

# 确保demuxer依赖ffmpeg
add_dependencies(demuxer ffmpeg)

# 设置utils的include目录
target_include_directories(utils PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/utils
)

# 链接必要的系统库
target_link_libraries(utils 
    pthread  # 用于std::thread
)

# 设置单个标准
# target_compile_features(utils PUBLIC cxx_std_17)

# 如果有其他源文件，可以在这里添加主程序
# set(MAIN_SOURCES
#     main.cpp
#     demuxer/demuxer.cpp
# )

# add_executable(FFGLPlayer ${MAIN_SOURCES})
# target_link_libraries(FFGLPlayer 
#     utils
#     ${FFMPEG_LIBRARIES}
#     glfw
# )
# target_include_directories(FFGLPlayer PRIVATE ${FFMPEG_INCLUDE_DIR})
# add_dependencies(FFGLPlayer ffmpeg)
