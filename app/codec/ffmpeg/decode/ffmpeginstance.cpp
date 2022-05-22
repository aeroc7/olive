/***

  Olive - Non-Linear Video Editor
  Copyright (C) 2022 Olive Team

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.

***/

#include "ffmpeginstance.h"

#include "../ffmpegwrappers.h"

extern "C" {
#include <libavcodec/avcodec.h>
}

namespace olive {
bool DecoderInstance::Open(const char *filename, int stream_index)
{
  // Open file in a format context
  int error_code = avformat_open_input(&fmt_ctx_, filename, nullptr, nullptr);

  // Handle format context error
  if (error_code != 0) {
    qCritical() << "Failed to open input: " << filename << FFmpegErrorCodeToStr(error_code);
    return false;
  }

  // Get stream information from format
  error_code = avformat_find_stream_info(fmt_ctx_, nullptr);

  // Handle get stream information error
  if (error_code < 0) {
    qCritical() << "Failed to find stream info: " << FFmpegErrorCodeToStr(error_code);
    return false;
  }

  // Get reference to correct AVStream
  avstream_ = fmt_ctx_->streams[stream_index];

  // Find decoder
  const AVCodec* codec = avcodec_find_decoder(avstream_->codecpar->codec_id);

  // Handle failure to find decoder
  if (codec == nullptr) {
    qCritical() << "Failed to find appropriate decoder for this codec: " << filename << stream_index
                << avstream_->codecpar->codec_id;
    return false;
  }

  // Allocate context for the decoder
  codec_ctx_ = avcodec_alloc_context3(codec);
  if (codec_ctx_ == nullptr) {
    qCritical() << "Failed to allocate codec context";
    return false;
  }

  // Copy parameters from the AVStream to the AVCodecContext
  error_code = avcodec_parameters_to_context(codec_ctx_, avstream_->codecpar);

  // Handle failure to copy parameters
  if (error_code < 0) {
    qCritical() << "Failed to copy parameters from AVStream to AVCodecContext";
    return false;
  }

  // Set multithreading setting
  error_code = av_dict_set(&opts_, "threads", "auto", 0);

  // Handle failure to set multithreaded decoding
  if (error_code < 0) {
    qCritical() << "Failed to set codec options, performance may suffer";
  }

  // Open codec
  error_code = avcodec_open2(codec_ctx_, codec, &opts_);
  if (error_code < 0) {
    qCritical() << "Failed to open codec " << codec->id << ": " << FFmpegErrorCodeToStr(error_code);
    return false;
  }

  return true;
}

int DecoderInstance::GetPacket(AVPacket* pkt) noexcept
{
  int ret{};

  do {
    av_packet_unref(pkt);

    ret = av_read_frame(fmt_ctx_, pkt);
  } while (pkt->stream_index != avstream_->index && ret >= 0);

  return ret;
}

int DecoderInstance::GetSubtitle(AVPacket* pkt, AVSubtitle* sub) noexcept
{
  int ret = GetPacket(pkt);

  if (ret >= 0) {
    int got_sub;
    ret = avcodec_decode_subtitle2(codec_ctx_, sub, &got_sub, pkt);
    if (!got_sub) {
      ret = -1;
    }
  }

  return ret;
}

std::string_view DecoderInstance::GetSubtitleHeader() const noexcept
{
  return reinterpret_cast<const char*>(codec_ctx_->subtitle_header);
}

int DecoderInstance::GetFrame(AVPacket* pkt, AVFrame* frame) noexcept
{
  bool eof = false;
  int ret{};

  // Clear any previous frames
  av_frame_unref(frame);

  while (true) {
    ret = avcodec_receive_frame(codec_ctx_, frame);
    if (ret != AVERROR(EAGAIN) || eof == true) {
      break;
    }

    // Find next packet in the correct stream index
    ret = GetPacket(pkt);

    if (ret == AVERROR_EOF) {
      // Don't break so that receive gets called again, but don't try to read again
      eof = true;

      // Send a null packet to signal end of
      avcodec_send_packet(codec_ctx_, nullptr);
      continue;
    } else if (ret < 0) {
      // Handle other error by breaking loop and returning the code we received
      break;
    }

    // Successful read, send the packet
    ret = avcodec_send_packet(codec_ctx_, pkt);

    // We don't need the packet anymore, so free it
    av_packet_unref(pkt);

    if (ret < 0) {
      break;
    }
  }

  return ret;
}

void DecoderInstance::Seek(std::int64_t timestamp) noexcept
{
  avcodec_flush_buffers(codec_ctx_);
  av_seek_frame(fmt_ctx_, avstream_->index, timestamp, AVSEEK_FLAG_BACKWARD);
}

void DecoderInstance::Close() noexcept
{
  av_dict_free(&opts_);
  avcodec_free_context(&codec_ctx_);
  avformat_close_input(&fmt_ctx_);
}

DecoderInstance::~DecoderInstance() { Close(); }
}  // namespace olive