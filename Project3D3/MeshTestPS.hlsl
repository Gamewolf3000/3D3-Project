
struct vertexData
{
	float4 pos : SV_Position;
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
	return data.colour;
}