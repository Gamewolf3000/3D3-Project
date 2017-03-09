Texture2D sampleTexture : register(t0);
sampler samplerState : register(s0);

struct vertexData
{
	float4 pos : SV_Position;
	float2 uv : TEXCOORDS;
	float4 colour : COLOUR;
};

float4 main(vertexData data) : SV_Target
{
	//return float4(1.0f, 1.0f, 1.0f, 1.0f);
	return float4(sampleTexture.Sample(samplerState, data.uv).xyz, 1.0f);
}