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

#include "cores/VideoPlayer/TimingConstants.h"
#include "settings/AdvancedSettings.h"
#include "settings/Settings.h"
#include "threads/SingleLock.h"
#include "utils/log.h"
#include "ServiceBroker.h"

using namespace RKMPP;

//------------------------------------------------------------------------------
// Video Buffers
//------------------------------------------------------------------------------

CVideoBufferRKMPP::CVideoBufferRKMPP(IVideoBufferPool& pool, int id)
  : CVideoBuffer(id)
{
  m_pFrame = av_frame_alloc();
}

CVideoBufferRKMPP::~CVideoBufferRKMPP()
{
  av_frame_free(&m_pFrame);
}

void CVideoBufferRKMPP::SetRef(AVFrame* frame)
{
  av_frame_unref(m_pFrame);
  av_frame_ref(m_pFrame, frame);
}

void CVideoBufferRKMPP::Unref()
{
  av_frame_unref(m_pFrame);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

class RKMPP::CVideoBufferPoolRKMPP : public IVideoBufferPool
{
public:
  virtual ~CVideoBufferPoolRKMPP();
  virtual void Return(int id) override;
  virtual CVideoBuffer* Get() override;

protected:
  CCriticalSection m_critSection;
  std::vector<CVideoBufferRKMPP*> m_all;
  std::deque<int> m_used;
  std::deque<int> m_free;
};

CVideoBufferPoolRKMPP::~CVideoBufferPoolRKMPP()
{
  for (auto buf : m_all)
  {
    delete buf;
  }
}

CVideoBuffer* CVideoBufferPoolRKMPP::Get()
{
  CSingleLock lock(m_critSection);

  CVideoBufferRKMPP* buf = nullptr;
  if (!m_free.empty())
  {
    int idx = m_free.front();
    m_free.pop_front();
    m_used.push_back(idx);
    buf = m_all[idx];
  }
  else
  {
    int id = m_all.size();
    buf = new CVideoBufferRKMPP(*this, id);
    m_all.push_back(buf);
    m_used.push_back(id);
  }

  buf->Acquire(GetPtr());
  return buf;
}

void CVideoBufferPoolRKMPP::Return(int id)
{
  CSingleLock lock(m_critSection);

  m_all[id]->Unref();
  auto it = m_used.begin();
  while (it != m_used.end())
  {
    if (*it == id)
    {
      m_used.erase(it);
      break;
    }
    else
      ++it;
  }
  m_free.push_back(id);
}

//------------------------------------------------------------------------------
// main class
//------------------------------------------------------------------------------

#undef CLASSNAME
#define CLASSNAME "CDecoderRKMPP"

CDecoder::CDecoder(CProcessInfo& processInfo)
  : m_processInfo(processInfo)
  , m_renderBuffer(nullptr)
{
  m_videoBufferPool = std::make_shared<CVideoBufferPoolRKMPP>();
}

CDecoder::~CDecoder()
{
  if (m_renderBuffer)
    m_renderBuffer->Release();
}

bool CDecoder::Open(AVCodecContext* avctx, AVCodecContext* mainctx, enum AVPixelFormat fmt)
{
  //if (!CServiceBroker::GetSettings().GetBool(CSettings::SETTING_VIDEOPLAYER_USERKMPP))
  //  return false;

  m_processInfo.SetVideoDeintMethod("none");

  std::list<EINTERLACEMETHOD> deintMethods;
  deintMethods.push_back(EINTERLACEMETHOD::VS_INTERLACEMETHOD_NONE);
  m_processInfo.UpdateDeinterlacingMethods(deintMethods);

  return true;
}

CDVDVideoCodec::VCReturn CDecoder::Decode(AVCodecContext* avctx, AVFrame* frame)
{
  CDVDVideoCodec::VCReturn status = Check(avctx);
  if (status)
    return status;

  if (frame)
  {
    if (m_renderBuffer)
      m_renderBuffer->Release();
    m_renderBuffer = dynamic_cast<CVideoBufferRKMPP*>(m_videoBufferPool->Get());
    m_renderBuffer->SetRef(frame);
    return CDVDVideoCodec::VC_PICTURE;
  }
  else
    return CDVDVideoCodec::VC_BUFFER;
}

bool CDecoder::GetPicture(AVCodecContext* avctx, VideoPicture* picture)
{
  if (g_advancedSettings.CanLogComponent(LOGVIDEO))
    CLog::Log(LOGDEBUG, "%s::%s - buffer:%p width:%u height:%u pts:%" PRId64, CLASSNAME, __FUNCTION__, m_renderBuffer, m_renderBuffer->GetWidth(), m_renderBuffer->GetHeight(), m_renderBuffer->GetPTS());

  ((ICallbackHWAccel*)avctx->opaque)->GetPictureCommon(picture);

  if (picture->videoBuffer)
    picture->videoBuffer->Release();

  picture->videoBuffer = m_renderBuffer;
  picture->videoBuffer->Acquire();

  picture->dts = DVD_NOPTS_VALUE;
  picture->pts = (double)m_renderBuffer->GetPTS() * DVD_TIME_BASE / AV_TIME_BASE;

  return true;
}

CDVDVideoCodec::VCReturn CDecoder::Check(AVCodecContext* avctx)
{
  return CDVDVideoCodec::VC_NONE;
}

unsigned CDecoder::GetAllowedReferences()
{
  return 4;
}
