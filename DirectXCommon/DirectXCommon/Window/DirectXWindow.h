#pragma once
#include <Util\DXHelper.h>
#include <wrl.h>
#include <d3d12.h>
#include <d3dx12.h>
#include <dxgi1_4.h>
#include <dxgidebug.h>

#include <cassert>
#include <Util\GameTimer.h>
#include <string>
#include <Export.h>

using namespace Microsoft::WRL;

class DirectXWindow
{
public:
	ENGINE_SHARED DirectXWindow(HINSTANCE instanceHandle);
	ENGINE_SHARED virtual ~DirectXWindow();
	ENGINE_SHARED bool Initialize();
	ENGINE_SHARED int Run();
	ENGINE_SHARED virtual LRESULT MessageProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


protected:
	ENGINE_SHARED D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView() const
	{
		return CD3DX12_CPU_DESCRIPTOR_HANDLE(
			rtvHeap->GetCPUDescriptorHandleForHeapStart(),
			currentBackBuffer,
			rtvDescriptorSize
			);
	}

	ENGINE_SHARED D3D12_CPU_DESCRIPTOR_HANDLE DepthBufferStencilView() const
	{
		return dsvHeap->GetCPUDescriptorHandleForHeapStart();
	}

	ENGINE_SHARED ID3D12Resource* CurrentBackBuffer()const;

	ENGINE_SHARED virtual void Init() = 0;
	ENGINE_SHARED virtual void Update(const GameTimer & timer) = 0;
	ENGINE_SHARED virtual void Draw(const GameTimer & timer) = 0;
	ENGINE_SHARED float AspectRatio();
	ENGINE_SHARED void FlushCommandQueue();
	ENGINE_SHARED virtual void OnResize();
	ENGINE_SHARED virtual void OnMouseUp(WPARAM state, int x, int y) {}
	ENGINE_SHARED virtual void OnMouseDown(WPARAM state, int x, int y) {}
	ENGINE_SHARED virtual void OnMouseMove(WPARAM state, int x, int y) {}

	HWND mainWindowHandle = 0;
	HINSTANCE applicationHandle = 0;

	ComPtr<IDXGIFactory4> factory = nullptr;
	ComPtr<ID3D12Device> device = nullptr;
	ComPtr<ID3D12Fence> fence = nullptr;

	ComPtr<ID3D12CommandQueue> commandQueue = nullptr;
	ComPtr<ID3D12GraphicsCommandList> commandList = nullptr;
	ComPtr<ID3D12CommandAllocator> commandListAllocator = nullptr;


	const static int swapChainBufferCount = 2;
	ComPtr<IDXGISwapChain> swapChain = nullptr;
	ComPtr<ID3D12Resource> swapChainBuffer[swapChainBufferCount];
	DXGI_FORMAT backbufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	int currentBackBuffer = 0;
	UINT64 currentFence = 0;



	ComPtr<ID3D12Resource> depthStencilBuffer = nullptr;
	DXGI_FORMAT depthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;


	ComPtr<ID3D12DescriptorHeap> rtvHeap = nullptr;
	ComPtr<ID3D12DescriptorHeap> dsvHeap = nullptr;
	UINT rtvDescriptorSize = 0;
	UINT dsvDescriptorSize = 0;
	UINT cbvDescriptorSize = 0;

	UINT m4xMsaaQuality;

	bool appPaused = false;
	bool minimized = false;
	bool maximized = false;
	bool resizing = false;
	bool fullScreen = false;

	RECT scissorRect;
	D3D12_VIEWPORT viewPort;
	int width = 1024;
	int height = 768;

	GameTimer timer;
	std::wstring mainWindowCaption = L"DirectX Window";
private:
	bool InitializeWindow();
	bool InitializeDirectX();
	void InitializeCommandObjects();
	void CreateSwapChain();
	void InitializeDescriptorHeaps();
	void CreateRenderTargetView();
	void CreateDepthStencilView();
	void SetupViewport();
	void SetupScissorRectangles();
	void CalculateFrameStats();
};

