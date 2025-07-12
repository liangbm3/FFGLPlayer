# tests/utils/CMakeLists.txt

# 创建日志系统测试可执行文件
add_executable(test_logger test_logger.cpp)

# 设置可执行文件的输出名称（可选，使其更明确）
set_target_properties(test_logger PROPERTIES
    OUTPUT_NAME "test_logger"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tests/utils"
)

# 链接utils库
target_link_libraries(test_logger 
    utils
    pthread
)

# 设置include目录
target_include_directories(test_logger PRIVATE 
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/utils
)

# 设置编译选项
target_compile_features(test_logger PRIVATE cxx_std_17)

# 添加到CTest测试套件
add_test(NAME LoggerTest COMMAND test_logger)

# 设置测试属性
set_tests_properties(LoggerTest PROPERTIES
    TIMEOUT 30  # 设置超时时间为30秒
    LABELS "unit"  # 为测试添加标签
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tests/utils  # 设置工作目录
)

# 如果需要，可以添加更多测试
# add_executable(test_logger_performance test_logger_performance.cpp)
# target_link_libraries(test_logger_performance utils pthread)
# add_test(NAME LoggerPerformanceTest COMMAND test_logger_performance)
