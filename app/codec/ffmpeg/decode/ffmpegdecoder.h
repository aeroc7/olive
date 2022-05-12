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

#ifndef FFMPEGDECODER_H
#define FFMPEGDECODER_H

#include "ffmpegdecodeframe.h"

#include <codec/decoder.h>

struct AVStream;

namespace olive {
/**
 * @brief A Decoder derivative that wraps FFmpeg functions as on Olive decoder
 */
class FFmpegDecoder : public Decoder
{
  Q_OBJECT
public:
  FFmpegDecoder() = default;
  FFmpegDecoder(const FFmpegDecoder &) = delete;
  FFmpegDecoder(FFmpegDecoder &&) = delete;
  FFmpegDecoder &operator=(const FFmpegDecoder &) = delete;
  FFmpegDecoder &operator=(FFmpegDecoder &&) = delete;
  ~FFmpegDecoder() = default;

  virtual QString id() const override;

  virtual bool SupportsVideo() override{return true;}
  virtual bool SupportsAudio() override{return true;}

  virtual FootageDescription Probe(const QString &filename, const QAtomicInt *cancelled) const override;

protected:
  virtual bool OpenInternal() override;
  virtual FramePtr RetrieveVideoInternal(const rational &timecode, const RetrieveVideoParams& params, const QAtomicInt *cancelled) override;
  virtual bool ConformAudioInternal(const QVector<QString>& filenames, const AudioParams &params, const QAtomicInt* cancelled) override;
  virtual void CloseInternal() override;

private:
  std::unique_ptr<FFmpegDecodeFrame> frame_decode_;
};

}

#endif // FFMPEGDECODER_H
