#include "widget.h"
#include "./ui_widget.h"

#include <QDebug>



int show_video_info()
{
    /*
    这里介绍怎么使用ffmpeg打开文件，读取文件信息
    */
    const char* filename = "E:\\Video\\test.mkv";

    // 格式上下文
    AVFormatContext* fmt_ctx = nullptr;
    // 打开文件
    if (avformat_open_input(&fmt_ctx, filename, NULL, NULL) < 0) {
        qDebug() << "无法打开文件" ;
        return -1;
    }

    // 读取流信息
    if (avformat_find_stream_info(fmt_ctx, NULL) < 0) {
        qDebug() << "无法获取流信息" ;
        return -1;
    }
    // 打印视频文件信息
    av_dump_format(fmt_ctx, 0, filename, 0);

    // 关闭上下文句柄
    avformat_close_input(&fmt_ctx);
    return 0;
}

int decoder_01()
{
    /*
    这里介绍打开解码器，获取视频信息
    */
    const char* filename = "E:\\Video\\test.mkv";


    // 格式上下文
    AVFormatContext* fmt_ctx = nullptr;
    // 打开文件
    if (avformat_open_input(&fmt_ctx, filename, NULL, NULL) < 0) {
        qDebug() << "无法打开文件" ;
        return -1;
    }

    // 查找视频流
    int video_stream_index = -1;
    for (int i = 0; i < fmt_ctx->nb_streams; i++) {
        if (fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_index = i;
            break;
        }
    }
    if (video_stream_index == -1) {
        qDebug() << "没有找到视频流" ;
        return -1;
    }

    // 查找解码器
    AVCodecParameters* codecpar = fmt_ctx->streams[video_stream_index]->codecpar;
    const AVCodec* codec = avcodec_find_decoder(codecpar->codec_id);
    if (!codec) {
        qDebug() << "找不到解码器" ;
        return -1;
    }

    // 打开解码器
    AVCodecContext* codec_ctx = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(codec_ctx, codecpar);
    if (avcodec_open2(codec_ctx, codec, NULL) < 0) {
        qDebug() << "打开解码器失败" ;
        return -1;
    }

    qDebug() << "解码器准备完成，分辨率: "
              << codec_ctx->width << "x"
              << codec_ctx->height;

    /// 读取 AVPacket 并解码成 AVFrame
    AVPacket pkt;
    AVFrame* frame = av_frame_alloc();

    // 用 av_read_frame 读取压缩数据包（Packet）
    while (av_read_frame(fmt_ctx, &pkt) >= 0) {
        if (pkt.stream_index == video_stream_index) {
            // 发送压缩数据 用 avcodec_send_packet 发送给解码器
            if (avcodec_send_packet(codec_ctx, &pkt) == 0) {
                // 接收解码后的帧 用 avcodec_receive_frame 接收解码后的原始帧（AVFrame）
                while (avcodec_receive_frame(codec_ctx, frame) == 0) {
                    qDebug() << "解码一帧: "
                             << frame->width << "x"
                             << frame->height;
                    // 创建转换上下文
                    struct SwsContext* sws_ctx = sws_getContext(
                        codec_ctx->width, codec_ctx->height, codec_ctx->pix_fmt,
                        codec_ctx->width, codec_ctx->height, AV_PIX_FMT_RGB24,
                        SWS_BILINEAR, NULL, NULL, NULL
                        );

                    // 分配 RGB buffer
                    AVFrame* rgb_frame = av_frame_alloc();
                    int numBytes = av_image_alloc(
                        rgb_frame->data,
                        rgb_frame->linesize,
                        codec_ctx->width,
                        codec_ctx->height,
                        AV_PIX_FMT_RGB24,
                        1 // 对齐 1 字节即可
                        );

                    if (numBytes < 0) {
                        qDebug() << "无法分配 RGB buffer";
                        continue;
                    }

                    // 转换
                    sws_scale(
                        sws_ctx,
                        frame->data, frame->linesize,
                        0, codec_ctx->height,
                        rgb_frame->data, rgb_frame->linesize
                        );

                    // 显示
                    QImage img(rgb_frame->data[0], codec_ctx->width, codec_ctx->height,
                               rgb_frame->linesize[0], QImage::Format_RGB888);
                    // 注意这里如果不进行任何处理，那么可以直接显示img
                    // 如果需要对图片进行处理，那么需要 img.copy() 拷贝图片出来
                    // 因为目前的图片数据是ffmpeg管理的
                    // label->setPixmap(QPixmap::fromImage(img));

                    // 释放资源
                    av_freep(&rgb_frame->data[0]);
                    av_frame_free(&rgb_frame);
                }
            }
        }
        av_packet_unref(&pkt);
    }

    av_frame_free(&frame);
    return 0;
}

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
}

Widget::~Widget()
{
    delete ui;
}

void Widget::on_pushButton_clicked()
{
    // show_video_info();
    // decoder_01();


    const char* filename = "E:\\Video\\test.mkv";

    // 打开文件
    if (avformat_open_input(&fmt_ctx, filename, NULL, NULL) < 0) {
        qDebug() << "无法打开文件" ;
        return;
    }

    // 查找视频流
    for (int i = 0; i < fmt_ctx->nb_streams; i++) {
        if (fmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            video_stream_index = i;
            break;
        }
    }
    if (video_stream_index == -1) {
        qDebug() << "没有找到视频流" ;
        return;
    }

    // 查找解码器
    AVCodecParameters* codecpar = fmt_ctx->streams[video_stream_index]->codecpar;
    const AVCodec* codec = avcodec_find_decoder(codecpar->codec_id);
    if (!codec) {
        qDebug() << "找不到解码器" ;
        return;
    }

    // 打开解码器
    codec_ctx = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(codec_ctx, codecpar);
    if (avcodec_open2(codec_ctx, codec, NULL) < 0) {
        qDebug() << "打开解码器失败" ;
        return;
    }

    qDebug() << "解码器准备完成，分辨率: "
             << codec_ctx->width << "x"
             << codec_ctx->height;

    /// 读取 AVPacket 并解码成 AVFrame
    /// 为了不阻塞 UI，最简单方法是用 QTimer 驱动解码单帧

    // 初始化
    frame = av_frame_alloc();


    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this]() {
        AVPacket pkt;
        if (av_read_frame(fmt_ctx, &pkt) >= 0 && pkt.stream_index == video_stream_index) {
            avcodec_send_packet(codec_ctx, &pkt);
            if (avcodec_receive_frame(codec_ctx, frame) == 0) {
                rgb_frame = av_frame_alloc();
                int numBytes = av_image_alloc(rgb_frame->data, rgb_frame->linesize,
                                              codec_ctx->width, codec_ctx->height,
                                              AV_PIX_FMT_RGB24, 1);

                sws_ctx = sws_getContext(
                    codec_ctx->width, codec_ctx->height, codec_ctx->pix_fmt,
                    codec_ctx->width, codec_ctx->height, AV_PIX_FMT_RGB24,
                    SWS_BILINEAR, nullptr, nullptr, nullptr
                    );

                sws_scale(sws_ctx, frame->data, frame->linesize,
                          0, codec_ctx->height,
                          rgb_frame->data, rgb_frame->linesize);

                QImage img(rgb_frame->data[0], codec_ctx->width, codec_ctx->height,
                           rgb_frame->linesize[0], QImage::Format_RGB888);
                ui->label->setPixmap(QPixmap::fromImage(img));

                av_freep(&rgb_frame->data[0]);
                av_frame_free(&rgb_frame);
            }
            av_packet_unref(&pkt);
        }
    });
    timer->start(30); // 大约 30fps

    return;
}

