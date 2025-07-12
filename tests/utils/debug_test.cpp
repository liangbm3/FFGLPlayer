#include "../../src/utils/logger.hpp"
#include <iostream>

using namespace utils;

int main() {
    std::cout << "调试开始..." << std::endl;
    
    Logger::setGlobalLevel(LogLevel::ERROR);
    std::cout << "设置日志级别为ERROR" << std::endl;
    
    std::cout << "创建LOG_INFO..." << std::endl;
    LOG_INFO << "这条消息应该被过滤";
    std::cout << "LOG_INFO完成" << std::endl;
    
    std::cout << "调试结束" << std::endl;
    return 0;
}
