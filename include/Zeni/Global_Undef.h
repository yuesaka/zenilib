/* This file is part of the Zenipex Library.
* Copyleft (C) 2008 Mitchell Keith Bloch a.k.a. bazald
*
* The Zenipex Library is free software; you can redistribute it and/or 
* modify it under the terms of the GNU General Public License as 
* published by the Free Software Foundation; either version 2 of the 
* License, or (at your option) any later version.
*
* The Zenipex Library is distributed in the hope that it will be useful, 
* but WITHOUT ANY WARRANTY; without even the implied warranty of 
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
* General Public License for more details.
*
* You should have received a copy of the GNU General Public License 
* along with the Zenipex Library; if not, write to the Free Software 
* Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 
* 02110-1301 USA.
*
* As a special exception, you may use this file as part of a free software
* library without restriction.  Specifically, if other files instantiate
* templates or use macros or inline functions from this file, or you compile
* this file and link it with other files to produce an executable, this
* file does not by itself cause the resulting executable to be covered by
* the GNU General Public License.  This exception does not however
* invalidate any other reasons why the executable file might be covered by
* the GNU General Public License.
*/

#ifdef ZENI_GLOBAL_H
#undef ZENI_GLOBAL_H

// Camera.h
#undef ZENI_DEFAULT_CAMERA_POSITION
#undef ZENI_DEFAULT_CAMERA_ORIENTATION
#undef ZENI_DEFAULT_CAMERA_NEAR_CLIP
#undef ZENI_DEFAULT_CAMERA_FAR_CLIP
#undef ZENI_DEFAULT_CAMERA_FOV
#undef ZENI_DEFAULT_CAMERA_TUNNEL_VISION

// Fog.h
#undef ZENI_DEFAULT_FOG_COLOR
#undef ZENI_DEFAULT_FOG_DENSITY
#undef ZENI_DEFAULT_FOG_TYPE
#undef ZENI_DEFAULT_FOG_START
#undef ZENI_DEFAULT_FOG_END

// Material.h
#undef ZENI_DEFAULT_MATERIAL_DIFFUSE
#undef ZENI_DEFAULT_MATERIAL_SPECULAR
#undef ZENI_DEFAULT_MATERIAL_EMISSIVE
#undef ZENI_DEFAULT_MATERIAL_POWER

// Net.h
#undef ZENI_DEFAULT_CHUNK_SIZE
#undef ZENI_DEFAULT_CHUNK_SETS

// Sound_Source.h
#undef ZENI_DEFAULT_PITCH
#undef ZENI_DEFAULT_GAIN
#undef ZENI_DEFAULT_NEAR_CLAMP
#undef ZENI_DEFAULT_FAR_CLAMP
#undef ZENI_DEFAULT_ROLLOFF
#undef ZENI_DEFAULT_SOUND_PRIORITY

// Widget.h
#undef ZENI_DEFAULT_SLIDER_POSITION
#undef ZENI_DEFAULT_TAB_SPACES
#undef ZENI_DEFAULT_JUSTIFY
#undef ZENI_DEFAULT_TEXTBOX_EDITABLE

// Video.hxx
#undef ZENI_2D_NEAR
#undef ZENI_2D_FAR

// Video_DX9.hxx
#undef ZENI_STANDARD_DPI

// Gamestate.cpp
#undef ZENI_DEFAULT_II_MIN_CONFIDENCE
#undef ZENI_DEFAULT_II_MAX_CONFIDENCE

// Material.cpp
#undef ZENI_DIFFUSE_TO_SPECULAR

// Net_Primitives.cpp
#undef ZENI_SPRINTF_BUFFER_SIZE

// Thread.cpp
#undef NO_ERROR_STATUS
#undef ZENI_ERROR_STATUS
#undef OTHER_ERROR_STATUS

// Vertex_Buffer.cpp
#undef CLOSENESS_THRESHOLD_SQUARED
#undef ALIKENESS_THRESHOLD
#undef CLOSENESS_THRESHOLD

// Miscellaney
#undef END_OF_TIME
#undef INFINTESSIMAL
#undef INFINTESSIMAL_SQUARED

#endif