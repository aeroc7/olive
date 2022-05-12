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
#include <libavutil/frame.h>
#include <libavcodec/packet.h>
}

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
}  // namespace olive

#endif  // FFMPEGWRAPPERS_H