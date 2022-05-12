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

#include "ffmpegdecodebase.h"

namespace olive {
class FFmpegDecodeInfo : public FFmpegDecode {
public:
  FFmpegDecodeInfo(const std::string &);
  FFmpegDecodeInfo(const FFmpegDecodeInfo &) = delete;
  FFmpegDecodeInfo(FFmpegDecodeInfo &&) = delete;
  FFmpegDecodeInfo &operator=(const FFmpegDecodeInfo &) = delete;
  FFmpegDecodeInfo &operator=(FFmpegDecodeInfo &&) = delete;

  FootageDescription get() const noexcept;

protected:
  std::tuple<AVFramePtr, int> recieve_single_frame();
  void set_frame_params();

  rational get_pixel_aspect_ratio(AVFrame *f) noexcept;
  rational get_frame_rate(AVFrame *f) noexcept;
  AVStream *get_video_avstream() noexcept;

private:
  FootageDescription footage_desc{DECODE_ID};
  VideoParams p;

};
}

#endif  // FFMPEGDECODEINFO_H