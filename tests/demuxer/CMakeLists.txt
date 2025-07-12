# tests/demuxer/CMakeLists.txt

# 创建测试可执行文件
add_executable(test_demuxer test_demuxer.cpp)

# 链接必要的库
target_link_libraries(test_demuxer 
    demuxer
    utils
    ${FFMPEG_INSTALL_DIR}/lib/libavformat.a
    ${FFMPEG_INSTALL_DIR}/lib/libavcodec.a
    ${FFMPEG_INSTALL_DIR}/lib/libavutil.a
    ${FFMPEG_INSTALL_DIR}/lib/libswscale.a
    ${FFMPEG_INSTALL_DIR}/lib/libswresample.a
    pthread
    z  # zlib
    m  # math library
)

# 设置include目录
target_include_directories(test_demuxer PRIVATE 
    ${CMAKE_SOURCE_DIR}/src
    ${FFMPEG_INSTALL_DIR}/include
)

# 确保依赖ffmpeg
add_dependencies(test_demuxer ffmpeg)

# 添加测试
add_test(NAME DemuxerTest COMMAND test_demuxer)
