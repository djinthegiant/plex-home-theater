/*
 *      Copyright (C) 2005-2013 Team XBMC
 *      http://xbmc.org
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
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#pragma once

#include "EGL/egl.h"
#include "gbm.h"

class CGLContextEGL
{
public:
  CGLContextEGL();
  virtual ~CGLContextEGL();

  bool CreateDisplay(gbm_device* connection,
                     EGLint renderable_type,
                     EGLint rendering_api);

  bool CreateSurface(gbm_surface* surface);
  bool CreateContext();
  bool BindContext();
  void Destroy();
  void Detach();
  void SwapBuffers();

  EGLDisplay m_eglDisplay;
  EGLSurface m_eglSurface;
  EGLContext m_eglContext;
  EGLConfig m_eglConfig;
};
