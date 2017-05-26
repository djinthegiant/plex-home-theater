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

#include "RKMPP.h"

#ifdef HAS_RKMPP

#include "cores/VideoPlayer/Process/ProcessInfo.h"
#include "cores/VideoPlayer/DVDClock.h"
#include "DVDVideoCodec.h"
#include "DVDCodecs/DVDCodecUtils.h"
#include "settings/AdvancedSettings.h"
#include "utils/log.h"
#include "windowing/WindowingFactory.h"

#undef CLASSNAME
#define CLASSNAME "RKMPP::CDVDDrmPrimeInfo"

CDVDDrmPrimeInfo::CDVDDrmPrimeInfo(AVFrame* frame)
  : m_refs(0)
  , m_frame(nullptr)
{
  // ref frame until MppFrame has been rendered
  m_frame = av_frame_alloc();
  if (m_frame)
    av_frame_ref(m_frame, frame);

  if (g_advancedSettings.CanLogComponent(LOGVIDEO))
    CLog::Log(LOGDEBUG, "%s::%s - create %p refs:%ld frame:%p drmprime:%p", CLASSNAME, __FUNCTION__, this, m_refs, m_frame, GetDrmPrime());
}

CDVDDrmPrimeInfo::~CDVDDrmPrimeInfo()
{
  if (g_advancedSettings.CanLogComponent(LOGVIDEO))
    CLog::Log(LOGDEBUG, "%s::%s - destroy %p refs:%ld frame:%p drmprime:%p", CLASSNAME, __FUNCTION__, this, m_refs, m_frame, GetDrmPrime());

  // unref frame to deinit MppFrame
  av_frame_free(&m_frame);
}

CDVDDrmPrimeInfo* CDVDDrmPrimeInfo::Retain()
{
  AtomicIncrement(&m_refs);

  if (g_advancedSettings.CanLogComponent(LOGVIDEO))
    CLog::Log(LOGDEBUG, "%s::%s - retain %p refs:%ld frame:%p drmprime:%p", CLASSNAME, __FUNCTION__, this, m_refs, m_frame, GetDrmPrime());

  return this;
}

long CDVDDrmPrimeInfo::Release()
{
  if (g_advancedSettings.CanLogComponent(LOGVIDEO))
    CLog::Log(LOGDEBUG, "%s::%s - release %p refs:%ld frame:%p drmprime:%p", CLASSNAME, __FUNCTION__, this, m_refs, m_frame, GetDrmPrime());

  long count = AtomicDecrement(&m_refs);
  if (count == 0)
    delete this;
  return count;
}

using namespace RKMPP;

#undef CLASSNAME
#define CLASSNAME "RKMPP::CDecoder"

CDecoder::CDecoder(CProcessInfo& processInfo)
  : m_processInfo(processInfo)
{
  if (g_advancedSettings.CanLogComponent(LOGVIDEO))
    CLog::Log(LOGDEBUG, "%s::%s - create %p", CLASSNAME, __FUNCTION__, this);
}

CDecoder::~CDecoder()
{
  if (g_advancedSettings.CanLogComponent(LOGVIDEO))
    CLog::Log(LOGDEBUG, "%s::%s - destroy %p", CLASSNAME, __FUNCTION__, this);
}

bool CDecoder::Open(AVCodecContext* avctx, AVCodecContext* mainctx, enum AVPixelFormat fmt, unsigned int surfaces)
{
  if (g_advancedSettings.CanLogComponent(LOGVIDEO))
    CLog::Log(LOGDEBUG, "%s::%s - avctx:%p mainctx:%p fmt:%d surfaces:%u", CLASSNAME, __FUNCTION__, avctx, mainctx, fmt, surfaces);

  m_processInfo.SetVideoDeintMethod("none");

  std::list<EINTERLACEMETHOD> deintMethods;
  deintMethods.push_back(EINTERLACEMETHOD::VS_INTERLACEMETHOD_NONE);
  m_processInfo.UpdateDeinterlacingMethods(deintMethods);

  return true;
}

int CDecoder::Decode(AVCodecContext* avctx, AVFrame* frame)
{
  int status = Check(avctx);
  if (status)
    return status;

  if (frame)
    return VC_BUFFER | VC_PICTURE;
  else
    return VC_BUFFER;
}

bool CDecoder::GetPicture(AVCodecContext* avctx, AVFrame* frame, DVDVideoPicture* picture)
{
  CDVDVideoCodecFFmpeg* ctx = static_cast<CDVDVideoCodecFFmpeg *>(avctx->opaque);
  bool ret = ctx->GetPictureCommon(picture);
  if (!ret || !frame)
    return false;

  picture->format = RENDER_FMT_RKMPP;
  picture->drmprime = (new CDVDDrmPrimeInfo(frame))->Retain();
  picture->pts = (double)frame->pts * DVD_TIME_BASE / AV_TIME_BASE;

  if (g_advancedSettings.CanLogComponent(LOGVIDEO))
    CLog::Log(LOGDEBUG, "%s::%s - width:%d height:%d pts:%" PRId64 " drmprime:%p", CLASSNAME, __FUNCTION__, frame->width, frame->height, frame->pts, picture->drmprime);

  return true;
}

void CDecoder::ClearPicture(DVDVideoPicture* picture)
{
  SAFE_RELEASE(picture->drmprime);
}

int CDecoder::Check(AVCodecContext* avctx)
{
  return 0;
}

unsigned CDecoder::GetAllowedReferences()
{
  return 4;
}

#endif
