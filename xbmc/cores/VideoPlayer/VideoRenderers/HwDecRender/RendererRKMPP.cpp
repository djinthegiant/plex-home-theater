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
  for (int i = 0; i < m_numRenderBuffers; ++i)
    m_buffers[i].videoBuffer = nullptr;
}

CRendererRKMPP::~CRendererRKMPP()
{
  SetVideoPlane(nullptr);

  for (int i = 0; i < m_numRenderBuffers; ++i)
    ReleaseBuffer(i);
}

bool CRendererRKMPP::HandlesVideoBuffer(CVideoBuffer* buffer)
{
  RKMPP::CVideoBufferRKMPP* vb = dynamic_cast<RKMPP::CVideoBufferRKMPP*>(buffer);
  if (vb)
    return true;

  return false;
}

bool CRendererRKMPP::Configure(const VideoPicture& picture, float fps, unsigned flags, unsigned int orientation)
{
  m_format = picture.videoBuffer->GetFormat();
  m_sourceWidth = picture.iWidth;
  m_sourceHeight = picture.iHeight;
  m_renderOrientation = orientation;

  // Save the flags.
  m_iFlags = flags;

  // Calculate the input frame aspect ratio.
  CalculateFrameAspectRatio(picture.iDisplayWidth, picture.iDisplayHeight);
  SetViewMode(CMediaSettings::GetInstance().GetCurrentVideoSettings().m_ViewMode);
  ManageRenderArea();

  m_bConfigured = true;

  for (int i = 0; i < m_numRenderBuffers; ++i)
    ReleaseBuffer(i);

  m_iLastRenderBuffer = -1;

  return true;
}

void CRendererRKMPP::AddVideoPicture(const VideoPicture& picture, int index)
{
  if (g_advancedSettings.CanLogComponent(LOGVIDEO))
    CLog::Log(LOGDEBUG, "%s::%s - index:%d buffer:%p", CLASSNAME, __FUNCTION__, index, picture.videoBuffer);

  ReleaseBuffer(index); // probably useless

  BUFFER& buf = m_buffers[index];
  buf.videoBuffer = picture.videoBuffer;
  buf.videoBuffer->Acquire();
}

void CRendererRKMPP::FlipPage(int source)
{
  if (g_advancedSettings.CanLogComponent(LOGVIDEO))
    CLog::Log(LOGDEBUG, "%s::%s - source:%d renderbuffer:%d", CLASSNAME, __FUNCTION__, source, m_iRenderBuffer);

  if (source >= 0 && source < m_numRenderBuffers)
    m_iRenderBuffer = source;
  else
    m_iRenderBuffer = (m_iRenderBuffer + 1) % m_numRenderBuffers;
}

void CRendererRKMPP::ReleaseBuffer(int index)
{
  BUFFER& buf = m_buffers[index];
  if (buf.videoBuffer)
  {
    if (g_advancedSettings.CanLogComponent(LOGVIDEO))
      CLog::Log(LOGDEBUG, "%s::%s - index:%d buffer:%p", CLASSNAME, __FUNCTION__, index, buf.videoBuffer);

    RKMPP::CVideoBufferRKMPP* vb = dynamic_cast<RKMPP::CVideoBufferRKMPP*>(buf.videoBuffer);
    if (vb)
      vb->Release();
    buf.videoBuffer = nullptr;
  }
}

bool CRendererRKMPP::NeedBuffer(int index)
{
  return m_iLastRenderBuffer == index;
}

CRenderInfo CRendererRKMPP::GetRenderInfo()
{
  CRenderInfo info;
  info.max_buffer_size = m_numRenderBuffers;
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

  RKMPP::CVideoBufferRKMPP* vb = dynamic_cast<RKMPP::CVideoBufferRKMPP*>(m_buffers[m_iRenderBuffer].videoBuffer);
  if (vb)
  {
    if (g_advancedSettings.CanLogComponent(LOGVIDEO))
      CLog::Log(LOGDEBUG, "%s::%s - index:%d buffer:%p pts:%" PRId64, CLASSNAME, __FUNCTION__, m_iRenderBuffer, vb, vb->GetPTS());

    SetVideoPlane(vb);
  }

  m_iLastRenderBuffer = m_iRenderBuffer;
}

bool CRendererRKMPP::RenderCapture(CRenderCapture* capture)
{
  capture->BeginRender();
  capture->EndRender();
  return true;
}

bool CRendererRKMPP::ConfigChanged(const VideoPicture& picture)
{
  if (picture.videoBuffer->GetFormat() != m_format)
    return true;

  return false;
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

void CRendererRKMPP::SetVideoPlane(RKMPP::CVideoBufferRKMPP* buffer)
{
  // TODO: implement
}
