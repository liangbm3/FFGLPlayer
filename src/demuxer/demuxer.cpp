#include "demuxer.hpp"

#include "utils/logger.hpp"

// 构造函数，根据多媒体类型来进行初始化
Demuxer::Demuxer(MediaType type)
    : format_ctx_(nullptr), type_(type), video_stream_(nullptr), audio_stream_(nullptr),
      video_stream_index_(-1), audio_stream_index_(-1), eof_file_(false)
{
    LOG_INFO << "Demuxer initialized for type: " << (type == MediaType::VIDEO ? "VIDEO" : "AUDIO");
}

// 析构函数
Demuxer::~Demuxer()
{
    LOG_INFO << "Demuxer destructor called.";
    close(); // 确保在析构时关闭demuxer
}

// 初始化和打开文件
bool Demuxer::open(const std::string &filename)
{
    LOG_INFO << "Opening demuxer for file: " << filename;

    // 检查文件名是否为空
    if (filename.empty())
    {
        LOG_ERROR << "Filename is empty.";
        return false;
    }

    // 确保之前的资源已经释放
    if (format_ctx_)
    {
        LOG_WARN << "Demuxer already initialized. Closing previous context.";
        close();
    }

    // 打开媒体文件
    // 它会探测文件格式并填充format_ctx_，执行完后fomat_ctx_会包含媒体文件的基本信息
    // 参数：AVFormatContext* 的指针地址，媒体文件路径，指定的输入格式（nullptr表示自动探测），以及可选的字典参数（通常为nullptr）
    if (avformat_open_input(&format_ctx_, filename.c_str(), nullptr, nullptr) < 0)
    {
        LOG_ERROR << "Failed to open media file: " << filename;
        return false; // 打开失败，返回false
    }

    // 查找流信息
    // 读取文件开头的部分数据，解析并填充到streams数组中
    // 通常包括编解码参数、时长、码率等详细信息
    // 第二个参数是用来传递额外选项的
    if (avformat_find_stream_info(format_ctx_, nullptr) < 0)
    {
        LOG_ERROR << "Could not find stream information in file: " << filename;
        close();      // 关闭demuxer
        return false; // 查找流信息失败，返回false
    }

    // 查找最佳视频流
    // 第三和第四个参数通常用于指定首选流和相关流，这里不指定，第五个参数用来指定编解码器，nullptr表示不指定，最后一个为查找选项
    video_stream_index_ = av_find_best_stream(format_ctx_, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    // 如果找到视频流
    if (video_stream_index_ >= 0)
    {
        video_stream_ = format_ctx_->streams[video_stream_index_]; // 获取视频流
        LOG_INFO << "Video stream found at index: " << video_stream_index_;
    }
    else
    {
        LOG_WARN << "No video stream found in file: " << filename;
    }

    // 查找最佳音频流
    audio_stream_index_ = av_find_best_stream(format_ctx_, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
    // 如果找到音频流
    if (audio_stream_index_ >= 0)
    {
        audio_stream_ = format_ctx_->streams[audio_stream_index_]; // 获取音频流
        LOG_INFO << "Audio stream found at index: " << audio_stream_index_;
    }
    else
    {
        LOG_WARN << "No audio stream found in file: " << filename;
    }

    // 查找成功返回true
    return true;
}

// 从媒体文件中循环读取数据包，过滤出目标流的包并返回
AVPacket *Demuxer::readPacket()
{
    // 确保上下文初始化
    if (!format_ctx_)
    {
        LOG_ERROR << "Demuxer not initialized.";
        return nullptr; // 如果没有初始化，返回nullptr
    }
    // 获取目标流的索引
    int target_stream_index = getStreamIndex();
    if (target_stream_index < 0)
    {
        LOG_ERROR << "No valid stream index found.";
        return nullptr; // 如果没有有效的流索引，返回nullptr
    }

    // 循环读取包
    while (true)
    {
        AVPacket *packet = av_packet_alloc();         // 分配一个新的AVPacket
        int ret = av_read_frame(format_ctx_, packet); // 从媒体文件中读取数据包
        // 错误处理
        if (ret < 0)
        {
            // 如果是文件结束，设置eof标志
            if (ret == AVERROR_EOF)
            {
                eof_file_ = true;
                LOG_INFO << "End of file reached.";
            }
            else // 否则记录日志
            {
                char errbuf[AV_ERROR_MAX_STRING_SIZE];
                av_strerror(ret, errbuf, sizeof(errbuf));
                LOG_ERROR << "Error reading frame: " << errbuf;
            }
            av_packet_free(&packet); // 释放AVPacket
            return nullptr;          // 返回nullptr表示读取失败或到达文件末尾
        }
        if (packet->stream_index == target_stream_index) // 如果包属于目标流
        {
            return packet; // 返回读取到的包
        }
        else // 如果包不属于目标流，释放包并继续读取下一个包
        {
            av_packet_free(&packet);
        }
    }
}

//跳转到目标时间戳
//flags：定位方式
//成功返回true，失败返回false
bool Demuxer::seek(int64_t timestamp,int flags)
{
    // 确保上下文初始化
    if (!format_ctx_)
    {
        LOG_ERROR << "Demuxer not initialized.";
        return false;
    }
    // 获取目标流的索引
    int stream_index = getStreamIndex();
    if (stream_index < 0)
    {
        LOG_ERROR << "No valid stream index found.";
        return false;
    }
    //目标流
    AVStream* stream = format_ctx_->streams[stream_index];
    //验证目标流的有效性
    if(!stream)
    {
        LOG_ERROR << "Stream is null.";
        return false;
    }   
    //从微秒转换到流的时间基
    int64_t seek_target = av_rescale_q(timestamp,AV_TIME_BASE_Q,stream->time_base);
    
    LOG_INFO << "Seeking to " << timestamp << "us (stream timebase: "
              << stream->time_base.num << "/" << stream->time_base.den
              << ", target: " << seek_target << ")";
    //在指定的媒体流上，跳转到目标时间戳，并根据flags参数控制定位方式
    int ret = av_seek_frame(format_ctx_, stream_index, seek_target, flags);
    //失败处理
    if(ret<0)
    {
        char errbuf[AV_ERROR_MAX_STRING_SIZE];
        av_strerror(ret, errbuf, sizeof(errbuf));
        LOG_ERROR << "Error seeking to " << timestamp << "us: " << errbuf;
        return false;
    }
    //定位成功重置eof标志
    eof_file_ = false;
    LOG_INFO<< "Seeked to " << timestamp << "us successfully.";
    return true;
}

// 获取媒体文件的总时长
int64_t Demuxer::getDuration() const
{
    // 确保format_ctx_不为空
    if (!format_ctx_)
    {
        LOG_ERROR << "Demuxer not initialized.";
        return 0;
    }
    // 优先考虑format_ctx_中的duration字段
    if (format_ctx_->duration != AV_NOPTS_VALUE)
    {
        return format_ctx_->duration; // 返回以微秒为单位的时长
    }
    
    // 根据当前媒体类型，尝试从对应的流中获取时长
    AVStream* target_stream = getAVStream();
    if (target_stream && target_stream->duration != AV_NOPTS_VALUE)
    {
        //从流的时间基转换到标准微秒时间基
        //参数：流时长、流时间基、ffmpeg定义的标准时间基
        return av_rescale_q(target_stream->duration, target_stream->time_base, AV_TIME_BASE_Q);
    }
    
    // 如果目标流不可用，尝试从视频流获取
    if (video_stream_ && video_stream_->duration != AV_NOPTS_VALUE)
    {
        return av_rescale_q(video_stream_->duration, video_stream_->time_base, AV_TIME_BASE_Q);
    }
    
    // 如果视频流也不可用，尝试从音频流获取
    if (audio_stream_ && audio_stream_->duration != AV_NOPTS_VALUE)
    {
        return av_rescale_q(audio_stream_->duration, audio_stream_->time_base, AV_TIME_BASE_Q);
    }
    
    // 如果都不可用，返回0
    return 0;
}

// 关闭函数
void Demuxer::close()
{
    LOG_INFO << "Closing Demuxer...";
    // 如果format_ctx_不为空，释放它
    if (format_ctx_)
    {
        // 释放format_ctx_，它会自动释放streams数组和其他相关资源
        avformat_close_input(&format_ctx_);
        format_ctx_ = nullptr; // 置空
        LOG_INFO << "Format context closed.";
    }
    // 重置所有成员变量
    video_stream_ = nullptr;
    audio_stream_ = nullptr;
    video_stream_index_ = -1;
    audio_stream_index_ = -1;
    eof_file_ = false;
    LOG_INFO << "Demuxer closed successfully.";
}