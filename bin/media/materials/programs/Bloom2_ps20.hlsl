//---------------------------------------------
// Bloom


float4 main(float2 texCoord: TEXCOORD0,
		uniform sampler RT: register(s0),
		uniform sampler Blur1: register(s1),
		uniform float OriginalImageWeight,
		uniform float BlurWeight
		) : COLOR {


	float4 sharp = tex2D(RT,   texCoord);
	float4 blur  = tex2D(Blur1, texCoord);

	return blur*BlurWeight+sharp*OriginalImageWeight;

}









