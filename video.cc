#include "video.h"
#include <assert.h>
#include <iostream>

using namespace std;

// Constructor opens video file, selects appropriate codecs and prepares
// to decode frames.
VideoDecoder::VideoDecoder(const char* path)
{
    int result = 0;

    result = avformat_open_input(&formatContext, path, nullptr, nullptr);
    if (result < 0) {
        cerr << "Failed to open video file '" << path << "'\n";
        goto exit;
    }

    // Find best video stream.
    result = avformat_find_stream_info(formatContext, nullptr);
    if (result < 0)
        goto exit;
    AVCodec* decoder;
    result = av_find_best_stream(formatContext, AVMEDIA_TYPE_VIDEO, -1, -1, &decoder, 0);
    if (result < 0) {
        cerr << "Failed to find suitable video stream in video file '" << path
             << "'. Are the correct codecs installed?\n";
        goto exit;
    }
    videoStreamIndex = result;

    // Create decoder context.
    decoderContext = avcodec_alloc_context3(decoder);
    if (decoderContext == nullptr) {
        result = ENOMEM;
        goto exit;
    }
    result = avcodec_parameters_to_context(
        decoderContext,
        formatContext->streams[videoStreamIndex]->codecpar);
    if (result < 0)
        goto exit;
    result = avcodec_open2(decoderContext, decoder, nullptr);
    if (result < 0)
        goto exit;

    // TODO Support videos encoded with formats other than YUV420P. This can be done by calling
    //      sws_scale to convert format of an AVFrame*. See README for more.
    assert(decoderContext->pix_fmt == AV_PIX_FMT_YUV420P);

exit:
    if (result != 0) {
        cleanup();
        throw VideoDecoderException(result);
    }
}

VideoDecoder::~VideoDecoder()
{
    cleanup();
}

void VideoDecoder::cleanup()
{
    if (decoderContext != nullptr)
        avcodec_free_context(&decoderContext);
    if (formatContext != nullptr)
        avformat_close_input(&formatContext);
}

// Read packets until we find one from desired stream.
int VideoDecoder::decodeNextPacket()
{
    for (;;) {

        AVPacket packet;
        int result = av_read_frame(formatContext, &packet);
        if (result < 0)
            return result; // Either unexpected error or end of stream (EOF).

        if (packet.stream_index != videoStreamIndex) {
            av_packet_unref(&packet);
            continue;
        }

        result = avcodec_send_packet(decoderContext, &packet);
        av_packet_unref(&packet);
        return result; // Either an unexpected error or a success.

    }
}

optional<VideoDecoder::Frame> VideoDecoder::getNextKeyFrame()
{
    while (auto frame = getNextFrame()) {
        if (frame->isKeyFrame())
            return frame;
    }
    return nullopt;
}

optional<VideoDecoder::Frame> VideoDecoder::getNextFrame()
{
    AVFrame* frame = av_frame_alloc();
    if (frame == nullptr) {
        throw VideoDecoderException(ENOMEM);
    }

    int result = 0;
    for (;;) {

        // First, try to decode a frame from already seen packets.
        result = avcodec_receive_frame(decoderContext, frame);
        if (result >= 0) {
            // Done. Management of AVFrame* passes to Frame object.
            return make_optional<Frame>(frame, formatContext->streams[videoStreamIndex]->time_base);
        } else if (result != AVERROR(EAGAIN) && result != AVERROR_EOF) {
            break; // Unexpected error.
        }

        // We either need to send more packets or have an EOF.
        result = decodeNextPacket();
        if (result < 0) {
            break; // Either EOF or unexpected error.
        }

    }

    // Either reached end of stream or hit an unexpected error.
    assert(result != 0);
    av_frame_free(&frame);
    if (result == AVERROR_EOF)
        return nullopt;
    throw VideoDecoderException(result);
}

VideoDecoder::Frame::Frame(AVFrame* f, const AVRational& timeBase) :
    frame(f), // Takes ownership of reference on f.
    timestampToSeconds(static_cast<float>(timeBase.num) / static_cast<float>(timeBase.den))
{
    assert(frame != nullptr);
}

VideoDecoder::Frame::Frame(Frame&& other) :
    frame(other.frame),
    timestampToSeconds(other.timestampToSeconds)
{
    other.frame = nullptr;
}

VideoDecoder::Frame::~Frame()
{
    if (frame != nullptr) {
        av_frame_unref(frame);
        av_frame_free(&frame);
    }
}

// Get frame timestamp in seconds.
float VideoDecoder::Frame::getTime() const noexcept
{
    return static_cast<float>(frame->best_effort_timestamp) * timestampToSeconds;
}

// Access grayscale pixel for coordinates y, x. Assumes YUV420P format (which means
// we just grab the Y value.
uint8_t VideoDecoder::Frame::operator()(int x, int y) const
{
    assert(frame->format == AV_PIX_FMT_YUV420P);
    return *(frame->data[0] + frame->linesize[0] * y + x);
}

// Print frame to file (for debugging purposes).
void VideoDecoder::Frame::print(const char* name)
{
    FILE* f = fopen(name, "w");
    assert(f != nullptr);

    fprintf(f, "P5\n%d %d\n%d\n", getWidth(), getHeight(), 255);

    for (int y = 0; y < getHeight(); y++) {
        for (int x = 0; x < getWidth(); x++) {
            uint8_t nextByte = (*this)(x, y);
            fwrite(&nextByte, 1, 1, f);
        }
    }

    fclose(f);
}