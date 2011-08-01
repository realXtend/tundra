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

// IN
varying float Opacity;
varying vec3  EyePixel;
varying float Distance;
// UNIFORM
uniform float     uInterpolation;
uniform vec3      uSunDirection;
uniform vec3      uAmbientColor;
uniform vec3      uSunColor;
uniform vec4      uLightResponse;
uniform vec4      uAmbientFactors;
uniform sampler3D uDensity0;
uniform sampler3D uDensity1;
uniform sampler2D uNoise;

void main()
{    
    // x - Sun light power
    // y - Sun beta multiplier
    // z - Ambient color multiplier
    // w - Distance attenuation
	// uLightResponse = float4(0.25,0.2,1,0.1);
	
	// Ambient light factors
	// x - constant, y - linear, z - cuadratic, w - cubic
	// float4 uAmbientFactors = float4(0.4,1,1,1);

	vec3 Noise = texture2D(uNoise, gl_TexCoord[2].xy*5.0).xyz;
	vec3 Final3DCoord = gl_TexCoord[0].xyz+0.002575*(Noise-0.5)*2.0;
	Final3DCoord.z = clamp(Final3DCoord.z, 0.0, 1.0);
	
	vec3 Density0 = texture3D(uDensity0, Final3DCoord.xyz).xyz;
	vec3 Density1 = texture3D(uDensity1, Final3DCoord.xyz).xyz;
	vec3 Density  = Density0*(1.0-uInterpolation) + Density1*uInterpolation;
	
	vec3  finalcolor   = vec3(0.0,0.0,0.0);
	float finalOpacity = 0.0;
	
	if (Density.x > 0.0)
	{
	    float cos0 = clamp(dot(uSunDirection,EyePixel), 0.0, 1.0);
	    float c3=cos0*cos0;
	    c3*=c3;
	
		float Beta = c3*uLightResponse.y*(0.5+0.5*Density.y);

		float sunaccumulation = clamp( Beta+Density.y*uLightResponse.x+pow(Distance,1.5)*uLightResponse.w , 0.0, 1.0);
		float ambientaccumulation = clamp(uAmbientFactors.x + uAmbientFactors.y*gl_TexCoord[0].z + uAmbientFactors.z*pow(gl_TexCoord[0].z,2.0) + uAmbientFactors.w*pow(gl_TexCoord[0].z,3.0), 0.0, 1.0)*uLightResponse.z;
	    
		finalcolor = uAmbientColor*ambientaccumulation + uSunColor*sunaccumulation;
		finalOpacity = (1.0 - exp(-Density.x*(7.5-6.5*gl_TexCoord[0].z)))*Opacity;
	}
	
    gl_FragColor = vec4(finalcolor, finalOpacity);
}
