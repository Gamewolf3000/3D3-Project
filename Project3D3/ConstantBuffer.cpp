#include "ConstantBuffer.h"

INT8 ConstantBufferHandler::CreateConstantBuffer(void * data, size_t dataSize, ConstantBufferType bufferType)
{
	ConstantBuffer* buffer = new ConstantBuffer;
	buffer->rawData = new char[dataSize];
	memcpy(buffer->rawData, data, dataSize);
	buffer->dataSize = dataSize;
	buffer->type = bufferType;

	if (cpu_MappedPtrs.find(bufferType) == cpu_MappedPtrs.end())
	{
		/*Taking for granted that the buffers are created during initialization, NOT during rendering*/
		CreateHeap(bufferType);
	}

	bufferVector.push_back(buffer);

	return nrOfBuffers++;
}

void ConstantBufferHandler::CreateHeap(ConstantBufferType bufferType)
{
	/*Heap creation goes here*/
}

void ConstantBufferHandler::BindBuffer(INT8 ID, UINT offset)
{
	/*No error checking yet!*/
	ConstantBuffer* buffer = bufferVector[ID];
	char* pointerWithOffset = (char*)cpu_MappedPtrs.find(buffer->type)->second;
	pointerWithOffset += offset;
	memcpy(pointerWithOffset, buffer->rawData, buffer->dataSize);

}

ConstantBufferHandler::ConstantBufferHandler()
{
}

ConstantBufferHandler::~ConstantBufferHandler()
{
	for (auto &buffer : bufferVector)
		delete buffer;
}
