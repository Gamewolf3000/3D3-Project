Texture2D sampleTexture : register(t0);
Texture2D shadowMap : register(t1);
sampler samplerState : register(s0);

struct vertexData
{
	float4 pos : SV_Position;
	float writeValue : WRITE_VALUE;
};

float main(vertexData data) : SV_Depth
{
	float2 xyCoords = data.pos.xy / float2(1280,720);
	return xyCoords.x;
	return data.writeValue;
}