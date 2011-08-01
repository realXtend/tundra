/*
--------------------------------------------------------------------------------
This source file is part of SkyX.
Visit ---

Copyright (C) 2009 Xavier Verguín González <xavierverguin@hotmail.com>
                                           <xavyiy@gmail.com>

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
--------------------------------------------------------------------------------

--------------------------------------------------------------------------------
GLSL shaders added by Jose Luis Cercos Pita.
--------------------------------------------------------------------------------
*/

// --------------------- SkyX skydome material ------------------------

// IN
varying vec3  RayleighColor;
varying vec3  MieColor;
varying vec3  Direction;
varying float Opacity;
varying float Height;
// UNIFORM
uniform float uTime;
uniform vec3 uLightDir;
// Phase function
uniform float  uG;
uniform float  uG2;
// Exposure
uniform float uExposure;
uniform sampler2D uStarfield;

void main()
{
    float cos = dot(uLightDir, Direction) / length(Direction);
	float cos2 = cos*cos;
	
	float rayleighPhase = 0.75 * (1.0 + 0.5*cos2);
	
	float miePhase = 1.5 * ((1.0 - uG2) / (2.0 + uG2)) * // <<< TODO
					 (1.0 + cos2) / pow(1.0 + uG2 - 2.0 * uG * cos, 1.5);

#ifdef LDR
	gl_FragColor = vec4((1.0 - exp(-uExposure * (rayleighPhase * RayleighColor + miePhase * MieColor))), Opacity);
#else // HDR
    gl_FragColor = vec4(rayleighPhase * RayleighColor + miePhase * MieColor, Opacity);
#endif // LDR
	
	// For night rendering
	float nightmult = clamp(1.0 - max(gl_FragColor.x, max(gl_FragColor.y, gl_FragColor.z))*10.0, 0.0, 1.0);
	
#ifdef STARFIELD
	gl_FragColor.xyz += nightmult *(vec3(0.05, 0.05, 0.1)*(2.0-0.75*clamp(-uLightDir.y, 0.0, 1.0))*pow(Height,3.0) + texture2D(uStarfield, gl_TexCoord[0].xy+uTime).xyz*(0.35 + clamp(-uLightDir.y*0.45, 0.0, 1.0))); 
#else // NO STARFIELD
	gl_FragColor.xyz += nightmult *(vec3(0.05, 0.05, 0.1)*(2.0-0.75*clamp(-uLightDir.y, 0.0, 1.0))*pow(Height,3.0)); 
#endif // STARFIELD
}
