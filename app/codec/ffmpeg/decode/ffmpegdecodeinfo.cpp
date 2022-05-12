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
FFmpegDecodeInfo::FFmpegDecodeInfo(const std::string &filename) { input_open_internal(filename); }

AVFramePtr FFmpegDecodeInfo::recieve_single_frame() {
  try {
    AVPacketPtr pkt{av_packet_alloc()};
    AVFramePtr frame{av_frame_alloc()};
    int ret{};

    while (av_read_frame(get_format_ctx(), pkt.get()) >= 0) {
      if (pkt->stream_index == get_best_video_stream_id()) {
        ret = avcodec_send_packet(get_codec_ctx(), pkt.get());
        if (ret < 0) {
          qWarning() << "Error sending packet for decoding.";
          throw DecoderError{DecoderErrorDesc::FAILURE, ret};
        }

        bool frm_success = (ret >= 0);

        while (frm_success) {
          ret = avcodec_receive_frame(get_codec_ctx(), frame.get());

          if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            break;
          } else if (ret < 0) {
            qWarning() << "Error while decoding.";
            throw DecoderError{DecoderErrorDesc::FAILURE, ret};
          }

          return frame;
        }

        if (frm_success) {
          break;
        }
      }
    }

  } catch (const DecoderError &e) {
    qWarning() << "Caught exception: " << QString::fromStdString(e.error_string()) << " (code " << e.error_code()
               << ")";
  } catch (const std::exception &e) {
    qWarning() << "Caught exception: " << e.what();
  }

  return {};
}

std::int64_t FFmpegDecodeInfo::footage_duration() const noexcept { return get_format_ctx()->duration; }
}  // namespace olive