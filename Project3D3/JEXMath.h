#ifndef JEX_MATH_H
#define JEX_MATH_H
#pragma once

#ifdef _WIN32


#include <DirectXMath.h>

#define Vec DirectX::XMVECTOR
#define Matrix DirectX::XMMATRIX
#define JEX_PI DirectX::XM_PI

#define Float4D DirectX::XMFLOAT4
#define Float3D DirectX::XMFLOAT3
#define Float2D DirectX::XMFLOAT2

#define Float4x4 DirectX::XMFLOAT4X4
#define Float3x3 DirectX::XMFLOAT3X3

#define Float3DPlus(a, b) Float3D(a.x + b.x, a.y + b.y, a.z + b.z)
#define Float3DMinus(a, b) Float3D(a.x - b.x, a.y - b.y, a.z - b.z)

#define VecGetByIndex(index, vector) DirectX::XMVectorGetByIndex(vector, index)
#define VecSetByIndex(index, vector, value) DirectX::XMVectorSetByIndex(vector, value, index)
#define VecToFloat3D(float3d, vector) XMStoreFloat3(&float3d, vector);
#define VecCreate(x, y, z, w) DirectX::XMVectorSet(x,y,z,w)
#define VecGetAbs(vector) DirectX::XMVectorGetByIndex(XMVectorAbs(vector), 0)
#define VecGetNormalized3D(vector) DirectX::XMVector3Normalize(vector)
#define VecNormalize3D(vector) vector = DirectX::XMVector3Normalize(vector)
#define VecGetScaled(vector, value) DirectX::XMVectorScale(vector, value)
#define VecScale(vector, value) vector = DirectX::XMVectorScale(vector, value)
#define VecAdd(vector1, vector2) DirectX::XMVectorAdd(vector1, vector2)
#define VecSub(vector1, vector2) DirectX::XMVectorSubtract(vector1, vector2)

#define VecMultMatrix3D(vector, matrix) DirectX::XMVector3Transform(vector, matrix)

#define MatrixIdentity DirectX::XMMatrixIdentity()
#define MatrixTranslation(x, y, z) DirectX::XMMatrixTranslation(x, y, z)
#define MatrixTranslationFromVec(vector) DirectX::XMMatrixTranslationFromVector(vector)
#define MatrixRotationAroundAxis(axis, radians) DirectX::XMMatrixRotationAxis(axis, radians)
#define MatrixScaling(x, y, z) DirectX::XMMatrixScaling(x, y, z)
#define MatrixScalingFromVector(vector) DirectX::XMMatrixScalingFromVector(vector)
#define MatrixViewLH(posVec, lookatVec, upVec) DirectX::XMMatrixLookAtLH(posVec, lookatVec, upVec)
#define MatrixViewRH(posVec, lookatVec, upVec) DirectX::XMMatrixLookAtRH(posVec, lookatVec, upVec)
#define MatrixProjectionLH(fov, aspectRatio, nearPlane, farPlane) DirectX::XMMatrixPerspectiveFovLH(fov, aspectRatio, nearPlane, farPlane)
#define MatrixProjectionRH(fov, aspectRatio, nearPlane, farPlane) DirectX::XMMatrixPerspectiveFovRH(fov, aspectRatio, nearPlane, farPlane)
#define MatrixGetTransposed(matrix) DirectX::XMMatrixTranspose(matrix)
#define MatrixTranspose(matrix) matrix = DirectX::XMMatrixTranspose(matrix)
#define MatrixGetDeterminant(matrix) DirectX::XMVectorGetByIndex(XMMatrixDeterminant(matrix), 0)
#define MatrixInvert(matrix) DirectX::XMMatrixInverse(nullptr, matrix)

#define MatrixToFloat4x4(float4x4, matrix) DirectX::XMStoreFloat4x4(&float4x4, matrix)
#define Float4x4ToMatrix(float4x4) DirectX::XMLoadFloat4x4(&float4x4)

#elif

//Put other version of math library here using the same define names as the dx11 ones

#endif

#endif
