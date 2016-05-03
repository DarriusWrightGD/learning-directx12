#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <Util/UploadBuffer.h>
#include <memory>
#include "ObjectConstants.h"

struct PassConstants
{
	DirectX::XMFLOAT4X4 View;
	DirectX::XMFLOAT4X4 Projection;
	DirectX::XMFLOAT4X4 InvView;
	DirectX::XMFLOAT4X4 InvProjection;
	DirectX::XMFLOAT4X4 ViewProjection;
	DirectX::XMFLOAT4X4 InvViewProjection;
	DirectX::XMFLOAT3 EyePosition;
	DirectX::XMFLOAT2 RenderTargetSize;
	DirectX::XMFLOAT2 InvRenderTargetSize;
	float NearZ;
	float FarZ;
	float TotalTime;
	float DeltaTime;
};

class FrameResource
{
public:
	FrameResource(ID3D12Device* device, UINT passCount, UINT objectCount);
	FrameResource(const FrameResource& rhs) = delete;
	FrameResource& operator=(const FrameResource& rhs) = delete;
	~FrameResource();

	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandListAllocator;
	std::unique_ptr<UploadBuffer<PassConstants>> passCB = nullptr;
	std::unique_ptr<UploadBuffer<ObjectConstants>> objectCB = nullptr;

	UINT64 fence=0;
};

