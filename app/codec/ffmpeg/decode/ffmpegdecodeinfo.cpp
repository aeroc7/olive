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

#include "ffmpegdecodeinfo.h"

extern "C" {
#include <libavformat/avformat.h>
}

namespace olive {
FFmpegDecodeInfo::FFmpegDecodeInfo(const std::string &filename) {
  int err{0};

  err = avformat_open_input(&fmt_ctx_, filename.c_str(), nullptr, nullptr);
  if (info_invalidated(err)) {
    return;
  }

  err = avformat_find_stream_info(fmt_ctx_, nullptr);
  if (info_invalidated(err)) {
    return;
  }
}

bool FFmpegDecodeInfo::info_invalidated(int av_err_code) noexcept {
  info_is_valid_ = (av_err_code == 0);
  return !info_is_valid_;
}

void FFmpegDecodeInfo::iterate_footage_streams() {
  Q_ASSERT(is_valid());

  for (unsigned i = 0; i < fmt_ctx_->nb_streams; i += 1) {
    auto avstream = fmt_ctx_->streams[i];

    const AVCodec *avdecoder = avcodec_find_decoder(avstream->codecpar->codec_id);
    if (!avdecoder) {
      continue;
    }

    switch (avstream->codecpar->codec_type) {
      case AVMEDIA_TYPE_VIDEO:
      case AVMEDIA_TYPE_AUDIO:
      default:
        break;
    }
  }
}

std::int64_t FFmpegDecodeInfo::footage_duration() const noexcept {
  Q_ASSERT(is_valid());
  return fmt_ctx_->duration;
}

FFmpegDecodeInfo::~FFmpegDecodeInfo() { avformat_close_input(&fmt_ctx_); }
}  // namespace olive