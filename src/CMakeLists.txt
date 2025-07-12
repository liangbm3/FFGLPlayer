# src/CMakeLists.txt

# 设置源文件
set(UTILS_SOURCES
    utils/logger.cpp
)

# 创建utils静态库
add_library(utils STATIC ${UTILS_SOURCES})

# 设置include目录
target_include_directories(utils PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/utils
)

# 链接必要的系统库
target_link_libraries(utils 
    pthread  # 用于std::thread
)

# 设置编译选项
target_compile_features(utils PUBLIC cxx_std_17)

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
