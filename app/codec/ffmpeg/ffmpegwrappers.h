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

#ifndef FFMPEGWRAPPERS_H
#define FFMPEGWRAPPERS_H

extern "C" {
#include <libavcodec/packet.h>
#include <libavformat/avformat.h>
#include <libavutil/channel_layout.h>
#include <libavutil/frame.h>
#include <libavcodec/packet.h>
#include <libavutil/frame.h>
#include <libavformat/avformat.h>
#include <libavutil/channel_layout.h>
#include <libavutil/frame.h>
}

#include <codec/ffmpeg/ffmpegframepool.h>

#include <memory>

namespace olive {
struct AvFramePtrDeleter {
  void operator()(AVFrame *f) noexcept { av_frame_free(&f); }
};

struct AvPacketPtrDeleter {
  void operator()(AVPacket *f) noexcept { av_packet_free(&f); }
};

using AVFramePtr = std::unique_ptr<AVFrame, AvFramePtrDeleter>;
using AVPacketPtr = std::unique_ptr<AVPacket, AvPacketPtrDeleter>;

constexpr VideoParams::Format GetNativePixelFormat(AVPixelFormat pix_fmt) noexcept {
  switch (pix_fmt) {
    case AV_PIX_FMT_RGB24:
    case AV_PIX_FMT_RGBA:
      return VideoParams::kFormatUnsigned8;
    case AV_PIX_FMT_RGB48:
    case AV_PIX_FMT_RGBA64:
      return VideoParams::kFormatUnsigned16;
    default:
      return VideoParams::kFormatInvalid;
  }
}

constexpr int GetNativeChannelCount(AVPixelFormat pix_fmt) noexcept {
  switch (pix_fmt) {
    case AV_PIX_FMT_RGB24:
    case AV_PIX_FMT_RGB48:
      return VideoParams::kRGBChannelCount;
    case AV_PIX_FMT_RGBA:
    case AV_PIX_FMT_RGBA64:
      return VideoParams::kRGBAChannelCount;
    default:
      return 0;
  }
}

inline std::int64_t ValidateChannelLayout(const AVStream *stream) noexcept {
  if (stream->codecpar->channel_layout) {
    return stream->codecpar->channel_layout;
  }

  return av_get_default_channel_layout(stream->codecpar->channels);
}

constexpr const char *GetInterlacingModeInFFmpeg(VideoParams::Interlacing interlacing) noexcept {
  if (interlacing == VideoParams::kInterlacedTopFirst) {
    return "tff";
  }

  return "bff";
}
}  // namespace olive

#endif  // FFMPEGWRAPPERS_H