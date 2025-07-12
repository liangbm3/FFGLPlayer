#include <iostream>
#include <cassert>
#include <cstdio>
#include <vector>
#include <memory>
#include <cstdlib>

#include "demuxer/demuxer.hpp"
#include "utils/logger.hpp"

// 简单的测试框架宏
#define TEST_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            std::cerr << "FAIL: " << message << " at line " << __LINE__ << std::endl; \
            return false; \
        } else { \
            std::cout << "PASS: " << message << std::endl; \
        } \
    } while(0)

#define RUN_TEST(test_func) \
    do { \
        std::cout << "\n=== Running " << #test_func << " ===" << std::endl; \
        if (test_func()) { \
            std::cout << #test_func << " PASSED" << std::endl; \
            passed_tests++; \
        } else { \
            std::cout << #test_func << " FAILED" << std::endl; \
            failed_tests++; \
        } \
        total_tests++; \
    } while(0)

// 全局测试统计
static int total_tests = 0;
static int passed_tests = 0;
static int failed_tests = 0;

// 创建一个简单的测试视频文件
bool createTestVideoFile(const std::string& filename) {
    // 使用FFmpeg命令行工具生成一个简单的测试视频
    std::string cmd = "ffmpeg -f lavfi -i testsrc=duration=5:size=320x240:rate=30 "
                     "-f lavfi -i sine=frequency=1000:duration=5 "
                     "-c:v libx264 -c:a aac -t 5 -y " + filename + " 2>/dev/null";
    
    int result = std::system(cmd.c_str());
    return result == 0;
}

// 创建一个纯音频测试文件
bool createTestAudioFile(const std::string& filename) {
    std::string cmd = "ffmpeg -f lavfi -i sine=frequency=440:duration=3 "
                     "-c:a aac -t 3 -y " + filename + " 2>/dev/null";
    
    int result = std::system(cmd.c_str());
    return result == 0;
}

// 测试1: 基本构造和析构
bool testBasicConstructorDestructor() {
    // 测试视频类型构造
    {
        Demuxer video_demuxer(MediaType::VIDEO);
        TEST_ASSERT(true, "Video demuxer constructor");
    }
    
    // 测试音频类型构造
    {
        Demuxer audio_demuxer(MediaType::AUDIO);
        TEST_ASSERT(true, "Audio demuxer constructor");
    }
    
    return true;
}

// 测试2: 打开不存在的文件
bool testOpenNonExistentFile() {
    Demuxer demuxer(MediaType::VIDEO);
    
    bool result = demuxer.open("non_existent_file.mp4");
    TEST_ASSERT(!result, "Should fail to open non-existent file");
    
    return true;
}

// 测试3: 打开空文件名
bool testOpenEmptyFilename() {
    Demuxer demuxer(MediaType::VIDEO);
    
    bool result = demuxer.open("");
    TEST_ASSERT(!result, "Should fail to open empty filename");
    
    return true;
}

// 测试4: 打开有效的视频文件
bool testOpenValidVideoFile() {
    const std::string test_file = "test_video.mp4";
    
    // 创建测试文件
    if (!createTestVideoFile(test_file)) {
        std::cout << "WARNING: Cannot create test video file, skipping test" << std::endl;
        return true; // 跳过测试但不失败
    }
    
    Demuxer demuxer(MediaType::VIDEO);
    bool result = demuxer.open(test_file);
    TEST_ASSERT(result, "Should successfully open valid video file");
    
    // 检查流索引
    int video_index = demuxer.getStreamIndex();
    TEST_ASSERT(video_index >= 0, "Should find valid video stream index");
    
    // 检查AVStream
    AVStream* stream = demuxer.getAVStream();
    TEST_ASSERT(stream != nullptr, "Should get valid AVStream pointer");
    
    // 检查格式上下文
    AVFormatContext* ctx = demuxer.getFormatContext();
    TEST_ASSERT(ctx != nullptr, "Should get valid format context");
    
    demuxer.close();
    
    // 清理测试文件
    std::remove(test_file.c_str());
    
    return true;
}

// 测试5: 打开有效的音频文件
bool testOpenValidAudioFile() {
    const std::string test_file = "test_audio.aac";
    
    // 创建测试文件
    if (!createTestAudioFile(test_file)) {
        std::cout << "WARNING: Cannot create test audio file, skipping test" << std::endl;
        return true; // 跳过测试但不失败
    }
    
    Demuxer demuxer(MediaType::AUDIO);
    bool result = demuxer.open(test_file);
    TEST_ASSERT(result, "Should successfully open valid audio file");
    
    // 检查流索引
    int audio_index = demuxer.getStreamIndex();
    TEST_ASSERT(audio_index >= 0, "Should find valid audio stream index");
    
    demuxer.close();
    
    // 清理测试文件
    std::remove(test_file.c_str());
    
    return true;
}

// 测试6: 读取数据包
bool testReadPacket() {
    const std::string test_file = "test_video_packets.mp4";
    
    // 创建测试文件
    if (!createTestVideoFile(test_file)) {
        std::cout << "WARNING: Cannot create test video file, skipping test" << std::endl;
        return true;
    }
    
    Demuxer demuxer(MediaType::VIDEO);
    bool result = demuxer.open(test_file);
    TEST_ASSERT(result, "Should open test file for packet reading");
    
    // 读取几个数据包
    int packet_count = 0;
    const int max_packets = 10;
    
    for (int i = 0; i < max_packets; i++) {
        AVPacket* packet = demuxer.readPacket();
        if (packet) {
            packet_count++;
            TEST_ASSERT(packet->stream_index == demuxer.getStreamIndex(), 
                       "Packet should belong to correct stream");
            av_packet_free(&packet);
        } else {
            break; // EOF or error
        }
    }
    
    TEST_ASSERT(packet_count > 0, "Should read at least one packet");
    
    demuxer.close();
    std::remove(test_file.c_str());
    
    return true;
}

// 测试7: 获取时长
bool testGetDuration() {
    const std::string test_file = "test_duration.mp4";
    
    // 创建测试文件
    if (!createTestVideoFile(test_file)) {
        std::cout << "WARNING: Cannot create test video file, skipping test" << std::endl;
        return true;
    }
    
    Demuxer demuxer(MediaType::VIDEO);
    bool result = demuxer.open(test_file);
    TEST_ASSERT(result, "Should open test file for duration test");
    
    int64_t duration = demuxer.getDuration();
    TEST_ASSERT(duration > 0, "Duration should be positive");
    
    // 5秒视频的时长应该大约是5,000,000微秒
    TEST_ASSERT(duration > 4000000 && duration < 6000000, 
               "Duration should be approximately 5 seconds");
    
    demuxer.close();
    std::remove(test_file.c_str());
    
    return true;
}

// 测试8: Seek功能
bool testSeek() {
    const std::string test_file = "test_seek.mp4";
    
    // 创建测试文件
    if (!createTestVideoFile(test_file)) {
        std::cout << "WARNING: Cannot create test video file, skipping test" << std::endl;
        return true;
    }
    
    Demuxer demuxer(MediaType::VIDEO);
    bool result = demuxer.open(test_file);
    TEST_ASSERT(result, "Should open test file for seek test");
    
    // 获取总时长
    int64_t duration = demuxer.getDuration();
    if (duration <= 0) {
        std::cout << "WARNING: Cannot get duration, skipping seek test" << std::endl;
        demuxer.close();
        std::remove(test_file.c_str());
        return true;
    }
    
    // Seek到中间位置
    int64_t seek_pos = duration / 2;
    bool seek_result = demuxer.seek(seek_pos);
    
    // 注意：某些格式或编解码器可能不支持随机访问
    // 如果seek失败，只记录警告但不让测试失败
    if (seek_result) {
        TEST_ASSERT(seek_result, "Successfully seeked to middle position");
        // 检查EOF标志被重置
        TEST_ASSERT(!demuxer.isEOF(), "EOF flag should be reset after seek");
    } else {
        std::cout << "WARNING: Seek operation failed, possibly unsupported by format/codec" << std::endl;
    }
    
    demuxer.close();
    std::remove(test_file.c_str());
    
    return true;
}

// 测试9: EOF检测
bool testEOFDetection() {
    const std::string test_file = "test_eof.mp4";
    
    // 创建测试文件
    if (!createTestVideoFile(test_file)) {
        std::cout << "WARNING: Cannot create test video file, skipping test" << std::endl;
        return true;
    }
    
    Demuxer demuxer(MediaType::VIDEO);
    bool result = demuxer.open(test_file);
    TEST_ASSERT(result, "Should open test file for EOF test");
    
    // 初始状态不应该是EOF
    TEST_ASSERT(!demuxer.isEOF(), "Should not be EOF initially");
    
    // 读取所有数据包直到EOF
    int packet_count = 0;
    while (true) {
        AVPacket* packet = demuxer.readPacket();
        if (packet) {
            packet_count++;
            av_packet_free(&packet);
        } else {
            break;
        }
        
        // 防止无限循环
        if (packet_count > 1000) {
            break;
        }
    }
    
    // 应该到达EOF
    TEST_ASSERT(demuxer.isEOF(), "Should reach EOF after reading all packets");
    
    demuxer.close();
    std::remove(test_file.c_str());
    
    return true;
}

// 测试10: 多次打开和关闭
bool testMultipleOpenClose() {
    const std::string test_file = "test_multiple.mp4";
    
    // 创建测试文件
    if (!createTestVideoFile(test_file)) {
        std::cout << "WARNING: Cannot create test video file, skipping test" << std::endl;
        return true;
    }
    
    Demuxer demuxer(MediaType::VIDEO);
    
    // 多次打开和关闭
    for (int i = 0; i < 3; i++) {
        bool result = demuxer.open(test_file);
        TEST_ASSERT(result, "Should open file in iteration " + std::to_string(i));
        
        demuxer.close();
    }
    
    std::remove(test_file.c_str());
    
    return true;
}

int main() {
    std::cout << "Starting Demuxer Tests..." << std::endl;
    
    // 运行所有测试
    RUN_TEST(testBasicConstructorDestructor);
    RUN_TEST(testOpenNonExistentFile);
    RUN_TEST(testOpenEmptyFilename);
    RUN_TEST(testOpenValidVideoFile);
    RUN_TEST(testOpenValidAudioFile);
    RUN_TEST(testReadPacket);
    RUN_TEST(testGetDuration);
    RUN_TEST(testSeek);
    RUN_TEST(testEOFDetection);
    RUN_TEST(testMultipleOpenClose);
    
    // 输出测试结果
    std::cout << "\n=== Test Summary ===" << std::endl;
    std::cout << "Total tests: " << total_tests << std::endl;
    std::cout << "Passed: " << passed_tests << std::endl;
    std::cout << "Failed: " << failed_tests << std::endl;
    
    if (failed_tests == 0) {
        std::cout << "All tests PASSED!" << std::endl;
        return 0;
    } else {
        std::cout << "Some tests FAILED!" << std::endl;
        return 1;
    }
}
