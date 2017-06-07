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
#include "windowing/gbm/GBMUtils.h"

#undef CLASSNAME
#define CLASSNAME "CRendererRKMPP"

CRendererRKMPP::CRendererRKMPP()
  : m_bConfigured(false)
  , m_iRenderBuffer(0)
  , m_iLastRenderBuffer(-1)
  , m_gem_handle(0)
  , m_fb_id(0)
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
  struct drm* drm = CGBMUtils::GetDrm();
  uint32_t gem_handle = 0;
  uint32_t fb_id = 0;

  if (buffer)
  {
    av_drmprime* drmprime = buffer->GetDrmPrime();
    uint32_t pitches[4] = { 0, 0, 0, 0 };
    uint32_t offsets[4] = { 0, 0, 0, 0 };
    uint32_t handles[4] = { 0, 0, 0, 0 };

    if (g_advancedSettings.CanLogComponent(LOGVIDEO))
      CLog::Log(LOGDEBUG, "%s::%s - buffer:%p width:%u height:%u", CLASSNAME, __FUNCTION__, buffer, buffer->GetWidth(), buffer->GetHeight());

    // get GEM handle from the prime fd
    int ret = drmPrimeFDToHandle(drm->fd, drmprime->fds[0], &gem_handle);
    if (ret < 0)
    {
      CLog::Log(LOGERROR, "%s::%s - failed to retrieve the GEM handle, ret = %d", CLASSNAME, __FUNCTION__, ret);
      return;
    }

    handles[0] = gem_handle;
    pitches[0] = drmprime->strides[0];
    offsets[0] = drmprime->offsets[0];

    handles[1] = gem_handle;
    pitches[1] = drmprime->strides[1];
    offsets[1] = drmprime->offsets[1];

    // add the video frame FB
    ret = drmModeAddFB2(drm->fd, buffer->GetWidth(), buffer->GetHeight(), drmprime->format, handles, pitches, offsets, &fb_id, 0);
    if (ret < 0)
    {
      CLog::Log(LOGERROR, "%s::%s - failed add drm layer %d", CLASSNAME, __FUNCTION__, fb_id);
      return;
    }

    int32_t crtc_x = (int32_t)m_destRect.x1;
    int32_t crtc_y = (int32_t)m_destRect.y1;
    uint32_t crtc_w = (uint32_t)m_destRect.Width();
    uint32_t crtc_h = (uint32_t)m_destRect.Height();
    uint32_t src_x = 0;
    uint32_t src_y = 0;
    uint32_t src_w = buffer->GetWidth() << 16;
    uint32_t src_h = buffer->GetHeight() << 16;

    // show the video frame FB on the video plane
    ret = drmModeSetPlane(drm->fd, drm->video_plane_id, drm->crtc_id, fb_id, 0,
                          crtc_x, crtc_y, crtc_w, crtc_h,
                          src_x, src_y, src_w, src_h);
    if (ret < 0)
    {
      CLog::Log(LOGERROR, "%s::%s - failed to set the plane %d (buffer %d)", CLASSNAME, __FUNCTION__, drm->video_plane_id, fb_id);
      return;
    }
  }

  // remove the previous video frame FB
  if (m_fb_id)
  {
    drmModeRmFB(drm->fd, m_fb_id);
  }
  m_fb_id = fb_id;

  // close the GEM handle for the previous video frame
  if (m_gem_handle)
  {
    struct drm_gem_close gem_close = { .handle = m_gem_handle };
    drmIoctl(drm->fd, DRM_IOCTL_GEM_CLOSE, &gem_close);
  }
  m_gem_handle = gem_handle;
}
