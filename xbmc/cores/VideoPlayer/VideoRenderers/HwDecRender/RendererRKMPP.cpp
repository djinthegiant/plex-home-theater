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

#include "RendererRKMPP.h"

#ifdef HAS_RKMPP

#include "cores/IPlayer.h"
#include "cores/VideoPlayer/VideoRenderers/RenderCapture.h"
#include "settings/AdvancedSettings.h"
#include "settings/MediaSettings.h"
#include "utils/GLUtils.h"
#include "utils/log.h"
#include "utils/SysfsUtils.h"
#include "windowing/WindowingFactory.h"

#undef CLASSNAME
#define CLASSNAME "CRendererRKMPP"

CRendererRKMPP::CRendererRKMPP()
: m_last_info(nullptr)
{
  if (g_advancedSettings.CanLogComponent(LOGVIDEO))
    CLog::Log(LOGDEBUG, "%s::%s - create %p", CLASSNAME, __FUNCTION__, this);
}

CRendererRKMPP::~CRendererRKMPP()
{
  if (g_advancedSettings.CanLogComponent(LOGVIDEO))
    CLog::Log(LOGDEBUG, "%s::%s - destroy %p", CLASSNAME, __FUNCTION__, this);

  m_last_info = nullptr;
}

bool CRendererRKMPP::RenderCapture(CRenderCapture* capture)
{
  capture->BeginRender();
  capture->EndRender();
  return true;
}

void CRendererRKMPP::AddVideoPictureHW(DVDVideoPicture& picture, int index)
{
  if (g_advancedSettings.CanLogComponent(LOGVIDEO))
    CLog::Log(LOGDEBUG, "%s::%s - add picture %p index:%d", CLASSNAME, __FUNCTION__, this, index);

  YUVBUFFER& buf = m_buffers[index];
  if (picture.drmprime)
    buf.hwDec = picture.drmprime->Retain();
}

void CRendererRKMPP::ReleaseBuffer(int index)
{
  if (g_advancedSettings.CanLogComponent(LOGVIDEO))
    CLog::Log(LOGDEBUG, "%s::%s - release buffer %p index:%d", CLASSNAME, __FUNCTION__, this, index);

  YUVBUFFER& buf = m_buffers[index];
  if (buf.hwDec)
  {
    CDVDDrmPrimeInfo* info = static_cast<CDVDDrmPrimeInfo *>(buf.hwDec);
    SAFE_RELEASE(info);
    buf.hwDec = nullptr;
  }
}

bool CRendererRKMPP::NeedBuffer(int index)
{
  YUVBUFFER& buf = m_buffers[index];
  return buf.hwDec == m_last_info;
}

int CRendererRKMPP::GetImageHook(YV12Image* image, int source, bool readonly)
{
  return source;
}

bool CRendererRKMPP::IsGuiLayer()
{
  return false;
}

bool CRendererRKMPP::Supports(ERENDERFEATURE feature)
{
  return false;
}

bool CRendererRKMPP::Supports(ESCALINGMETHOD method)
{
  return false;
}

bool CRendererRKMPP::LoadShadersHook()
{
  if (g_advancedSettings.CanLogComponent(LOGVIDEO))
    CLog::Log(LOGDEBUG, "%s::%s - load shaders hook %p", CLASSNAME, __FUNCTION__, this);

  m_renderMethod = RENDER_BYPASS;
  return false;
}

bool CRendererRKMPP::RenderHook(int index)
{
  if (g_advancedSettings.CanLogComponent(LOGVIDEO))
    CLog::Log(LOGDEBUG, "%s::%s - render hook %p index:%d", CLASSNAME, __FUNCTION__, this, index);

  return true;
}

bool CRendererRKMPP::RenderUpdateVideoHook(bool clear, DWORD flags, DWORD alpha)
{
  CDVDDrmPrimeInfo* info = static_cast<CDVDDrmPrimeInfo *>(m_buffers[m_iYV12RenderBuffer].hwDec);
  if (info)
  {
    if (m_last_info == info)
      return true;

    if (g_advancedSettings.CanLogComponent(LOGVIDEO))
      CLog::Log(LOGDEBUG, "%s::%s - render update video hook %p drmprime:%p pts:%" PRId64, CLASSNAME, __FUNCTION__, this, info, info->GetPTS());

#ifdef HAVE_GBM
    // inject video frame into gbm windowing system
    g_Windowing.SetVideoPlane(info->GetWidth(), info->GetHeight(), info->GetDrmPrime());
#endif

    m_last_info = info;
  }
  return true;
}

#endif
