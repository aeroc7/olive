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

#include "ffmpegdecodebase.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

#include <QDebug>
#include <stdexcept>

namespace {
AVCodecContext *ffmpegav_alloc_and_set_ctx(const AVCodec *codec, const AVCodecParameters *codecpar) {
  using namespace olive;
  int ret{};

  AVCodecContext *codec_ctx = avcodec_alloc_context3(codec);
  if (!codec_ctx) {
    qWarning() << "Failed to create codec context.";
    throw DecoderError(DecoderErrorDesc::FAILURE);
  }

  ret = avcodec_parameters_to_context(codec_ctx, codecpar);
  if (ret < 0) {
    qWarning() << "Failed to fill context with paramters.";
    throw DecoderError(DecoderErrorDesc::FAILURE, ret);
  }

  return codec_ctx;
}
}  // namespace

namespace olive {
void FFmpegDecodeBase::input_open_internal(const std::string &url) {
  try {
    int ret{};

    // Note: avformat_open_input will allocate our context for us.
    ret = avformat_open_input(&format_ctx_, url.c_str(), nullptr, nullptr);
    if (ret < 0) {
      qWarning() << "Failed to open input stream and/or read the header of: " << QString::fromStdString(url);
      throw DecoderError(DecoderErrorDesc::FAILURE, ret);
    }

    ret = avformat_find_stream_info(format_ctx_, nullptr);
    if (ret < 0) {
      qWarning() << "Failed to read stream information.";
      throw DecoderError(DecoderErrorDesc::FAILURE, ret);
    }

    find_best_stream();
    find_decoder();
    setup_decoder();
    open_codec();

  } catch (const DecoderError &e) {
    qWarning() << "Caught exception: " << QString::fromStdString(e.error_string()) << " (code " << e.error_code()
               << ")";
  } catch (const std::exception &e) {
    qWarning() << "Caught exception: " << e.what();
  }
}

void FFmpegDecodeBase::find_best_stream() {
  int ret{};

  ret = av_find_best_stream(format_ctx_, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
  if (ret < 0) {
    qWarning() << "Failed to find valid stream/decoder.";
    throw DecoderError(DecoderErrorDesc::FAILURE, ret);
  }

  best_vid_stream_id_ = ret;
}

void FFmpegDecodeBase::find_decoder() {
  const auto decoder_id = get_decoder_id();

  codec_ = avcodec_find_decoder(static_cast<AVCodecID>(decoder_id));
  if (!codec_) {
    qWarning() << "Unsupported codec";
    throw DecoderError(DecoderErrorDesc::FAILURE);
  }
}

int FFmpegDecodeBase::get_decoder_id() noexcept {
  Q_ASSERT(get_best_video_stream_id() >= 0);

  const AVCodecID id = format_ctx_->streams[get_best_video_stream_id()]->codecpar->codec_id;

  return id;
}

void FFmpegDecodeBase::setup_decoder() {
  Q_ASSERT(get_codec());
  Q_ASSERT(get_format_ctx());

  codec_ctx_orig_ =
      ffmpegav_alloc_and_set_ctx(get_codec(), get_format_ctx()->streams[get_best_video_stream_id()]->codecpar);
  codec_ctx_ = ffmpegav_alloc_and_set_ctx(get_codec(), get_format_ctx()->streams[get_best_video_stream_id()]->codecpar);
}

void FFmpegDecodeBase::open_codec() {
  int ret{};

  // auto-determine thread number
  codec_ctx_->thread_count = 0;

  if (codec_->capabilities | AV_CODEC_CAP_FRAME_THREADS) {
    codec_ctx_->thread_type = FF_THREAD_FRAME;
  } else if (codec_->capabilities | AV_CODEC_CAP_SLICE_THREADS) {
    codec_ctx_->thread_type = FF_THREAD_SLICE;
  } else {
    codec_ctx_->thread_count = 1;  // don't use multithreading
  }

  ret = avcodec_open2(codec_ctx_, codec_, nullptr);
  if (ret < 0) {
    qWarning() << "Failed to open codec.";
    throw DecoderError(DecoderErrorDesc::FAILURE, ret);
  }
}

FFmpegDecodeBase::~FFmpegDecodeBase() {
  avcodec_close(codec_ctx_);

  avcodec_free_context(&codec_ctx_);
  avcodec_free_context(&codec_ctx_orig_);
}
}  // namespace olive