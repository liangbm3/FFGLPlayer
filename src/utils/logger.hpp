#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <ctime>
#include <thread>
#include <iomanip>
#include <chrono>

namespace utils
{

    struct Color
    {
        static constexpr const char *RESET = "\033[0m";
        static constexpr const char *RED = "\033[31m";
        static constexpr const char *GREEN = "\033[32m";
        static constexpr const char *YELLOW = "\033[33m";
        static constexpr const char *BLUE = "\033[34m";
        static constexpr const char *MAGENTA = "\033[35m";
        static constexpr const char *CYAN = "\033[36m";
        static constexpr const char *WHITE = "\033[37m";
        static constexpr const char *BOLD = "\033[1m";
    };

    // 强枚举类型，不会隐式转换为整数
    // 显式赋值可以通过>=运算符来比较日志级别
    enum class LogLevel
    {
        DEBUG = 0,
        INFO = 1,
        WARN = 2,
        ERROR = 3,
        FATAL = 4 // 致命的
    };

    class Logger
    {
    public:
        class LogStream
        {
        public:
            LogStream(LogLevel level, const char *file, const char *function, int line)
                : level_(level), file_(getFileName(file)), function_(function), line_(line)
            {
                // 只有当日志级别满足条件时才添加时间戳和头部信息
                if (level_ >= globalLevel_) {
                    // 添加时间戳
                    auto now = std::chrono::system_clock::now();//获取当前系统时间点
                    auto now_time_t = std::chrono::system_clock::to_time_t(now);//将时间点转换为time_t类型
                    auto now_tm = *std::localtime(&now_time_t);//将time_t转换为本地时间结构体（年月日时分秒）
                    // 计算当前时间点距离 epoch（1970年1月1日）经过的微秒数，并取模 1000000，得到当前秒内的微秒部分。
                    auto now_ms = std::chrono::duration_cast<std::chrono::microseconds>(
                                      now.time_since_epoch()) %
                                  1000000;
                    char buffer[80];
                    //将当前时间格式化为字符串，包含年月日时分秒
                    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &now_tm);
                    //将日志头部信息写入日志流
                    stream_ << Color::CYAN << "[" << buffer << "."
                                << std::setfill('0') << std::setw(6) << now_ms.count() << "] "
                                << getLevelColor(level_) << Color::BOLD << "[" << getLevelStr(level_) << "] "
                                << Color::MAGENTA << "[" << std::this_thread::get_id() << "] "
                                << Color::BLUE << "[" << file_ << ":" << line_ << "] "
                                << Color::CYAN << "[" << function_ << "] "
                                << getLevelColor(level_);
                }
            }
            //析构函数自动输出日志
            ~LogStream()
            {
                if (level_ >= globalLevel_) // 只有当当前日志级别大于等于全局日志级别时才输出
                {
                    stream_ << Color::RESET << std::endl;//重置颜色
                    std::cout << stream_.str(); // 输出日志内容
                    std::cout.flush(); // 刷新输出流

                    // 如果是错误或者致命错误，输出到stderr
                    if(level_>= LogLevel::ERROR)
                    {
                        std::cerr << stream_.str(); // 输出错误日志内容
                        std::cerr.flush(); // 刷新错误输出流
                    }
                }
            }
            //重载<<运算符
            template <typename T>
            LogStream &operator<<(const T &value)
            {
                if (level_ >= globalLevel_) // 只有当当前日志级别大于等于全局日志级别时才添加内容
                {
                    stream_ << value; // 将值添加到日志流中
                }
                return *this; // 返回自身以支持链式调用
            }
        private:
            std::ostringstream stream_; // 内存字符串流
            LogLevel level_;
            const char *file_;
            const char *function_;
            int line_;
            // 获取文件名
            static const char *getFileName(const char *filePath)
            {
                const char *fileName = filePath;
                // 每遇到一个分隔符，就把fileName指向后面的分隔符
                for (const char *p = filePath; *p; p++)
                {
                    if (*p == '/' || *p == '\\')
                    {
                        fileName = p + 1;
                    }
                }
                return fileName;
            }
            // 返回日志级别字符串
            static const char *getLevelStr(LogLevel level)
            {
                switch (level)
                {
                case LogLevel::DEBUG:
                    return "DEBUG";
                case LogLevel::INFO:
                    return "INFO";
                case LogLevel::WARN:
                    return "WARN";
                case LogLevel::ERROR:
                    return "ERROR";
                case LogLevel::FATAL:
                    return "FATAL";
                default:
                    return "UNKNOWN";
                }
            }
            // 根据日志级别返回颜色格式
            static const char *getLevelColor(LogLevel level)
            {
                switch (level)
                {
                case LogLevel::DEBUG:
                    return Color::RESET;
                case LogLevel::INFO:
                    return Color::GREEN;
                case LogLevel::WARN:
                    return Color::YELLOW;
                case LogLevel::ERROR:
                    return Color::RED;
                case LogLevel::FATAL:
                    return Color::RED;
                default:
                    return Color::RESET;
                }
            }
        };
        // 设置全局日志等级
        static void setGlobalLevel(LogLevel level)
        {
            globalLevel_ = level;
        }

        // 获取全局日志等级
        static LogLevel getGlobalLevel()
        {
            return globalLevel_;
        }
        static LogStream Debug(const char *file, const char *function, int line)
        {
            return LogStream(LogLevel::DEBUG, file, function, line);
        }

        static LogStream Info(const char *file, const char *function, int line)
        {
            return LogStream(LogLevel::INFO, file, function, line);
        }

        static LogStream Warn(const char *file, const char *function, int line)
        {
            return LogStream(LogLevel::WARN, file, function, line);
        }

        static LogStream Error(const char *file, const char *function, int line)
        {
            return LogStream(LogLevel::ERROR, file, function, line);
        }

        static LogStream Fatal(const char *file, const char *function, int line)
        {
            return LogStream(LogLevel::FATAL, file, function, line);
        }

    private:
        static LogLevel globalLevel_; // 全局日志等级
    };

    // 在C++17之前，静态成员变量只能在类外的.cpp文件中定义一次；
    // C++17及以后，使用inline可以直接在头文件定义和初始化静态成员变量，且多次包含头文件不会导致重复定义
    inline LogLevel Logger::globalLevel_ = LogLevel::INFO;
}

// 定义便捷宏
// 每个宏会自动传递当前的源文件名，函数名和行号给日志系统，方便定位日志来源
#define LOG_DEBUG utils::Logger::Debug(__FILE__, __FUNCTION__, __LINE__)
#define LOG_INFO utils::Logger::Info(__FILE__, __FUNCTION__, __LINE__)
#define LOG_WARN utils::Logger::Warn(__FILE__, __FUNCTION__, __LINE__)
#define LOG_ERROR utils::Logger::Error(__FILE__, __FUNCTION__, __LINE__)
#define LOG_FATAL utils::Logger::Fatal(__FILE__, __FUNCTION__, __LINE__)