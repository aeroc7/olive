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

#include "ffmpegdecodeframe.h"

#include <common/ffmpegutils.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

namespace olive {
bool FFmpegDecodeFrame::open(const std::string &filename) {
  frame_.reset(av_frame_alloc());
  frame_cnvt_.reset(av_frame_alloc());

  return input_open_internal(filename);
}

void FFmpegDecodeFrame::setup_cnvt_process() noexcept {
  if (buf_size_) {
    return;
  }

  const auto vid_stream = get_format_ctx()->streams[get_best_video_stream_id()];
  ideal_pix_fmt_ = FFmpegUtils::GetCompatiblePixelFormat(static_cast<AVPixelFormat>(vid_stream->codecpar->format));

  buf_size_ =
      av_image_get_buffer_size(ideal_pix_fmt_, get_codec_ctx()->width, get_codec_ctx()->height, FRAME_BUF_ALIGNMENT);
  cnvt_buf_ = static_cast<std::uint8_t *>(av_malloc(buf_size_ * sizeof(std::uint8_t)));

  av_image_fill_arrays(frame_cnvt_->data, frame_cnvt_->linesize, cnvt_buf_, ideal_pix_fmt_, get_codec_ctx()->width,
                       get_codec_ctx()->height, FRAME_BUF_ALIGNMENT);

  sws_ctx_ =
      sws_getContext(get_codec_ctx()->width, get_codec_ctx()->height, get_codec_ctx()->pix_fmt, get_codec_ctx()->width,
                     get_codec_ctx()->height, ideal_pix_fmt_, SWS_BILINEAR, nullptr, nullptr, nullptr);
}

AVFrame *FFmpegDecodeFrame::decode_frame() {
  try {
    AVPacket pkt{};
    int ret{};

    if (buf_size_ == 0) {
      setup_cnvt_process();
    }

    while (av_read_frame(get_format_ctx(), &pkt) >= 0) {
      if (pkt.stream_index == get_best_video_stream_id()) {
        ret = avcodec_send_packet(get_codec_ctx(), &pkt);
        if (ret < 0) {
          qWarning() << "Error sending packet for decoding.";
          throw DecoderError{DecoderErrorDesc::FAILURE, ret};
        }

        bool frm_success = (ret >= 0);

        while (frm_success) {
          ret = avcodec_receive_frame(get_codec_ctx(), frame_.get());

          if (ret == AVERROR(EAGAIN)) {
            frm_success = false;
            break;
          } else if (ret == AVERROR_EOF) {
            break;
          } else if (ret < 0) {
            qWarning() << "Error while decoding.";
            throw DecoderError{DecoderErrorDesc::FAILURE, ret};
          }

          sws_scale(sws_ctx_, static_cast<uint8_t const *const *>(frame_->data), frame_->linesize, 0,
                    get_codec_ctx()->height, frame_cnvt_->data, frame_cnvt_->linesize);

          frame_cnvt_->width = get_codec_ctx()->width;
          frame_cnvt_->height = get_codec_ctx()->height;
          frame_cnvt_->format = ideal_pix_fmt_;

          return frame_cnvt_.get();
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

  return nullptr;
}
}  // namespace olive
