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

#ifdef HAS_RKMPP

#include "DVDVideoCodecFFmpeg.h"

extern "C" {
#include "libavcodec/drmprime.h"
#include "libavutil/frame.h"
}

class CProcessInfo;

class CDVDDrmPrimeInfo
{
public:
  CDVDDrmPrimeInfo(AVFrame* frame);

  // reference counting
  CDVDDrmPrimeInfo* Retain();
  long Release();

  av_drmprime* GetDrmPrime() const { return (av_drmprime*)m_frame->data[3]; }
  uint32_t GetWidth() const { return m_frame->width; }
  uint32_t GetHeight() const { return m_frame->height; }
  int64_t GetPTS() const { return m_frame->pts; }

private:
  // private because we are reference counted
  virtual ~CDVDDrmPrimeInfo();

  long m_refs;
  AVFrame* m_frame;
};

namespace RKMPP
{

class CDecoder
  : public CDVDVideoCodecFFmpeg::IHardwareDecoder
{
public:
  CDecoder(CProcessInfo& processInfo);
  virtual ~CDecoder();
  bool Open(AVCodecContext* avctx, AVCodecContext* mainctx, const enum AVPixelFormat, unsigned int surfaces = 0) override;
  int Decode(AVCodecContext* avctx, AVFrame* frame) override;
  bool GetPicture(AVCodecContext* avctx, AVFrame* frame, DVDVideoPicture* picture) override;
  void ClearPicture(DVDVideoPicture* picture) override;
  int Check(AVCodecContext* avctx) override;
  unsigned GetAllowedReferences() override;
  const std::string Name() override { return "rkmpp"; }

protected:
  CProcessInfo& m_processInfo;
};

}

#endif
