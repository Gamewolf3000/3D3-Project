/*COMMENCE THE RASTERIZATION!*/

struct VSIn
{
    float3 pos : POSITION;
    float2 uv : TEXCOORDS;
    float3 normal : NORMAL;
};

cbuffer perObjectMatrix : register(b0)
{
    float4x4 worldMatrix : WORLDMATRIX;
}

cbuffer viewProjectionMatrixes : register(b1)
{
    float4x4 projection;
    float4x4 view;
}

struct vertexData
{
    float4 pos : SV_POSITION;
    float4 wPos : WORLDPOS;
    float4 normal : NORMAL;
    float2 uv : TEXCOORDS;
};

vertexData main(VSIn input, uint index : SV_VertexID)
{
    vertexData outPut;
    
    outPut.uv = input.uv;
    outPut.normal = mul(float4(input.normal, 0.0f), worldMatrix);
    outPut.normal = normalize(float4(outPut.normal.xyz, 0.0f));
    //outPut.normal = float4(input.normal, 0.0f);
    outPut.wPos = mul(float4(input.pos, 1.0f), worldMatrix);
    outPut.wPos /= outPut.wPos.w;
    outPut.pos = mul(float4(input.pos, 1.0f), mul(worldMatrix, mul(view, projection)));
    //outPut.pos /= outPut.pos.w;

    return outPut;
}