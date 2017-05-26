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

#include "cores/VideoPlayer/VideoRenderers/RenderCapture.h"
#include "settings/AdvancedSettings.h"
#include "settings/MediaSettings.h"
#include "utils/log.h"
#include "windowing/WindowingFactory.h"

#undef CLASSNAME
#define CLASSNAME "CRendererRKMPP"

CRendererRKMPP::CRendererRKMPP()
  : m_bConfigured(false)
  , m_iRenderBuffer(0)
  , m_iLastRenderBuffer(-1)
{
  if (g_advancedSettings.CanLogComponent(LOGVIDEO))
    CLog::Log(LOGDEBUG, "%s::%s - create %p", CLASSNAME, __FUNCTION__, this);
}

CRendererRKMPP::~CRendererRKMPP()
{
  if (g_advancedSettings.CanLogComponent(LOGVIDEO))
    CLog::Log(LOGDEBUG, "%s::%s - destroy %p", CLASSNAME, __FUNCTION__, this);

  for (int i = 0; i < m_numRenderBuffers; ++i)
    ReleaseBuffer(i);
}

bool CRendererRKMPP::Configure(unsigned int width, unsigned int height, unsigned int d_width, unsigned int d_height, float fps, unsigned flags, ERenderFormat format, unsigned extended_format, unsigned int orientation)
{
  CLog::Log(LOGNOTICE, "%s::%s - width:%u height:%u d_width:%u d_height:%u flags:%u orientation:%u configured:%d", CLASSNAME, __FUNCTION__, width, height, d_width, d_height, flags, orientation, m_bConfigured);

  m_sourceWidth = width;
  m_sourceHeight = height;
  m_renderOrientation = orientation;

  // Save the flags.
  m_iFlags = flags;
  m_format = format;

  // Calculate the input frame aspect ratio.
  CalculateFrameAspectRatio(d_width, d_height);
  SetViewMode(CMediaSettings::GetInstance().GetCurrentVideoSettings().m_ViewMode);
  ManageRenderArea();

  m_bConfigured = true;

  for (int i = 0; i < m_numRenderBuffers; ++i)
    m_buffers[i].hwPic = nullptr;

  m_iLastRenderBuffer = -1;

  return true;
}

int CRendererRKMPP::GetImage(YV12Image* image, int source, bool readonly)
{
  if (image == nullptr)
    return -1;

  // take next available buffer
  if (source == -1)
    source = (m_iRenderBuffer + 1) % m_numRenderBuffers;

  return source;
}

void CRendererRKMPP::AddVideoPictureHW(DVDVideoPicture& picture, int index)
{
  if (g_advancedSettings.CanLogComponent(LOGVIDEO))
    CLog::Log(LOGDEBUG, "%s::%s - index:%d drmprime:%p configured:%d", CLASSNAME, __FUNCTION__, index, picture.drmprime, m_bConfigured);

  ReleaseBuffer(index); // probably useless

  BUFFER& buf = m_buffers[index];
  buf.hwPic = picture.drmprime ? picture.drmprime->Retain() : nullptr;
}

void CRendererRKMPP::FlipPage(int source)
{
  if (g_advancedSettings.CanLogComponent(LOGVIDEO))
    CLog::Log(LOGDEBUG, "%s::%s - source:%d renderbuffer:%d configured:%d", CLASSNAME, __FUNCTION__, source, m_iRenderBuffer, m_bConfigured);

  if (source >= 0 && source < m_numRenderBuffers)
    m_iRenderBuffer = source;
  else
    m_iRenderBuffer = (m_iRenderBuffer + 1) % m_numRenderBuffers;
}

void CRendererRKMPP::ReleaseBuffer(int index)
{
  BUFFER& buf = m_buffers[index];
  if (buf.hwPic)
  {
    if (g_advancedSettings.CanLogComponent(LOGVIDEO))
      CLog::Log(LOGDEBUG, "%s::%s - index:%d drmprime:%p", CLASSNAME, __FUNCTION__, index, buf.hwPic);

    CDVDDrmPrimeInfo* info = static_cast<CDVDDrmPrimeInfo *>(buf.hwPic);
    SAFE_RELEASE(info);
    buf.hwPic = nullptr;
  }
}

bool CRendererRKMPP::NeedBuffer(int index)
{
  return m_iLastRenderBuffer == index;
}

CRenderInfo CRendererRKMPP::GetRenderInfo()
{
  CRenderInfo info;
  info.formats.push_back(RENDER_FMT_RKMPP);
  info.max_buffer_size = m_numRenderBuffers;
  info.optimal_buffer_size = m_numRenderBuffers;
  return info;
}

void CRendererRKMPP::Update()
{
  if (!m_bConfigured)
    return;

  ManageRenderArea();
}

void CRendererRKMPP::RenderUpdate(bool clear, unsigned int flags, unsigned int alpha)
{
  if (m_iLastRenderBuffer == m_iRenderBuffer)
    return;

  CDVDDrmPrimeInfo* info = static_cast<CDVDDrmPrimeInfo *>(m_buffers[m_iRenderBuffer].hwPic);
  if (info)
  {
    if (g_advancedSettings.CanLogComponent(LOGVIDEO))
      CLog::Log(LOGDEBUG, "%s::%s - drmprime:%p pts:%" PRId64, CLASSNAME, __FUNCTION__, info, info->GetPTS());
  }

  m_iLastRenderBuffer = m_iRenderBuffer;
}

bool CRendererRKMPP::RenderCapture(CRenderCapture* capture)
{
  capture->BeginRender();
  capture->EndRender();
  return true;
}

bool CRendererRKMPP::Supports(ERENDERFEATURE feature)
{
  if (feature == RENDERFEATURE_ZOOM ||
      feature == RENDERFEATURE_STRETCH ||
      feature == RENDERFEATURE_PIXEL_RATIO)
    return true;

  return false;
}

bool CRendererRKMPP::Supports(ESCALINGMETHOD method)
{
  return false;
}

#endif
