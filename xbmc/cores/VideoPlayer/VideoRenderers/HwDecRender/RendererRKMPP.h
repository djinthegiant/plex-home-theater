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

#ifdef HAS_RKMPP

#include "cores/VideoPlayer/VideoRenderers/BaseRenderer.h"
#include "DVDCodecs/Video/RKMPP.h"

class CRendererRKMPP : public CBaseRenderer
{
public:
  CRendererRKMPP();
  virtual ~CRendererRKMPP();

  // Player functions
  bool Configure(unsigned int width, unsigned int height, unsigned int d_width, unsigned int d_height, float fps, unsigned flags, ERenderFormat format, unsigned extended_format, unsigned int orientation) override;
  bool IsConfigured() override { return m_bConfigured; };
  int GetImage(YV12Image* image, int source = -1, bool readonly = false) override;
  void ReleaseImage(int source, bool preserve = false) override {};
  void AddVideoPictureHW(DVDVideoPicture& picture, int index) override;
  void FlipPage(int source) override;
  void PreInit() override {};
  void UnInit() override {};
  void Reset() override {};
  void ReleaseBuffer(int idx) override;
  bool NeedBuffer(int idx) override;
  bool IsGuiLayer() override { return false; };
  CRenderInfo GetRenderInfo() override;
  void Update() override;
  void RenderUpdate(bool clear, unsigned int flags = 0, unsigned int alpha = 255) override;
  bool RenderCapture(CRenderCapture* capture) override;

  // Feature support
  bool SupportsMultiPassRendering() override { return false; };
  bool Supports(ERENDERFEATURE feature) override;
  bool Supports(ESCALINGMETHOD method) override;

private:
  bool m_bConfigured;

  int m_iRenderBuffer;
  int m_iLastRenderBuffer;
  static const int m_numRenderBuffers = 4;

  struct BUFFER
  {
    void *hwPic;
  } m_buffers[m_numRenderBuffers];
};

#endif
