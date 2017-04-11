/*Applying Lightning*/
Texture2D normalTexture : register(t0);
Texture2D colour : register(t1);
Texture2D worldPos : register(t2);

struct lightData
{
    float4 lightPos;
    float4 lightColour;
    float4 rangeInXRestPadding;
};

cbuffer camera : register(b0)
{
    float4 cameraPos;
}

cbuffer lightBuffer : register(b1)
{
    int4 nrOfLightsInX;
    lightData lights[5];
}


float4 main(in float4 screenPos : SV_Position) : SV_TARGET
{
    int3 sampleIndices = int3(screenPos.xy, 0);

    float4 pixelColour = float4(colour.Load(sampleIndices).xyz, 1.0f);
    float3 normal = normalTexture.Load(sampleIndices).xyz;
    float4 totalColour = pixelColour;
    float3 posWorld = worldPos.Load(sampleIndices).xyz;
    for (int i = 0; i < nrOfLightsInX.x; i++)
    {
        /*Compute light here*/
        float3 lightVector = lights[i].lightPos.xyz - posWorld;
        float dist = length(lightVector);
        lightVector /= dist;
        float attenuation = max(0.0f, 1.0f - (dist / lights[i].rangeInXRestPadding.x));
        
        float NdotL = saturate(dot(normal, lightVector));

        float3 diffuse = NdotL * pixelColour.xyz * lights[i].lightColour.xyz;

        float3 V = cameraPos.xyz - posWorld;
        float3 H = normalize(lightVector + V);
        
        float3 specular = pow(saturate(dot(normal, H)), 2048.0f) * lights[i].lightColour.xyz * NdotL;


        totalColour.xyz += (diffuse + specular) * attenuation;
        //return float4(NdotL, 0.0f, 0.0f, 1.0f);

    }
    return float4(totalColour.xyz, 1.0f);
}