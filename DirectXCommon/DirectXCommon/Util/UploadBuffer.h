#pragma once
#include <Util\DxUtil.h>
#include <Export.h>
#include "DXHelper.h"

template<typename T>
class UploadBuffer
{
public:
	UploadBuffer(ID3D12Device * device, UINT elementCount, bool isConstantBuffer = false)
		: mIsConstantBuffer(isConstantBuffer)
	{
		mElementByteSize = sizeof(T);
		if (isConstantBuffer)
		{
			mElementByteSize = DxUtil::GetConstantBufferPadding(mElementByteSize);
		}

		ThrowIfFailed(device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(mElementByteSize*elementCount),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&mUploadBuffer)));

		ThrowIfFailed(mUploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&mMappedData)));


		//ThrowIfFailed(mDevice->CreateCommittedResource(
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
		if (mUploadBuffer != nullptr)
			mUploadBuffer->Unmap(0, nullptr);

		mMappedData = nullptr;
	}
	ID3D12Resource * Resource() const
	{
		return mUploadBuffer.Get();
	}

	void CopyData(int elementIndex, const T & data)
	{
		memcpy(&mMappedData[elementIndex*mElementByteSize], &data, sizeof(T));
	}
private:
	UINT mElementByteSize = 0;
	bool mIsConstantBuffer;
	Microsoft::WRL::ComPtr<ID3D12Resource> mUploadBuffer;
	BYTE * mMappedData = nullptr;
};

