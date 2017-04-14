struct VertexData
{
	float3 position;
	float2 uv;
	float3 normal;

	float3 tangent;
	float3 bitangent;
};

RWTexture2D<float> map : register(u0);
Texture2D<float> depth : register(t0);
StructuredBuffer<VertexData> meshPositions : register(t1);

cbuffer FrameData : register(b0)
{
	float4x4 revProjMat;
	float4x4 revViewMat;

	int nrOfTriangles;
	float3 camPos;

	unsigned int windowWidth;
	unsigned int windowHeight;

	float2 pad;

	float4x4 viewMat;
}

#define NROFLIGHTS 100

struct LightData
{
	float4 position_range;
	float4 colour;
};

cbuffer Lights : register(b1)
{
	int4 nrOfLightsAndPadding;
	LightData lightStructs[NROFLIGHTS];
}


float raysVsSphere(float3 origin, float3 direction, float3 sphereOrigin, float sphereRadius)
{
	bool returnBool = true;
	float3 l = sphereOrigin - origin;
	float s = dot(l, direction);
	float l2 = dot(l, l);
	float r2 = sphereRadius * sphereRadius;
	if (l2 < r2) /* Check if we're inside the sphere, for debugging purposes*/
		returnBool = false;
	if (s < 0.0f && l2 > r2)
		returnBool = false;

	float m = l2 - s*s;
	if (m > r2)
		returnBool = false;

	float q = sqrt(r2 - m);
	int sign = 1.0;
	if (l2 > r2)
		sign = -1.0;

	if (returnBool)
		return s + sign*q;
	else
		return 1550;
}

float rayVsMeshTriangle(float3 origin, float3 direction, int indexFirstPoint)
{
	float3 points[3];
	points[0] = meshPositions[indexFirstPoint + 0].position;
	points[1] = meshPositions[indexFirstPoint + 1].position;
	points[2] = meshPositions[indexFirstPoint + 2].position;


	float3 e1 = points[1].xyz - points[0].xyz;
	float3 e2 = points[2].xyz - points[0].xyz;

	float3 normal = normalize(cross(e1, e2));

	float visible = dot(normal, -direction);

	if (visible < 0)
		return 1500;


	float3 q = cross(direction, e2);
	float a = dot(e1, q);

	if (a > -0.00001 && a < 0.00001) /*Abs*/
	{
		return 1500;
	}
	float f = 1 / a;
	float3 s = origin - points[0].xyz;
	float u = f*(dot(s, q));

	if (u < 0.0)
		return 1500;

	float3 r = cross(s, e1);
	float v = f*(dot(direction, r));
	if (v < 0.0 || u + v > 1.0)
		return 1500;

	float t = f*(dot(e2, r));

	if (t < 0.0f)
		return 1500;



	return t;
}

float3 WorldPosFromDepth(float depth, float2 TexCoord) {
	float z = depth;// *2.0 - 1.0;

	float4 clipSpacePosition = float4(TexCoord * 2.0f - 1.0f, z, 1.0f);
	float4 viewSpacePosition = mul(clipSpacePosition, revProjMat);

	// Perspective division
	viewSpacePosition.xyz /= viewSpacePosition.w;

	//return viewSpacePosition.xyz;
	
	//viewSpacePosition.w = 1;

	float4 worldSpacePosition = mul(float4(viewSpacePosition.xyz, 1), revViewMat);

	return worldSpacePosition.xyz;
}

[numthreads(16, 16, 1)] // matches 1280x720 with the dispatch call on the cpu
void main( uint3 threadID : SV_DispatchThreadID )
{
	float2 xyCoords = threadID.xy / float2(windowWidth, windowHeight);
	xyCoords.y = 1 - xyCoords.y;
	float depthValue = depth[threadID.xy];

	float3 posW = WorldPosFromDepth(depthValue, xyCoords);
	float outputValue = 0.2f;
	

	for (int lights = 0; lights < nrOfLightsAndPadding.x; lights++)
	{
		bool foundCloser = false;
		float3 origin = lightStructs[lights].position_range.xyz;
		float distance = length(posW.xyz - origin);
		if (distance > lightStructs[lights].position_range.w)
			continue;

		float3 direction = normalize(posW.xyz - origin);


		for (int i = 0; i < nrOfTriangles; i++)
		{

			float temp = rayVsMeshTriangle(origin, direction, i * 3);
			//float temp = raysVsSphere(origin, direction, float3(0, 0, 0), 0.5);

			if (temp < distance)
			{
				foundCloser = true;
				break;
				//outputValue -= 0.25f;
			}
		}

		if (!foundCloser)
		{
			outputValue = 1.0f;
			break;
		}
	}
	map[threadID.xy] = outputValue;
	
}