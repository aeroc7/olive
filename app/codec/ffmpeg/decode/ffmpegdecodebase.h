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

#ifndef FFMPEGDECODEBASE_H
#define FFMPEGDECODEBASE_H

#include "../ffmpegwrappers.h"

#include <array>
#include <string>

struct AVFormatContext;
struct AVCodecContext;
struct AVCodec;
struct AVPacket;
struct SwsContext;

namespace olive {
enum class DecoderErrorDesc { FAILURE = -1, SUCCESS = 0 };
class DecoderError {
public:
  constexpr DecoderError() = default;
  constexpr DecoderError(DecoderErrorDesc d) : desc(d) {}
  constexpr explicit DecoderError(DecoderErrorDesc d, int code) : desc(d), err_code(code) {}
  constexpr explicit operator bool() const noexcept { return status(); }
  constexpr bool status() const noexcept { return (desc != DecoderErrorDesc::SUCCESS); }
  constexpr int error_code() const noexcept { return err_code; }
  std::string error_string() const {
    std::array<char, 128> err_buf;
    av_strerror(err_code, err_buf.data(), err_buf.size());
    return std::string{err_buf.data()};
  }

private:
  DecoderErrorDesc desc{DecoderErrorDesc::SUCCESS};
  int err_code{};
};

class FFmpegDecode {
public:
  static constexpr auto DECODE_ID = "ffmpeg";

  FFmpegDecode(const FFmpegDecode &) = delete;
  FFmpegDecode(FFmpegDecode &&) = delete;
  FFmpegDecode &operator=(const FFmpegDecode &) = delete;
  FFmpegDecode &operator=(FFmpegDecode &&) = delete;

  FFmpegDecode() = default;
  virtual ~FFmpegDecode();

protected:
  virtual void input_open_internal(const std::string &);

  void find_best_stream();
  void find_decoder();
  int get_decoder_id() noexcept;
  void setup_decoder();
  void open_codec();

  AVFormatContext *get_format_ctx() const noexcept { return format_ctx_; }
  const AVCodec *get_codec() const noexcept { return codec_; }
  AVCodecContext *get_codec_ctx() const noexcept { return codec_ctx_; }
  int get_best_vid_stream_id() const noexcept { return best_vid_stream_id_; }

private:
  AVFormatContext *format_ctx_{nullptr};
  const AVCodec *codec_{nullptr};
  AVCodecContext *codec_ctx_orig_{nullptr}, *codec_ctx_{nullptr};

  int best_vid_stream_id_{-1};
};
}  // namespace olive

#endif  // FFMPEGDECODEBASE_H