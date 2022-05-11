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

struct AVFormatContext;
struct AVCodecContext;
struct AVCodec;
struct AVPacket;
struct SwsContext;
struct AvFramePtr;

namespace olive {
class FFmpegDecodeFrame {
public:
  FFmpegDecodeFrame();
  ~FFmpegDecodeFrame();

  void open_input(const std::string &url);

  AVFrame *decode_frame();
  double clip_fps() const noexcept;

private:
  void find_best_stream();
  void find_decoder();
  int get_decoder_id() noexcept;
  void setup_decoder();
  void open_codec();

  bool packet_is_from_video_stream(const AVPacket *p) const noexcept;

  void setup_cnvt_process() noexcept;

  AVFormatContext *format_ctx_{nullptr};
  const AVCodec *codec_{nullptr};
  AVCodecContext *codec_ctx_orig_{nullptr}, *codec_ctx_{nullptr};

  AVFramePtr frame, frame_cnvt;

  SwsContext *sws_ctx{nullptr};

  int best_vid_stream_id_{-1};
  int buf_size{};

  std::uint8_t *cnvt_buf{nullptr};

  static constexpr auto FRAME_BUF_ALIGNMENT = 32;
};
}  // namespace olive

#endif  // FFMPEGDECODEFRAME_H