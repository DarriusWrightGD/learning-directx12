#pragma once
#include <Util\DxUtil.h>
#include <Export.h>
#include "DXHelper.h"

template<typename T>
class UploadBuffer
{
public:
	UploadBuffer(ID3D12Device * device, UINT elementCount, bool isConstantBuffer = false)
		: isConstantBuffer(isConstantBuffer)
	{
		elementByteSize = sizeof(T);
		if (isConstantBuffer)
		{
			elementByteSize = DxUtil::GetConstantBufferPadding(elementByteSize);
		}

		ThrowIfFailed(device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(elementByteSize*elementCount),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&uploadBuffer)));

		ThrowIfFailed(uploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&mappedData)));


		//ThrowIfFailed(device->CreateCommittedResource(
		//	&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		//	D3D12_HEAP_FLAG_NONE,
		//	&CD3DX12_RESOURCE_DESC::Buffer(elementByteSize * elementCount),
		//	D3D12_RESOURCE_STATE_GENERIC_READ,
		//	nullptr,
		//	IID_PPV_ARGS(uploadBuffer.GetAddressOf())
		//	));

		//ThrowIfFailed(uploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(mappedData)))
	}

	UploadBuffer(const UploadBuffer & rhs) = delete;
	UploadBuffer& operator=(const UploadBuffer & buffer) = delete;
	~UploadBuffer()
	{
		if (uploadBuffer != nullptr)
			uploadBuffer->Unmap(0, nullptr);

		mappedData = nullptr;
	}
	ID3D12Resource * Resource() const
	{
		return uploadBuffer.Get();
	}

	void CopyData(int elementIndex, const T & data)
	{
		memcpy(&mappedData[elementIndex*elementByteSize], &data, sizeof(T));
	}
private:
	UINT elementByteSize = 0;
	bool isConstantBuffer;
	Microsoft::WRL::ComPtr<ID3D12Resource> uploadBuffer;
	BYTE * mappedData = nullptr;
};

