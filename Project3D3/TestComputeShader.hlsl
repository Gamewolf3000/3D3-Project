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
}

#define NROFLIGHTS 5

cbuffer LightData : register(b1)
{
	float4 posAndActiveW[NROFLIGHTS];
	float4 colourAndRangeW[NROFLIGHTS];
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

[numthreads(16, 16, 1)]
void main( uint3 threadID : SV_DispatchThreadID )
{
	float closestDistance = 1500;
	float2 xyCoords = threadID.xy / float2(1280, 720);
	xyCoords *= 2.0f;
	xyCoords -= 1.0f;
	xyCoords.y *= -1.0f;

	float3 posV = mul(float4(xyCoords, 0.f, 1.0f), revProjMat).xyz;
	float4 posW = mul(posV, revViewMat);
	posW /= posW.w;

	float3 origin = camPos;
	float3 direction = (mul(float4(posV.xyz, 1.0f), revViewMat)).xyz;
	direction = normalize(direction - origin);

	for (int i = 0; i < nrOfTriangles; i++)
	{
		float temp = rayVsMeshTriangle(origin, direction, i * 3);

		if (temp < closestDistance)
			closestDistance = temp;
	}

	
	map[threadID.xy] = closestDistance / 100;

	//map[threadID.xy] = (1 - length(threadID.xy - float2(640, 360)) / 640.0f);
	//map[threadID.xy] = depth[threadID.xy];
	//map[threadID.xy] = meshPositions[threadID.x % (NROFTRIANGLES * 3)].position.x;
}