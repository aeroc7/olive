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

#ifndef FFMPEGDECODEINFO_H
#define FFMPEGDECODEINFO_H

#include <node/project/footage/footagedescription.h>

struct AVFormatContext;

namespace olive {
class FFmpegDecodeInfo final {
public:
  static constexpr auto DECODE_ID = "ffmpeg";

  FFmpegDecodeInfo(const std::string &);
  FFmpegDecodeInfo(const FFmpegDecodeInfo &) = delete;
  FFmpegDecodeInfo(FFmpegDecodeInfo &&) = delete;
  FFmpegDecodeInfo &operator=(const FFmpegDecodeInfo &) = delete;
  FFmpegDecodeInfo &operator=(FFmpegDecodeInfo &&) = delete;

  bool is_valid() const noexcept { return info_is_valid_; }
  explicit operator bool() const noexcept { return is_valid(); }

  ~FFmpegDecodeInfo();

private:

  bool info_invalidated(int av_err_code) noexcept;

  void iterate_footage_streams();
  std::int64_t footage_duration() const noexcept;

  AVFormatContext *fmt_ctx_{nullptr};
  bool info_is_valid_{false};

  FootageDescription footage_desc{DECODE_ID};
};
}

#endif  // FFMPEGDECODEINFO_H