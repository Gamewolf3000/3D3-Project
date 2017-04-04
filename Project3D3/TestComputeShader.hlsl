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
SamplerState sampState;

cbuffer FrameData : register(b0)
{
	float4x4 revProjMat;
	float4x4 revViewMat;

	int nrOfTriangles;
	float3 camPos;

	unsigned int windowWidth;
	unsigned int windowHeight;
}

#define NROFLIGHTS 5

cbuffer LightData : register(b1)
{
	float4 posAndActiveW[NROFLIGHTS];
	float4 colourAndRangeW[NROFLIGHTS];
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
	int sizeFactor = 2;
	points[0] = float3(0.5f, -0.5f, 0.0f) * sizeFactor;
	points[1] = float3(-0.5f, -0.5f, 0.0f) * sizeFactor;
	points[2] = float3(0.0f, 0.5f, 0.0f) * sizeFactor;


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
	float z = depth * 2.0 - 1.0;

	float4 clipSpacePosition = float4(TexCoord * 2.0 - 1.0, z, 1.0);
	float4 viewSpacePosition = mul(clipSpacePosition, revProjMat);

	// Perspective division
	viewSpacePosition /= viewSpacePosition.w;

	float4 worldSpacePosition = mul(viewSpacePosition, revViewMat);

	return worldSpacePosition.xyz;
}

[numthreads(16, 16, 1)]
void main( uint3 threadID : SV_DispatchThreadID )
{
	float2 xyCoords = threadID.xy / float2(windowWidth, windowHeight);
	xyCoords.y *= -1.0f;
	float depthValue = depth.SampleLevel(sampState, xyCoords, 0).x;

	float3 posW = WorldPosFromDepth(depthValue, xyCoords);

	//posW /= posW.w;

	float3 origin = float3(0.0f, 0.0f, -2.5f);
	//float3 direction = (mul(float4(posV.xyz, 1.0f), revViewMat)).xyz;
	float distance = length(posW.xyz - origin);
	float3 direction = normalize(posW.xyz - origin);

	map[threadID.xy] = 1;

	for (int i = 0; i < nrOfTriangles; i++)
	{
		//float temp = rayVsMeshTriangle(origin, direction, i * 3);
		float temp = raysVsSphere(origin, direction, float3(0, 0, 0), 1);

		if (temp < distance)
			map[threadID.xy] = 0.2f;
	}

	//map[threadID.xy] = distance;

	//for (int l = 0; l < NROFLIGHTS; l++)
	//{
	//	//origin = float3(0.0f, 0.0f, 0.0f);
	//	//direction = float3(0.0f, 0.0f, 1.0f);
	//	//direction = normalize(posW - origin);
	//	closestDistance = length(posW.xyz - origin);

	//	for (int i = 0; i < nrOfTriangles; i++)
	//	{
	//		//float temp = rayVsMeshTriangle(origin, direction, i * 3);
	//		float temp = raysVsSphere(origin, direction, float3(0.0f, 0.0f, 10.0f), 2.0f);

	//		if (temp < 1550.0f)/*temp < 0.0f || temp > 0.0f || temp == 0.0f || isinf(temp)*/
	//		{
	//			map[threadID.xy] = 1;
	//			//closestDistance = -temp;
	//		}
	//		else
	//		{
	//			map[threadID.xy] = posW.z;
	//		}
	//	}
	//}
	//map[threadID.xy] = posW.x;
	/*if(closestDistance == 1500)
		map[threadID.xy] = 0.5;*/
	//else
	//	map[threadID.xy] = 0;

	//map[threadID.xy] = closestDistance;
	
	//map[threadID.xy] = closestDistance / 4;

	//map[threadID.xy] = (1 - length(threadID.xy - float2(640, 360)) / 640.0f);
	//map[threadID.xy] = depth[threadID.xy];
	//map[threadID.xy] = meshPositions[threadID.x % (NROFTRIANGLES * 3)].position.x;
}