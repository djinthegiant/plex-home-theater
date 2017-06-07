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

#include "system.h"

#include "cores/VideoPlayer/VideoRenderers/BaseRenderer.h"
#include "DVDCodecs/Video/RKMPP.h"

class CRendererRKMPP : public CBaseRenderer
{
public:
  CRendererRKMPP();
  virtual ~CRendererRKMPP();

  static bool HandlesVideoBuffer(CVideoBuffer* buffer);

  // Player functions
  bool Configure(const VideoPicture& picture, float fps, unsigned flags, unsigned int orientation) override;
  bool IsConfigured() override { return m_bConfigured; };
  void AddVideoPicture(const VideoPicture& picture, int index) override;
  void FlipPage(int source) override;
  void UnInit() override {};
  void Reset() override {};
  void ReleaseBuffer(int idx) override;
  bool NeedBuffer(int idx) override;
  bool IsGuiLayer() override { return false; };
  CRenderInfo GetRenderInfo() override;
  void Update() override;
  void RenderUpdate(bool clear, unsigned int flags = 0, unsigned int alpha = 255) override;
  bool RenderCapture(CRenderCapture* capture) override;
  bool ConfigChanged(const VideoPicture& picture) override;

  // Feature support
  bool SupportsMultiPassRendering() override { return false; };
  bool Supports(ERENDERFEATURE feature) override;
  bool Supports(ESCALINGMETHOD method) override;

private:
  void SetVideoPlane(RKMPP::CVideoBufferRKMPP* buffer);

  bool m_bConfigured;

  int m_iRenderBuffer;
  int m_iLastRenderBuffer;
  static const int m_numRenderBuffers = 4;

  struct BUFFER
  {
    CVideoBuffer* videoBuffer;
  } m_buffers[m_numRenderBuffers];

  uint32_t m_gem_handle;
  uint32_t m_fb_id;
};
