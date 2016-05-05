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
			mRtvHeap->GetCPUDescriptorHandleForHeapStart(),
			mCurrentBackBuffer,
			mRtvDescriptorSize
			);
	}

	ENGINE_SHARED D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView() const
	{
		return mDsvHeap->GetCPUDescriptorHandleForHeapStart();
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

	HWND mMainWindowHandle = 0;
	HINSTANCE mApplicationHandle = 0;

	ComPtr<IDXGIFactory4> mFactory = nullptr;
	ComPtr<ID3D12Device> mDevice = nullptr;
	ComPtr<ID3D12Fence> mFence = nullptr;

	ComPtr<ID3D12CommandQueue> mCommandQueue = nullptr;
	ComPtr<ID3D12GraphicsCommandList> mCommandList = nullptr;
	ComPtr<ID3D12CommandAllocator> mCommandListAllocator = nullptr;


	const static int SwapChainBufferCount = 2;
	ComPtr<IDXGISwapChain> mSwapChain = nullptr;
	ComPtr<ID3D12Resource> mSwapChainBuffer[SwapChainBufferCount];
	DXGI_FORMAT mBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	int mCurrentBackBuffer = 0;
	UINT64 mCurrentFence = 0;



	ComPtr<ID3D12Resource> mDepthStencilBuffer = nullptr;
	DXGI_FORMAT mDepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;


	ComPtr<ID3D12DescriptorHeap> mRtvHeap = nullptr;
	ComPtr<ID3D12DescriptorHeap> mDsvHeap = nullptr;
	UINT mRtvDescriptorSize = 0;
	UINT mDsvDescriptorSize = 0;
	UINT mCbvDescriptorSize = 0;

	UINT m4xMsaaQuality;

	bool mAppPaused = false;
	bool mMinimized = false;
	bool mMaximized = false;
	bool mResizing = false;
	bool mFullScreen = false;

	RECT mScissorRect;
	D3D12_VIEWPORT mViewPort;
	int mWidth = 1024;
	int mHeight = 768;

	GameTimer mTimer;
	std::wstring mMainWindowCaption = L"DirectX Window";
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

