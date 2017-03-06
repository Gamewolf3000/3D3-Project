#ifndef OBJ_LOADER_H
#define OBJ_LOADER_H
#pragma once

#include <fstream>
#include <DirectXMath.h>
#include <string>

#include "JEXMath.h"

struct StandardVertex
{
	Float3D position = Float3D(0.0f, 0.0f, 0.0f);
	Float2D uv = Float2D(0.0f, 0.0f);
	Float3D normal = Float3D(0.0f, 0.0f, 0.0f);

	Float3D tangent = Float3D(0.0f, 0.0f, 0.0f);
	Float3D bitangent = Float3D(0.0f, 0.0f, 0.0f);
};

struct MeshData
{
	StandardVertex* vertexBuffer = nullptr;
	unsigned int* indexBuffer = nullptr;
	unsigned int nrOfIndices = 0;
	bool render = true;

	unsigned int nrOfUsers = 0;
	std::string identifier = "";
};

class OBJLoader
{
private:
	std::string* modelDirectory;

	Float3D* vertices = nullptr;
	Float2D* uvCoords = nullptr;
	Float3D* normals = nullptr;

	int vCount, vtCount, vnCount, iCount;

	void CountData(std::string fileName, MeshData& data);

public:

	OBJLoader(wchar_t directory[200]);
	virtual ~OBJLoader();

	MeshData LoadOBJFile(std::string fileName);

};

#endif
