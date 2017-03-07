/*No input data as of now!*/

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

vertexData main(uint index : SV_VertexID)
{
    vertexData outPut;
    outPut.pos = float4(0.0f, 0.0f, 0.0f, 1.0f);
    outPut.colour = colourData[index];

    switch(index)
    {
        case 0:
            outPut.pos = float4(-1.0f, 1.0f, 0.0f, 1.0f);
            break;
        case 1:
            outPut.pos = float4(1.f, -1.0f, 0.0f, 1.0f);
            break;
        case 2:
            outPut.pos = float4(-1.0f, -1.0f, 0.0f, 1.0f);
            break;
        case 3:
            outPut.pos = float4(-1.0f, 1.0f, 0.0f, 1.0f);
            break;
        case 4:
            outPut.pos = float4(1.0f, 1.0f, 0.0f, 1.0f);
            break;
        case 5:
            outPut.pos = float4(1.f, -1.0f, 0.0f, 1.0f);
            break;
    }
    outPut.pos = mul(outPut.pos, mul(worldMatrix, mul(view, projection)));

	return outPut;
}