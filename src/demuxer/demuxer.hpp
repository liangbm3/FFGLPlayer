#pragma once

extern "C"
{
//多媒体封装和解封装API
#include <libavformat/avformat.h>
}

#include <string>
#include "mediadefs.hpp"//多媒体类型的定义

class Demuxer
{
public:
    Demuxer(MediaType type);
    ~Demuxer();

    bool open(const std::string &filename);
    void close();

    AVPacket* readPacket();

    //timestamp为微秒
    bool seek(int64_t timestamp,int flags = 0);

    int64_t getDuration() const;

    //返回当前关注流的索引号
    int getStreamIndex() const
    {
        return (type_ == MediaType::VIDEO) ? video_stream_index_ : audio_stream_index_;
    }
    //返回当前关注流的AVStream指针
    AVStream* getAVStream() const
    {
        return (type_ == MediaType::VIDEO) ? video_stream_ : audio_stream_;
    }
    AVFormatContext* getFormatContext() const { return format_ctx_; }
    
    // 检查是否到达文件末尾
    bool isEOF() const { return eof_file_; }
private:
    MediaType type_; // 媒体类型
    AVFormatContext *format_ctx_; // FFmpeg格式上下文，代表媒体文件
    AVStream* video_stream_; // 视频流
    AVStream* audio_stream_; // 音频流
    int video_stream_index_; // 视频流索引
    int audio_stream_index_; // 音频流索引
    bool eof_file_; // 是否到达文件末尾
};

