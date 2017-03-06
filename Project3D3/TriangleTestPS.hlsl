
struct vertexData
{
    float4 pos : SV_Position;
    float4 colour : COLOUR;
};

float4 main(vertexData data) : SV_TARGET
{
	return data.colour;
}