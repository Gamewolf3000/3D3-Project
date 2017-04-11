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
	return data.writeValue;
}