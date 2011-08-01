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

// ------------------------- SkyX clouds -----------------------------
// IN
varying vec3 Position;
// UNIFORM
uniform float   uExposure;
// Sun information
uniform vec3    uSunPosition;
uniform vec3    uSunColor;
// Main cloud layer parameters
uniform float   uHeight;
uniform float   uTime;
uniform float   uScale;
uniform vec2    uWindDirection;
// Advanced cloud layer parameters
uniform float   uCloudLayerHeightVolume; // 0.25
uniform float   uCloudLayerVolumetricDisplacement; // 0.01
uniform vec3    uAmbientLuminosity; // 0.55 0.55 0.55
uniform float   uNormalMultiplier; // 2
uniform float   uDetailAttenuation; // 0.45
uniform float   uDistanceAttenuation; // 0.05
uniform sampler2D uCloud;
uniform sampler2D uNormal;
uniform sampler2D uDensity;

void main()
{
    // Get the cloud pixel lenght on the projected plane
    float vh = uHeight / Position.y;
    // Get the 3D position of the cloud pixel
    vec3 CloudPosition = Position * vh;
    
    // Get texture coords
    vec2 TexCoord = CloudPosition.xz*uScale;
    float Density = texture2D(uDensity, TexCoord+uTime*uWindDirection*0.25).x;
    vec3 Normal   = -(2.0*texture2D(uNormal, TexCoord+uTime*uWindDirection*0.25).xyz-1.0);
    Normal.zy = Normal.yz;
 
    ///------------ Volumetric effect:
    float CloudLayerHeightVolume = uCloudLayerHeightVolume*Position.y;
    float CloudLayerVolumetricDisplacement = uCloudLayerVolumetricDisplacement*Position.y;
    vec3 iNewPosition = normalize(Position + CloudLayerVolumetricDisplacement*vec3(Normal.x,0.0,Normal.z));
    vh = (uHeight+uHeight*(1.0-Density)*CloudLayerHeightVolume) / iNewPosition.y;
    CloudPosition = iNewPosition * vh;
    TexCoord = CloudPosition.xz*uScale;
    Density = texture2D(uDensity, TexCoord+uTime*uWindDirection*0.25).x;
    ///------------
    
    vec3 SunToPixel = CloudPosition - uSunPosition;
    
    float CloudDetail = texture2D(uCloud, TexCoord-uTime*uWindDirection*0.25).x;
    Normal    = -(2.0*texture2D(uNormal, TexCoord+uTime*uWindDirection*0.25).xyz-1.0);
    Normal.zy = Normal.yz;
    Normal.xz*=uNormalMultiplier;

    vec3 PixelColor = vec3(0.0,0.0,0.0);
    
    // AMBIENT addition
    PixelColor += uAmbientLuminosity;
    
    // SUN addition 
    PixelColor  += uSunColor*clamp(dot(-normalize(Normal), normalize(uSunPosition)), 0.0, 1.0);
    
    // FINAL colour
    float Alpha = Density * clamp(10.0*clamp(-uDistanceAttenuation+Position.y, 0.0, 1.0), 0.0, 1.0);
    
    gl_FragColor = vec4(PixelColor*(1.0-Density*0.35), Alpha*clamp(1.0-CloudDetail*uDetailAttenuation, 0.0, 1.0));
    
#ifdef LDR
    gl_FragColor.xyz = vec3(1.0 - exp(-uExposure * gl_FragColor.xyz));
#endif // LDR
}
