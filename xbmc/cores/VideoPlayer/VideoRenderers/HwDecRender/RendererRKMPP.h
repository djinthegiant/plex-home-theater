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

#include "cores/VideoPlayer/VideoRenderers/LinuxRendererGLES.h"
#include "DVDCodecs/Video/RKMPP.h"

// TODO: replace CLinuxRendererGLES
class CRendererRKMPP : public CLinuxRendererGLES
{
public:
  CRendererRKMPP();
  virtual ~CRendererRKMPP();

  bool RenderCapture(CRenderCapture* capture) override;
  void AddVideoPictureHW(DVDVideoPicture& picture, int index) override;
  void ReleaseBuffer(int idx) override;
  bool NeedBuffer(int idx) override;

  // Player functions
  bool IsGuiLayer() override;

  // Feature support
  bool Supports(ERENDERFEATURE feature) override;
  bool Supports(ESCALINGMETHOD method) override;

protected:

  // hooks for hw dec renderer
  bool LoadShadersHook() override;
  bool RenderHook(int index) override;
  int  GetImageHook(YV12Image* image, int source = AUTOSOURCE, bool readonly = false) override;
  bool RenderUpdateVideoHook(bool clear, DWORD flags = 0, DWORD alpha = 255) override;

private:
  CDVDDrmPrimeInfo* m_last_info;
};

#endif
