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

#include <common/ffmpegutils.h>

extern "C" {
#include <libavformat/avformat.h>
}

namespace olive {

FFmpegDecodeInfo::FFmpegDecodeInfo(const std::string &filename) {
  input_open_internal(filename);
  set_frame_params();
}

FootageDescription FFmpegDecodeInfo::get() const noexcept {
  return footage_desc;
}

std::tuple<AVFramePtr, int> FFmpegDecodeInfo::recieve_single_frame() {
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

          if (ret == AVERROR(EAGAIN)) {
            frm_success = false;
            break;
          } else if(ret == AVERROR_EOF) {
            break;
          } else if (ret < 0) {
            qWarning() << "Error while decoding.";
            throw DecoderError{DecoderErrorDesc::FAILURE, ret};
          }

          return {std::move(frame), ret};
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

  return {nullptr, -1};
}

void FFmpegDecodeInfo::set_frame_params() {
  auto first_frame = std::get<0>(recieve_single_frame());
  if (!first_frame) {
    return;
  }

  if (first_frame->interlaced_frame) {
    if (first_frame->top_field_first) {
      p.set_interlacing(VideoParams::kInterlacedTopFirst);
    } else {
      p.set_interlacing(VideoParams::kInterlacedBottomFirst);
    }
  } else {
    p.set_interlacing(VideoParams::kInterlaceNone);
  }

  auto second_frame_pkg = recieve_single_frame();
  if (std::get<1>(second_frame_pkg) == AVERROR_EOF) {
    p.set_video_type(VideoParams::kVideoTypeStill);
  } else {
    p.set_video_type(VideoParams::kVideoTypeVideo);
  }

  auto second_frame = std::move(std::get<0>(second_frame_pkg));
  if (!second_frame) {
    return;
  }

  const auto compatible_pix_fmt =
      FFmpegUtils::GetCompatiblePixelFormat(static_cast<AVPixelFormat>(get_video_avstream()->codecpar->format));

  // Missing? Manual duration-getting?
  p.set_stream_index(get_best_video_stream_id());
  p.set_width(get_video_avstream()->codecpar->width);
  p.set_height(get_video_avstream()->codecpar->height);
  p.set_format(GetNativePixelFormat(compatible_pix_fmt));
  p.set_channel_count(GetNativeChannelCount(compatible_pix_fmt));
  p.set_pixel_aspect_ratio(get_pixel_aspect_ratio(first_frame.get()));
  p.set_frame_rate(get_frame_rate(first_frame.get()));
  p.set_start_time(get_video_avstream()->start_time);
  p.set_time_base(get_video_avstream()->time_base);
  p.set_duration(get_video_avstream()->duration);
  p.set_premultiplied_alpha(false);
  footage_desc.AddVideoStream(p);
}

rational FFmpegDecodeInfo::get_pixel_aspect_ratio(AVFrame *f) noexcept {
  const auto px_aspect_ratio = av_guess_sample_aspect_ratio(get_format_ctx(), get_video_avstream(), f);
  return px_aspect_ratio;
}

rational FFmpegDecodeInfo::get_frame_rate(AVFrame *f) noexcept {
  const auto frame_rate = av_guess_frame_rate(get_format_ctx(), get_video_avstream(), f);
  return frame_rate;
}

AVStream *FFmpegDecodeInfo::get_video_avstream() noexcept {
  const auto cur_stream = get_format_ctx()->streams[get_best_video_stream_id()];
  return cur_stream;
}
}  // namespace olive