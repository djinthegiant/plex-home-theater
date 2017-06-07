/*
 *      Copyright (C) 2017 Team Kodi
 *      http://kodi.tv
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this Program; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#pragma once

#include "DVDVideoCodecFFmpeg.h"
#include "cores/VideoPlayer/DVDCodecs/Video/DVDVideoCodec.h"
#include "cores/VideoPlayer/Process/VideoBuffer.h"

extern "C" {
#include "libavcodec/drmprime.h"
#include "libavutil/frame.h"
}

class CProcessInfo;

namespace RKMPP
{
class CVideoBufferRKMPP;
class CVideoBufferPoolRKMPP;

class CVideoBufferRKMPP
  : public CVideoBuffer
{
public:
  CVideoBufferRKMPP(IVideoBufferPool& pool, int id);
  virtual ~CVideoBufferRKMPP();
  void SetRef(AVFrame* frame);
  void Unref();

  av_drmprime* GetDrmPrime() const { return (av_drmprime*)m_pFrame->data[3]; }
  uint32_t GetWidth() const { return m_pFrame->width; }
  uint32_t GetHeight() const { return m_pFrame->height; }
  int64_t GetPTS() const { return m_pFrame->pts; }
protected:
  AVFrame* m_pFrame;
};

class CDecoder
  : public IHardwareDecoder
{
public:
  CDecoder(CProcessInfo& processInfo);
  virtual ~CDecoder();
  bool Open(AVCodecContext* avctx, AVCodecContext* mainctx, const enum AVPixelFormat) override;
  CDVDVideoCodec::VCReturn Decode(AVCodecContext* avctx, AVFrame* frame) override;
  bool GetPicture(AVCodecContext* avctx, VideoPicture* picture) override;
  CDVDVideoCodec::VCReturn Check(AVCodecContext* avctx) override;
  unsigned GetAllowedReferences() override;
  const std::string Name() override { return "rkmpp"; }

protected:
  CProcessInfo& m_processInfo;
  CVideoBufferRKMPP* m_renderBuffer;
  std::shared_ptr<CVideoBufferPoolRKMPP> m_videoBufferPool;
};

}
