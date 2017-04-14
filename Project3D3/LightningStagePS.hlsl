/*Applying Lightning*/
Texture2D normalTexture : register(t0);
Texture2D colour : register(t1);
Texture2D worldPos : register(t2);

struct lightData
{
    float4 lightPosxyzRangeW;
    float4 lightColour;
};

cbuffer camera : register(b0)
{
    float4 cameraPos;
}

cbuffer lightBuffer : register(b1)
{
    int4 nrOfLightsInX;
    lightData lights[100];
}


float4 main(in float4 screenPos : SV_Position) : SV_TARGET
{
    int3 sampleIndices = int3(screenPos.xy, 0);

    float4 pixelColour = float4(colour.Load(sampleIndices).xyz, 1.0f);
    float3 normal = normalTexture.Load(sampleIndices).xyz;
    normalTexture.Load(sampleIndices).xyz;
    float4 totalColour = pixelColour*0.05;
    float3 posWorld = worldPos.Load(sampleIndices).xyz;
    for (int i = 0; i < nrOfLightsInX.x; i++)
    {
        /*Compute light here*/
        float3 lightVector = lights[i].lightPosxyzRangeW.xyz - posWorld;
        float dist = length(lightVector);
        
        float r = lights[i].lightPosxyzRangeW.w;
        
        float d = max(dist - r, 0);

        lightVector /= dist;
        
        float denom = d / r + 1; 

        float attenuation = max(0,1 / (denom * denom));
        
        float NdotL = saturate(dot(normal, lightVector));
        
        float3 diffuse = NdotL * pixelColour.xyz * lights[i].lightColour.xyz;

        float3 V = cameraPos.xyz - posWorld;
        float3 H = normalize(lightVector + V);
        
        float3 specular = pow(saturate(dot(normal, H)), 0.25f) * lights[i].lightColour.xyz * NdotL;


        totalColour.xyz += (diffuse + specular) * attenuation;

    }
    return float4(totalColour.xyz, 1.0f);
}