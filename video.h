#pragma once

#include <optional>
#include <exception>
#include <errno.h>
#include <string.h>
#include <assert.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

// Encapsulates errors from libav* functions.
class VideoDecoderException : public std::exception
{
public:
    VideoDecoderException(int e) noexcept : err(e)
    {
        if (err < 0) {
            // If AVERROR, populate error string ahead of time.
            int result = av_strerror(err, errString, sizeof(errString));
            assert(result >= 0); // Increase ErrStringSize.
        }
    }

    const char* what() const noexcept override
    {
        // TODO Separate exception types for AVERRORs and errnos.
        if (err < 0) {
            // Interpret as AVERROR.
            return errString;
        } else {
            // Interpret as errno.
            return strerror(err);
        }
    }

    int code() const noexcept { return err; }

private:
    static constexpr size_t ErrStringSize = 128;
    int err;
    char errString[ErrStringSize];
};

// Decodes a video file for the purpose of iterating through keyframes.
class VideoDecoder
{
public:
    VideoDecoder(const char* path);
    ~VideoDecoder();

    VideoDecoder(const VideoDecoder&) = delete;
    VideoDecoder& operator=(const VideoDecoder&) = delete;

    // Simple RAII class to manage AVFrame* and provide basic functions around it.
    class Frame
    {
    public:
        Frame(AVFrame* f, const AVRational& timeBase);
        Frame(Frame&& other);
        ~Frame();
        Frame(const Frame&) = delete;
        Frame& operator=(const Frame&) = delete;

        int getWidth() const noexcept { return frame->width; }
        int getHeight() const noexcept { return frame->height; }
        bool isKeyFrame() const noexcept { return frame->key_frame; }
        float getTime() const noexcept;
        uint8_t operator()(int x, int y) const;

        void print(const char* name);

    private:
        AVFrame* frame;
        float timestampToSeconds; // Convert frame timestamp to seconds from beginning.
    };

    std::optional<Frame> getNextFrame();
    std::optional<Frame> getNextKeyFrame();

private:
    void cleanup();
    int decodeNextPacket();

    AVFormatContext* formatContext = nullptr;
    AVCodecContext* decoderContext = nullptr;
    int videoStreamIndex = -1;
};