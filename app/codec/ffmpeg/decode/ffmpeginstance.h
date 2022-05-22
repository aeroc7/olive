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

#ifndef FFMPEGINSTANCE_H
#define FFMPEGINSTANCE_H

#include <cstdint>
#include <string_view>

class QString;
struct AVFormatContext;
struct AVStream;
struct AVPacket;
struct AVFrame;
struct AVSubtitle;
struct AVCodecContext;
struct AVDictionary;

namespace olive {
class DecoderInstance {
public:
  DecoderInstance() = default;
  ~DecoderInstance();

  DecoderInstance(const DecoderInstance&) = delete;
  DecoderInstance(DecoderInstance&&) noexcept = delete;
  DecoderInstance& operator=(const DecoderInstance&) = delete;
  DecoderInstance& operator=(DecoderInstance&&) noexcept = delete;

  bool Open(const char *fn, int stream_index);

  /**
   * @brief Uses the FFmpeg API to retrieve a packet (stored in pkt_) and decode it (stored in frame_)
   *
   * @return
   *
   * An FFmpeg error code, or >= 0 on success
   */
  int GetFrame(AVPacket* pkt, AVFrame* frame) noexcept;

  std::string_view GetSubtitleHeader() const noexcept;

  int GetSubtitle(AVPacket* pkt, AVSubtitle* sub) noexcept;

  int GetPacket(AVPacket* pkt) noexcept;

  void Seek(std::int64_t timestamp) noexcept;

  AVFormatContext* fmt_ctx() const noexcept { return fmt_ctx_; }

  AVStream* avstream() const noexcept { return avstream_; }

private:
  void Close() noexcept;

  AVFormatContext* fmt_ctx_{nullptr};
  AVCodecContext* codec_ctx_{nullptr};
  AVStream* avstream_{nullptr};
  AVDictionary* opts_{nullptr};
};
}  // namespace olive

#endif  // FFMPEGINSTANCE_H