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

#include "ffmpegdecoder.h"

extern "C" {
#include <libavformat/avformat.h>
}

namespace olive {
bool FFmpegDecoder::OpenInternal()
{
  return true;
}

FramePtr FFmpegDecoder::RetrieveVideoInternal(const rational &timecode, const RetrieveVideoParams &params, const QAtomicInt *cancelled)
{
  return nullptr;
}

void FFmpegDecoder::CloseInternal()
{
}

// Generate footage info from file, must be re-entrant
FootageDescription FFmpegDecoder::Probe(const QString &filename, const QAtomicInt *cancelled) const
{
  // We shouldn't be here for very long *at all*, so no point
  Q_UNUSED(cancelled);
  return {};
}

bool FFmpegDecoder::ConformAudioInternal(const QVector<QString> &filenames, const AudioParams &params, const QAtomicInt *cancelled)
{
  return true;
}

constexpr VideoParams::Format FFmpegDecoder::GetNativePixelFormat(AVPixelFormat pix_fmt) noexcept
{
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

QString FFmpegDecoder::id() const
{
  return QStringLiteral("ffmpeg");
}

constexpr int FFmpegDecoder::GetNativeChannelCount(AVPixelFormat pix_fmt) noexcept
{
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

std::int64_t FFmpegDecoder::ValidateChannelLayout(const AVStream* stream) noexcept
{
  if (stream->codecpar->channel_layout) {
    return stream->codecpar->channel_layout;
  }

  return av_get_default_channel_layout(stream->codecpar->channels);
}

constexpr const char *FFmpegDecoder::GetInterlacingModeInFFmpeg(VideoParams::Interlacing interlacing) noexcept
{
  if (interlacing == VideoParams::kInterlacedTopFirst) {
    return "tff";
  }

  return "bff";
}
}
