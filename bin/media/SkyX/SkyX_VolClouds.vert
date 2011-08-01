/**
--------------------------------------------------------------------------------
This source file is a modified part of SkyX (Copyright (C) 2009 Xavier Verguín
González <xavierverguin@hotmail.com>, <xavyiy@gmail.com>).

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

Author: Jose Luis Cercos Pita
--------------------------------------------------------------------------------
**/

// OUT
varying float  Opacity;
varying vec3   EyePixel;
varying float  Distance;
// UNIFORM
// UNIFORM
uniform mat4   uWorldViewProj;	// Don't needed, only for hlsl compatibility.
uniform vec3   uCameraPosition;
uniform float  uRadius;

void main()
{
    // Clip space position
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;

	// 3D coords
	gl_TexCoord[0]  = gl_MultiTexCoord0;
	gl_TexCoord[1]  = gl_MultiTexCoord1;
    // Noise coords
    gl_TexCoord[2]  = gl_MultiTexCoord2;
    // Opacity
    Opacity = gl_Color.w;
    // Eye to pixel vector
    EyePixel = normalize(gl_Vertex.xyz - uCameraPosition);
    
    // Distance in [0,1] range
    Distance = length(vec2(gl_Vertex.x, gl_Vertex.z)) / uRadius;
}

