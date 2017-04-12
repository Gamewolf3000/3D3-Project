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

#define NROFLIGHTS 5

cbuffer LightData : register(b1)
{
	float4 posAndActiveW[NROFLIGHTS];
	float4 colourAndRangeW[NROFLIGHTS];
}

#define MAXNROFMESHES 1

struct MeshRelatedData
{
	float4x4 worldMatrix;
	int nrOfTrianglesInMesh;
	int3 padding;
};

cbuffer MeshMatrices : register(b2)
{
	MeshRelatedData data;
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

float rayVsMeshTriangle(float3 origin, float3 direction, int indexFirstPoint, unsigned int worldMatrixIndex)
{
	float3 points[3];
	points[0] = meshPositions[indexFirstPoint + 0].position;
	points[1] = meshPositions[indexFirstPoint + 1].position;
	points[2] = meshPositions[indexFirstPoint + 2].position;
	int sizeFactor = 2;
	points[0] = float3(-0.5f, -0.5f, 0.0f) * sizeFactor;
	points[1] = float3(0.5f, -0.5f, 0.0f) * sizeFactor;
	points[2] = float3(-0.0f, 0.5f, 0.0f) * sizeFactor;

	for (int i = 0; i < 3; i++)
	{
		points[i] = mul(points[i], data.worldMatrix);
	}



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

//float3 WorldPosFromDepth(float depth, float2 TexCoord) {
//	float z = depth;// *2.0 - 1.0;
//	float n = 0.1f;
//	float f = 100.0f;
//	float EZ = (n * f) / (f - z * (f - n));
//	float LZ  = z / (f - z * (f - n));
//	z = LZ;
//	//return z.xxx;
//
//	//z = 1.00100100f / (z - 1.0f);
//	//return z.xxx;
//
//	//float FarClipDistance = 100.0f;
//	//float NearClipDistance = 0.1f;
//	//float ProjectionA = FarClipDistance / (FarClipDistance - NearClipDistance);
//	//float ProjectionB = (-FarClipDistance * NearClipDistance) / (FarClipDistance - NearClipDistance);
//	//float linearDepth = ProjectionB / (depth - ProjectionA);
//
//	//z = linearDepth;
//
//	//return (LZ - linearDepth).xxx;
//
//	float4 clipSpacePosition = float4(TexCoord * 2.0f - 1.0f, z, 1.0f);
//	float4 viewSpacePosition = mul(clipSpacePosition, revProjMat);
//
//	// Perspective division
//	viewSpacePosition.xyz /= viewSpacePosition.w;
//
//	float4 worldSpacePosition = mul(viewSpacePosition, revViewMat);
//
//	return worldSpacePosition.xyz;
//}
//
//float3 ViewPosFromDepth(float depth, float2 TexCoord) {
//	float z = depth;// *2.0 - 1.0;
//	float n = 0.1f;
//	float f = 100.0f;
//	float EZ = (n * f) / (f - z * (f - n));
//	float LZ = z / (f - z * (f - n));
//	z = LZ;
//
//	float4 clipSpacePosition = float4(TexCoord * 2.0f - 1.0f, z, 1.0f);
//	float4 viewSpacePosition = mul(clipSpacePosition, revProjMat);
//
//	// Perspective division
//	viewSpacePosition.xyz /= viewSpacePosition.w;
//
//
//	float zView = projMat[3][2] / (depth - projMat[2][2]);
//
//	float4 ray;
//	float4 something = float4(TexCoord * 2.0f - 1.0f, 0.0f, 1.0f);
//
//	ray = mul(something, revProjMat);
//	ray /= ray.w;
//	ray /= ray.z;
//
//	return (zView * ray).xyz;
//
//
//	return viewSpacePosition.xyz;
//}

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
	float3 origin = float3(0.0f, 3.0f, 2.0f);
	float distance = length(posW.xyz - origin);
	float3 direction = normalize(posW.xyz - origin);

	float outputValue = 1.0f;
	int meshDataIndex = 0;
	int trianglesOfMeshTracedAgainst = 0;

	if (data.padding[0] == 3)
		map[threadID.xy] = 1.0f;
	else
		map[threadID.xy] = 0.0f;

	return;

	for (int i = 0; i < nrOfTriangles; i++)
	{

		float temp = rayVsMeshTriangle(origin, direction, i * 3, meshDataIndex);
		//float temp = raysVsSphere(origin, direction, float3(0, 0, 0), 0.5);

		if (temp < distance)
		{
			outputValue = 0.2f;
			break;
		}

		trianglesOfMeshTracedAgainst++;

		if (trianglesOfMeshTracedAgainst > data.nrOfTrianglesInMesh)
		{
			trianglesOfMeshTracedAgainst = 0;
			meshDataIndex++;
		}

	}

	map[threadID.xy] = outputValue;
	
}