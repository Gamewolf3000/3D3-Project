Texture2D sampleTexture : register(t0);
Texture2D shadowMap : register(t1);
sampler samplerState : register(s0);

struct vertexData
{
	float4 pos : SV_Position;
	float2 uv : TEXCOORDS;
	float4 colour : COLOUR;
};

struct PointLight
{
    float4 position;
    float4 colour;
    float4 rangeXRestPadding;
};

cbuffer LightData : register(b0)
{
    PointLight pointLights;
}

float4 main(vertexData data) : SV_Target
{
	float2 xyCoords = data.pos.xy;
	xyCoords += 1.0f;
	xyCoords *= 0.5f;

	//return float4(shadowMap.Sample(samplerState, xyCoords).xyz, 1.0f);

	//return float4(1.0f, 1.0f, 1.0f, 1.0f);
	return float4(sampleTexture.Sample(samplerState, data.uv).xyz, 1.0f) * shadowMap.Sample(samplerState, xyCoords).x;
}