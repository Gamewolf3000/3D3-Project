struct VSIn
{
	float3 pos : POSITION;
	float2 uv : TEXCOORDS;
	float3 normal : NORMAL;
};

cbuffer colourData : register(b0)
{
	float4x4 worldMatrix : WORLDMATRIX;
	float4 colourData[6] : COLOUR;
}

cbuffer viewProjectionMatrixes : register(b1)
{
	float4x4 projection;
	float4x4 view;
}

struct vertexData
{
	float4 pos : SV_POSITION;
	float4 colour : COLOUR;
};

vertexData main(VSIn input, uint index : SV_VertexID)
{
	vertexData outPut;
	outPut.pos = float4(input.pos, 1.0f);
	outPut.colour = float4(1.0f, 1.0f, 1.0f, 1.0f);

	outPut.pos = mul(outPut.pos, mul(worldMatrix, mul(view, projection)));

	return outPut;
}