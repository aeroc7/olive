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

#include "ffmpegdecoder.h"
#include "ffmpegdecodeinfo.h"

extern "C" {
#include <libavformat/avformat.h>
}

namespace olive {
bool FFmpegDecoder::OpenInternal()
{
  const auto filename_as_stdstr = stream().filename().toUtf8().toStdString();
  frame_decode_ = std::make_unique<FFmpegDecodeFrame>();

  const auto success = frame_decode_->open(filename_as_stdstr);

  return success;
}

FramePtr FFmpegDecoder::RetrieveVideoInternal(const rational &timecode, const RetrieveVideoParams &params, const QAtomicInt *cancelled)
{
  FramePtr fp = Frame::Create();

  auto next_frame = frame_decode_->decode_frame();

  if (!next_frame) {
    return {};
  }

  fp->set_video_params(VideoParams{next_frame->width, next_frame->height, VideoParams::kFormatUnsigned8, 3, 1, VideoParams::kInterlaceNone, 1});

  fp->set_timestamp(timecode);
  fp->allocate();
  Q_ASSERT(fp->is_allocated());

  memcpy(fp->data(), next_frame->data, fp->allocated_size());
  return fp;
}

void FFmpegDecoder::CloseInternal()
{
  frame_decode_.reset();
}

// Generate footage info from file, must be re-entrant
FootageDescription FFmpegDecoder::Probe(const QString &filename, const QAtomicInt *cancelled) const
{
  // We shouldn't be here for very long *at all*, so no point
  Q_UNUSED(cancelled);

  const auto fstr_as_std = filename.toUtf8().toStdString();
  FFmpegDecodeInfo probe_info{fstr_as_std};

  return probe_info.get();
}

bool FFmpegDecoder::ConformAudioInternal(const QVector<QString> &filenames, const AudioParams &params, const QAtomicInt *cancelled)
{
  return false;
}

QString FFmpegDecoder::id() const
{
  return FFmpegDecodeBase::DECODE_ID;
}
}
