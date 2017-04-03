/*Applying Lightning*/
Texture2D normalTexture : register(t0);
Texture2D colour : register(t1);
Texture2D worldPos : register(t2);

cbuffer camera : register(b0)
{
    float4 cameraPos;
}

float4 main(in float4 screenPos : SV_Position) : SV_TARGET
{
    int3 sampleIndices = int3(screenPos.xy, 0);
    return float4(normalTexture.Load(sampleIndices));
}