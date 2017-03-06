#include "OBJLoader.h"

using namespace std;
using namespace DirectX;

OBJLoader::OBJLoader(wchar_t directory[200])
{
	std::wstring temp(directory);
	modelDirectory = new std::string(temp.begin(), temp.end());
}

OBJLoader::~OBJLoader()
{
	delete modelDirectory;
}

MeshData OBJLoader::LoadOBJFile(std::string fileName)
{

	std::ifstream file;
	std::string temp;
	std::string::size_type sz;
	MeshData mesh;

	CountData(fileName, mesh); // Count positions, uv coords, normals and faces
	//string mName = getMtlName(filename); // Get the mtl file name
	//loadMTL(gDevice, gDeviceContext, mName); // Load the texture from the mtl file

	char data, last = NULL, failsafe = NULL;


	file.open(fileName);
	file.get(data);
	while (!file.eof())
	{
		if (data == ' ' && last == 'v') // Read all position data
		{
			for (int i = 0; i < vCount; i++)
			{
				file.get(data);

				while (data != ' ')
				{
					temp += data;
					file.get(data);
				}


				vertices[i].x = stof(temp, &sz);
				temp = "";



				file.get(data);

				while (data != ' ')
				{
					temp += data;
					file.get(data);
				}

				vertices[i].y = stof(temp, &sz);
				temp = "";



				file.get(data);

				while (data != '\n')
				{
					temp += data;
					file.get(data);
				}

				vertices[i].z = stof(temp, &sz);
				temp = "";

				file.get(data);

				if (data == 'g')
					while (data != 'v')
					{
						file.get(data);
					}

				//while (data != 'v')
				//{
				//	file.get(data);
				//}

				file.get(data);


			}
		}

		//---------------------------------------------------------------------------------------------------------------------------------

		if (data == 't' && last == 'v') // Read all UV data
		{
			for (int i = 0; i < vtCount; i++)
			{
				file.get(data);
				file.get(data);

				while (data != ' ')
				{
					temp += data;
					file.get(data);
				}

				uvCoords[i].x = stof(temp, &sz);// = stof(temp, &sz);
				temp = "";



				file.get(data);

				while (data != '\n')
				{
					temp += data;
					file.get(data);
				}

				uvCoords[i].y = 1 - stof(temp, &sz);// = 1 - stof(temp, &sz);
				temp = "";

				file.get(data);
				file.get(data);


			}
		}

		//---------------------------------------------------------------------------------------------------------

		if (data == 'n' && last == 'v') // Read all Normals data
		{
			for (int i = 0; i < vnCount; i++)
			{
				file.get(data);
				file.get(data);

				while (data != ' ')
				{
					temp += data;
					file.get(data);
				}

				normals[i].x = stof(temp, &sz); // = stof(temp, &sz);
				temp = "";



				file.get(data);

				while (data != ' ')
				{
					temp += data;
					file.get(data);
				}

				normals[i].y = stof(temp, &sz);// = stof(temp, &sz);
				temp = "";



				file.get(data);

				while (data != '\n')
				{
					temp += data;
					file.get(data);
				}

				normals[i].z = stof(temp, &sz);// = stof(temp, &sz);
				temp = "";

				file.get(data);
				file.get(data);

			}
		}

		//---------------------------------------------------------------------------------------------------------------------------

		if (data == ' ' && last == 'f') // Read all face data and put everything together
		{
			int test = 0;
			int index = 0;

			for (int i = 0; i < iCount; i++)
			{
				int count;
				file.get(data);


				for (int j = 0; j < 3; j++)
				{

					while (data != '/')
					{
						temp += data;
						file.get(data);
					}

					file.get(data);
					index = stoi(temp, &sz);
					temp = "";

					count = iCount * 3 - 1 - i * 3 - j;

					/*modelData[count].x = vertices[index - 1].x;
					modelData[count].y = vertices[index - 1].y;
					modelData[count].z = vertices[index - 1].z;*/

					mesh.vertexBuffer[count].position.x = vertices[index - 1].x; // THIS ALSO NEEDS TO CHANGE TO ALLOW SUBMESHES
					mesh.vertexBuffer[count].position.y = vertices[index - 1].y;
					mesh.vertexBuffer[count].position.z = vertices[index - 1].z;

					while (data != '/')
					{
						temp += data;
						file.get(data);
					}

					file.get(data);
					index = stoi(temp, &sz);
					temp = "";

					/*modelData[count].u = uvCoords[index - 1].x;
					modelData[count].v = uvCoords[index - 1].y;*/

					mesh.vertexBuffer[count].uv.x = uvCoords[index - 1].x;
					mesh.vertexBuffer[count].uv.y = uvCoords[index - 1].y;

					while (data != ' ' && data != '\n')
					{
						temp += data;
						file.get(data);
					}

					file.get(data);
					index = stoi(temp, &sz);
					temp = "";

					/*modelData[count].nx = normals[index - 1].x;
					modelData[count].ny = normals[index - 1].y;
					modelData[count].nz = normals[index - 1].z;*/

					mesh.vertexBuffer[count].normal.x = normals[index - 1].x;
					mesh.vertexBuffer[count].normal.y = normals[index - 1].y;
					mesh.vertexBuffer[count].normal.z = normals[index - 1].z;

				}



				test++;

				while (data != 'f' && !file.eof())
				{
					failsafe = data;
					file.get(data);

					if (data == 'f' && failsafe != '\n')
						while (!(data == 'f' && failsafe == '\n'))
						{
							failsafe = data;
							file.get(data);
						}

				}

				file.get(data);

			}
		}


		last = data;
		file.get(data);
	}

	mesh.indexBuffer = new unsigned int[iCount * 3];
	
	for (int i = 0; i < iCount * 3; i++)
	{
		mesh.indexBuffer[i] = iCount * 3 - 1 - i;
	}

	//Calculate Tangent and BiTangent

	for (int i = 0; i < iCount * 3; i += 3)
	{
		StandardVertex vert0 = mesh.vertexBuffer[i];
		StandardVertex vert1 = mesh.vertexBuffer[i + 1];
		StandardVertex vert2 = mesh.vertexBuffer[i + 2];

		Float3D edge1 = Float3DMinus(vert1.position, vert0.position);
		Float3D edge2 = Float3DMinus(vert2.position, vert0.position);

		float deltaU1 = vert1.uv.x - vert0.uv.x;
		float deltaV1 = vert1.uv.y - vert0.uv.y;
		float deltaU2 = vert2.uv.x - vert0.uv.x;
		float deltaV2 = vert2.uv.y - vert0.uv.y;

		float f = 1.0f / (deltaU1 * deltaV2 - deltaU2 * deltaV1);

		Float3D tangent, bitangent;

		tangent.x = f * (deltaV2 * edge1.x - deltaV1 * edge2.x);
		tangent.y = f * (deltaV2 * edge1.y - deltaV1 * edge2.y);
		tangent.z = f * (deltaV2 * edge1.z - deltaV1 * edge2.z);

		float length = sqrt((tangent.x * tangent.x) + (tangent.y * tangent.y) + (tangent.z * tangent.z));
		XMFLOAT3 temp;

		temp.x = tangent.x / length;
		temp.y = tangent.y / length;
		temp.z = tangent.z / length;

		tangent = temp;

		bitangent.x = f * (-deltaU2 * edge1.x - deltaU1 * edge2.x);
		bitangent.y = f * (-deltaU2 * edge1.y - deltaU1 * edge2.y);
		bitangent.z = f * (-deltaU2 * edge1.z - deltaU1 * edge2.z);

		length = sqrt((bitangent.x * bitangent.x) + (bitangent.y * bitangent.y) + (bitangent.z * bitangent.z));

		temp.x = bitangent.x / length;
		temp.y = bitangent.y / length;
		temp.z = bitangent.z / length;

		bitangent = temp;

		mesh.vertexBuffer[i].tangent = Float3DPlus(mesh.vertexBuffer[i].tangent, tangent);
		mesh.vertexBuffer[i + 1].tangent = Float3DPlus(mesh.vertexBuffer[i + 1].tangent, tangent);
		mesh.vertexBuffer[i + 2].tangent = Float3DPlus(mesh.vertexBuffer[i + 2].tangent, tangent);

		mesh.vertexBuffer[i].bitangent = Float3DPlus(mesh.vertexBuffer[i].bitangent, bitangent);
		mesh.vertexBuffer[i + 1].bitangent = Float3DPlus(mesh.vertexBuffer[i + 1].bitangent, bitangent);
		mesh.vertexBuffer[i + 2].bitangent = Float3DPlus(mesh.vertexBuffer[i + 2].bitangent, bitangent);


		//float test = mesh.vertexBuffer[i].tangent.x * mesh.vertexBuffer[i].normal.x + mesh.vertexBuffer[i].tangent.y * mesh.vertexBuffer[i].normal.y + mesh.vertexBuffer[i].tangent.z * mesh.vertexBuffer[i].normal.z;
		//Float3D gram = Float3DMinus(tangent, Float3D(mesh.vertexBuffer[i].normal.x * test, mesh.vertexBuffer[i].normal.y * test, mesh.vertexBuffer[i].normal.z * test));
		//test = gram.x * mesh.vertexBuffer[i].normal.x + gram.y * mesh.vertexBuffer[i].normal.y + gram.z * mesh.vertexBuffer[i].normal.z;
		//int apa = 0;

	}

	//float test = mesh.vertexBuffer[0].tangent.x * mesh.vertexBuffer[0].bitangent.x + mesh.vertexBuffer[0].tangent.y * mesh.vertexBuffer[0].bitangent.y + mesh.vertexBuffer[0].tangent.z * mesh.vertexBuffer[0].bitangent.z;

	
	delete vertices;
	delete uvCoords;
	delete normals;

	return mesh;
	//setBuffers(gDevice);
}

void OBJLoader::CountData(std::string fileName, MeshData& mesh) // Count the different data types
{
	std::ifstream file;
	int nrOfMeshes = 1; //CHANGE THIS LATER TO BE ABLE TO HANDLE SUBMESHES

	vCount = vtCount = vnCount = iCount = 0;

	char data, last = NULL;

	file.open(fileName);
	file.get(data);
	while (!file.eof())
	{
		if (data == ' ' && last == 'v')
			vCount++;

		if (data == 't' && last == 'v')
			vtCount++;

		if (data == 'n' && last == 'v')
			vnCount++;

		if (data == ' ' && last == 'f')
			iCount++;

		last = data;
		file.get(data);
	}
	file.close();

	if (nrOfMeshes == 1)
	{
		//mesh.subMeshGroup = -1;
	}
	//else set subMeshGroup stuff

	mesh.vertexBuffer = new StandardVertex[iCount * 3]; // THESE THINGS ALSO NEED TO CHANGE TO BE ABLE TO HANDLE MULTIPLE SUBMESHES

	//mesh.vertexBuffer[0] = new StandardVertex[iCount * 3]; 
	mesh.nrOfIndices = iCount * 3;

	vertices = new Float3D[vCount];
	uvCoords = new Float2D[vtCount];
	normals = new Float3D[vnCount];

	//modelData = new ObjVertexData[iCount * 3];
}

//void ModelImporter::setBuffers(ID3D11Device* gDevice)
//{
//	HRESULT test;
//
//	D3D11_BUFFER_DESC vBufferDesc;
//	memset(&vBufferDesc, 0, sizeof(vBufferDesc));
//	vBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
//	vBufferDesc.Usage = D3D11_USAGE_DEFAULT;
//	vBufferDesc.ByteWidth = sizeof(ObjVertexData) * iCount * 3;
//
//	D3D11_SUBRESOURCE_DATA vData;
//	vData.pSysMem = modelData;
//	gDevice->CreateBuffer(&vBufferDesc, &vData, &modelVertexBuffer);
//
//	indices = new UINT[iCount * 3];
//
//	for (int i = 0; i < iCount * 3; i++)
//	{
//		indices[i] = iCount * 3 - 1 - i;
//	}
//
//	D3D11_BUFFER_DESC iBufferDesc;
//	iBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
//	iBufferDesc.ByteWidth = sizeof(int) * iCount * 3;
//	iBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
//	iBufferDesc.CPUAccessFlags = 0;
//	iBufferDesc.MiscFlags = 0;
//	iBufferDesc.StructureByteStride = 0;
//
//	D3D11_SUBRESOURCE_DATA orderData;
//	orderData.pSysMem = indices;
//	test = gDevice->CreateBuffer(&iBufferDesc, &orderData, &modelIndexBuffer);
//}

//void ModelImporter::loadMTL(ID3D11Device* gDevice, ID3D11DeviceContext* gDeviceContext, string mtlname)
//{
//	HRESULT hr;
//	wstring pathname(directory);
//	wstring filepath(directory);
//	pathname = pathname + L"\\Textures\\Objects\\";
//	filepath = filepath + L"\\Obj\\";
//	filepath.append(mtlname.begin(), mtlname.end());
//	string test;
//	test.assign(filepath.begin(), filepath.end());
//
//	if (mtlname.compare("") == 0) // If no name was found, use default texture
//	{
//		CreateWICTextureFromFile(gDevice, gDeviceContext, L"dragonTex.jpg", 0, &texture, NULL);
//		return;
//	}
//
//	ifstream file;
//	string temps;
//	wstring temp;
//
//	std::string::size_type sz;
//
//	char data, last = NULL, failsafe = NULL;
//
//
//	file.open(test.c_str());
//
//	if (file.fail()) // If file can't be opened, use default texture
//	{
//		CreateWICTextureFromFile(gDevice, gDeviceContext, L"dragonTex.jpg", 0, &texture, NULL);
//		file.close();
//		return;
//	}
//
//	file.get(data);
//	while (!file.eof())
//	{
//		file.get(data);
//
//		while (data != ' ' && data != '\n')
//		{
//			temps += data;
//			file.get(data);
//		}
//
//
//		if (temps.compare("map_Kd") == 0)
//		{
//			temp = L"";
//			file.get(data);
//
//			while (data != ' ' && data != '\n')
//			{
//				temp += data;
//				file.get(data);
//			}
//
//			temp = pathname + temp;
//			CreateWICTextureFromFile(gDevice, gDeviceContext, temp.c_str(), 0, &texture, NULL);
//
//			file.close();
//			return;
//		}
//		temp = L"";
//		temps = "";
//
//
//	}
//
//}

//string ModelImporter::getMtlName(string filename)
//{
//	ifstream file;
//	string temp;
//
//	char data;
//
//	file.open(filename);
//	file.get(data);
//	while (!file.eof())
//	{
//
//		while (data != ' ' && data != '\n')
//		{
//			temp += data;
//			file.get(data);
//		}
//
//
//		if (temp.compare("mtllib") == 0)
//		{
//			temp = "";
//			file.get(data);
//
//			while (data != ' ' && data != '\n')
//			{
//				temp += data;
//				file.get(data);
//			}
//
//			file.close();
//			return temp;
//		}
//
//		temp = "";
//		file.get(data);
//	}
//	file.close();
//
//	return "";
//}