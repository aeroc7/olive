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

#ifndef FFMPEGDECODEFRAME_H
#define FFMPEGDECODEFRAME_H

#include "ffmpegdecodebase.h"

namespace olive {
class FFmpegDecodeFrame : public FFmpegDecodeBase {
public:
  bool open(const std::string &);
  AVFrame* decode_frame();
protected:
private:
  void setup_cnvt_process() noexcept;
  AVFramePtr frame_, frame_cnvt_;
  SwsContext *sws_ctx_{nullptr};
  int buf_size_{};
  std::uint8_t *cnvt_buf_{nullptr};

  static constexpr auto FRAME_BUF_ALIGNMENT = 32;
};
}  // namespace olive

#endif  // FFMPEGDECODEFRAME_H