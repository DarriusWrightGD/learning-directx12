#pragma once
#include <Window\DirectXWindow.h>
#include <DirectXMath.h>
#include <Util\UploadBuffer.h>
#include <MeshGeometry.h>
#include "ObjectConstants.h"
#include <memory>
#include <vector>


class HelloRender : public DirectXWindow
{
public:
	HelloRender(HINSTANCE instanceHandle);
	virtual ~HelloRender();

	// Inherited via DirectXWindow
	void Init() override;
	void Update(const GameTimer & timer) override;
	void Draw(const GameTimer & timer) override;
	void OnMouseMove(WPARAM state, int x, int y) override;
	void OnMouseDown(WPARAM state, int x, int y) override;
	void OnMouseUp(WPARAM state, int x, int y) override;
	void OnResize() override;
private:
	void BuildDescriptorHeaps();
	void BuildConstantBuffers();
	void BuildRootSignature();
	void BuildShadersAndInputLayout();
	void BuildBoxGeometry();
	void BuildPSO();

	ComPtr<ID3D12DescriptorHeap> cbvHeap = nullptr;
	ComPtr<ID3D12RootSignature> rootSignature = nullptr;
 

	D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;

	std::unique_ptr<UploadBuffer<ObjectConstants>> objectConstantsCB = nullptr;
	std::unique_ptr<MeshGeometry> boxGeo = nullptr;

	std::vector<D3D12_INPUT_ELEMENT_DESC> inputLayout;
	ComPtr<ID3D12PipelineState> pso = nullptr;

	DirectX::XMFLOAT2 lastMousePosition;
	DirectX::XMFLOAT4X4 view;
	DirectX::XMFLOAT4X4 world;
	DirectX::XMFLOAT4X4 projection;
	ComPtr<ID3DBlob> mVsByteCode = nullptr;
	ComPtr<ID3DBlob> mPsByteCode = nullptr;

	float theta = 1.5f * DirectX::XM_PI;
	float phi = DirectX::XM_PIDIV4;
	float radius = 5.0f;
};

