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

This program is distributed in the hope that it will be useful, but WITHvarying
ANY WARRANTY; withvarying even the implied warranty of MERCHANTABILITY or FITNESS
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

// OUT
varying vec3  RayleighColor;
varying vec3  MieColor;
varying vec3  Direction;
varying float Opacity;
varying float Height;
// UNIFORM
uniform mat4 uWorldViewProj;
// Global information
uniform vec3 uLightDir;
// Position information
uniform vec3 uCameraPos;
uniform vec3 uInvWaveLength;
uniform float  uInnerRadius;
// Scattering parameters
uniform float  uKrESun; // Kr * ESun
uniform float  uKmESun; // Km * ESun
uniform float  uKr4PI;  // Kr * 4 * PI
uniform float  uKm4PI;  // Km * 4 * PI
// Atmosphere properties
uniform float uScale;               // 1 / (varyingerRadius - innerRadius)
uniform float uScaleDepth;          // Where the average atmosphere density is found
uniform float uScaleOverScaleDepth; // Scale / ScaleDepth
// Number of samples
uniform int   uNumberOfSamples;
uniform float uSamples;

float scale(float cos, float uScaleDepth)
{
	float x = 1.0 - cos;
	return uScaleDepth * exp(-0.00287 + x*(0.459 + x*(3.83 + x*(-6.80 + x*5.25))));
}

void main()
{
    // Clip space position
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;

	vec3 v3Pos = gl_MultiTexCoord0.xyz/gl_MultiTexCoord0.w;
	v3Pos.y += uInnerRadius;

    vec3 v3Ray = v3Pos - uCameraPos;
	float fFar = length(v3Ray);
	v3Ray /= fFar;
	
	// Calculate the ray's starting position, then calculate its scattering offset
	vec3 v3Start = uCameraPos;
	float fHeight = uCameraPos.y;
	float fStartAngle = dot(v3Ray, v3Start) / fHeight;
	// NOTE: fDepth is not pased as parameter(like a constant) to avoid the little precission issue (Apreciable)
	float fDepth = exp(uScaleOverScaleDepth * (uInnerRadius - uCameraPos.y));
	float fStartOffset = fDepth * scale(fStartAngle, uScaleDepth);

    // Init loop variables
	float fSampleLength = fFar / uSamples;
	float fScaledLength = fSampleLength * uScale;
	vec3 v3SampleRay = v3Ray * fSampleLength;
	vec3 v3SamplePoint = v3Start + v3SampleRay * 0.5;
	
	// Loop the ray
	vec3 color = vec3(0.0, 0.0, 0.0);
	for (int i=0; i < uNumberOfSamples; i++)
	{
		float fHeight = length(v3SamplePoint);
		float fDepth = exp(uScaleOverScaleDepth * (uInnerRadius-fHeight));
		
		float fLightAngle = dot(uLightDir, v3SamplePoint) / fHeight;
		float fCameraAngle = dot(v3Ray, v3SamplePoint) / fHeight;
		
		float fScatter = (fStartOffset + fDepth*(scale(fLightAngle, uScaleDepth) - scale(fCameraAngle, uScaleDepth)));
		vec3 v3Attenuate = exp(-fScatter * (uInvWaveLength * uKr4PI + uKm4PI)); // <<< TODO
		
		// Accumulate color
		v3Attenuate *= (fDepth * fScaledLength);
		color += v3Attenuate;
		
		// Next sample point
		v3SamplePoint += v3SampleRay;
	}

    // varyingputs
    RayleighColor = color * (uInvWaveLength * uKrESun); // TODO <--- parameter
    MieColor      = color * uKmESun; // TODO <--- *uInvMieWaveLength
    Direction     = uCameraPos - v3Pos;
	gl_TexCoord[0] = gl_MultiTexCoord1;
    Opacity = gl_Color.w;
    Height = 1.0-gl_MultiTexCoord0.y/gl_MultiTexCoord0.w;
}

