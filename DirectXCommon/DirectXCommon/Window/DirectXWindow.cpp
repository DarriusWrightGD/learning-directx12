#include "DirectXWindow.h"
#include <windowsx.h>

using std::to_wstring;
using std::wstring;

namespace
{
	static DirectXWindow * window = nullptr;
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return window->MessageProc(hWnd, msg, wParam, lParam);
}

DirectXWindow::DirectXWindow(HINSTANCE instanceHandle) : mApplicationHandle(instanceHandle)
{
}

DirectXWindow::~DirectXWindow()
{
	if (mDevice != nullptr)
	{
		FlushCommandQueue();
	}
}

bool DirectXWindow::Initialize()
{
	bool success = InitializeWindow() && InitializeDirectX();
	if (success) Init();
	OnResize();
	return success;
}

bool DirectXWindow::InitializeWindow()
{
	window = this;
	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW; //horizontial and vertical 
	wc.lpfnWndProc = WndProc; // set the event handler 
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = mApplicationHandle; // window instance handle thing?
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);// what other icons are there?
	wc.hCursor = LoadCursor(0, IDC_CROSS);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName = nullptr;
	wc.lpszClassName = L"BasicWndClass";

	//register the window class to later create the window 
	if (!RegisterClass(&wc))
	{
		MessageBox(0, L"Could not register window class", 0, 0);
		return false;
	}

	//calling create window to, you guessed it create the window. We will get a handle back 
	// from the createwindow function.

	mMainWindowHandle = CreateWindow(
		L"BasicWndClass",
		mMainWindowCaption.c_str(), // window name
		WS_OVERLAPPEDWINDOW, //what does this mean
		100, //x
		25, // y
		mWidth, // mWidth
		mHeight, // mHeight
		nullptr, // no parent
		nullptr, // no menu
		mApplicationHandle, // app instance
		nullptr // no extra creation information
		);

	if (mMainWindowHandle == nullptr)
	{
		MessageBox(0, L"Create window failed...", 0, 0);
		return false;
	}

	ShowWindow(mMainWindowHandle, true);
	UpdateWindow(mMainWindowHandle);
	return true;
}

bool DirectXWindow::InitializeDirectX()
{
//#if defined(DEBUG) || defined(_DEBUG)
//{
//	ComPtr<ID3D12Debug> debugController;
//	ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
//	debugController->EnableDebugLayer();
//}
//#endif

	ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&mFactory)));
	auto hardwareResult = D3D12CreateDevice(nullptr, //default adapter
		D3D_FEATURE_LEVEL_11_0, // why 11?
		IID_PPV_ARGS(&mDevice)
		);

	//Fall back to warp mDevice (WARP - Windows Advanced Rasterization Platform)
	if (FAILED(hardwareResult))
	{
		ComPtr<IDXGIAdapter> warpAdapter;
		ThrowIfFailed(mFactory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));
		ThrowIfFailed(D3D12CreateDevice(warpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&mDevice)));
	}

	//initialize the mFence for CPU/GPU syncronization
	ThrowIfFailed(mDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&mFence)));

	mRtvDescriptorSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	mDsvDescriptorSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	mCbvDescriptorSize = mDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
	msQualityLevels.Format = mBackBufferFormat;
	msQualityLevels.SampleCount = 4;
	msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	msQualityLevels.NumQualityLevels = 1;
	ThrowIfFailed(mDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &msQualityLevels, sizeof(msQualityLevels)));
	m4xMsaaQuality = msQualityLevels.NumQualityLevels;

	//4x sampling should always be avaiable therefore the assert
	assert(m4xMsaaQuality > 0 && "Unexpected quality levels");

	InitializeCommandObjects();
	CreateSwapChain();
	InitializeDescriptorHeaps();
	CreateRenderTargetView();
	//open command list?
	CreateDepthStencilView();
	SetupViewport();
	SetupScissorRectangles();

	return true;
}

void DirectXWindow::InitializeCommandObjects()
{
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	ThrowIfFailed(mDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&mCommandQueue)));
	ThrowIfFailed(mDevice->CreateCommandAllocator(queueDesc.Type, IID_PPV_ARGS(mCommandListAllocator.GetAddressOf())));
	ThrowIfFailed(mDevice->CreateCommandList(0, queueDesc.Type, mCommandListAllocator.Get(), nullptr, IID_PPV_ARGS(mCommandList.GetAddressOf())));

	//You have to start off closed, this is because the first that we refer to the command list we will reset it. 
	//The only way to reset it is if it is in a closed state.
	mCommandList->Close();
}

void DirectXWindow::CreateSwapChain()
{

	// Release the previous swapchain we will be recreating.
	mSwapChain.Reset();

	DXGI_SWAP_CHAIN_DESC sd;
	sd.BufferDesc.Width = mWidth;
	sd.BufferDesc.Height = mHeight;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.Format = mBackBufferFormat;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	sd.SampleDesc.Count =  1;
	sd.SampleDesc.Quality = 0;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = SwapChainBufferCount;
	sd.OutputWindow = mMainWindowHandle;
	sd.Windowed = true;
	sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	// Note: Swap chain uses queue to perform flush.
	ThrowIfFailed(mFactory->CreateSwapChain(
		mCommandQueue.Get(),
		&sd,
		mSwapChain.GetAddressOf()));

	//DXGI_SWAP_CHAIN_DESC chainDesc = {};
	//chainDesc.BufferDesc.Width = mWidth;
	//chainDesc.BufferDesc.Height = mHeight;
	//chainDesc.BufferDesc.RefreshRate.Numerator = 60;
	//chainDesc.BufferDesc.RefreshRate.Denominator = 1;
	//chainDesc.BufferDesc.Format = mBackbufferFormat;
	//chainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	//chainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	//chainDesc.SampleDesc.Count =  1;
	//chainDesc.SampleDesc.Quality = 0;
	//chainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	//chainDesc.OutputWindow = mainWindowHandle;
	//chainDesc.Windowed = true;
	//chainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	//chainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	//ThrowIfFailed(mFactory->CreateSwapChain(mCommandQueue.Get(), &chainDesc, mSwapChain.GetAddressOf()));

	/*DXGI_SWAP_CHAIN_DESC1 chainDesc = {};
	chainDesc.Width = mWidth;
	chainDesc.Height = mHeight;
	chainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	chainDesc.Stereo = FALSE;
	chainDesc.SampleDesc.Count = 1;
	chainDesc.SampleDesc.Quality = 0;
	chainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	chainDesc.BufferCount = 2;
	chainDesc.Scaling = DXGI_SCALING_NONE;
	chainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	chainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	chainDesc.Flags = 0;

	ThrowIfFailed(mFactory->CreateSwapChainForHwnd(mCommandQueue.Get(), mainWindowHandle, &chainDesc, nullptr, nullptr, mSwapChain.GetAddressOf()));*/
}

void DirectXWindow::InitializeDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
	rtvHeapDesc.NumDescriptors = SwapChainBufferCount;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NodeMask = 0;

	ThrowIfFailed(mDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(mRtvHeap.GetAddressOf())));

	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvHeapDesc.NodeMask = 0;

	ThrowIfFailed(mDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(mDsvHeap.GetAddressOf())));
}

void DirectXWindow::CreateRenderTargetView()
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(mRtvHeap->GetCPUDescriptorHandleForHeapStart());
	for (size_t i = 0; i < SwapChainBufferCount; i++)
	{
		ThrowIfFailed(mSwapChain->GetBuffer(i, IID_PPV_ARGS(mSwapChainBuffer[i].GetAddressOf())));
		mDevice->CreateRenderTargetView(mSwapChainBuffer[i].Get(), nullptr, rtvHeapHandle);
		rtvHeapHandle.Offset(1, mRtvDescriptorSize);
	}
}

void DirectXWindow::CreateDepthStencilView()
{
	D3D12_RESOURCE_DESC depthStencilDesc;
	depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthStencilDesc.Alignment = 0;
	depthStencilDesc.Width = mWidth;
	depthStencilDesc.Height = mHeight;
	depthStencilDesc.DepthOrArraySize = 1;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.Format = mDepthStencilFormat;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE optClear;
	optClear.Format = mDepthStencilFormat;
	optClear.DepthStencil.Depth = 1.0f;
	optClear.DepthStencil.Stencil = 0;
	ThrowIfFailed(mDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&depthStencilDesc,
		D3D12_RESOURCE_STATE_COMMON,
		&optClear,
		IID_PPV_ARGS(mDepthStencilBuffer.GetAddressOf())
		));

	mDevice->CreateDepthStencilView(
		mDepthStencilBuffer.Get(),
		nullptr,
		DepthStencilView());
}

void DirectXWindow::SetupViewport()
{
	mViewPort.TopLeftX = 0;
	mViewPort.TopLeftY = 0;
	mViewPort.Width = static_cast<float>(mWidth);
	mViewPort.Height = static_cast<float>(mHeight);
	mViewPort.MinDepth = 0.0f;
	mViewPort.MaxDepth = 1.0f;
}

void DirectXWindow::SetupScissorRectangles()
{
	mScissorRect = { 0,0,mWidth,mHeight };
}

void DirectXWindow::CalculateFrameStats()
{
	static int frameCount = 0;
	static float timeElapsed = 0.0f;

	frameCount++;

	if (mTimer.TotalTime() - timeElapsed >= 1.0f)
	{
		float fps = (float)frameCount;
		float mspf = 1000.0f / fps;

		wstring fpsStr = to_wstring(fps);
		wstring mspfStr = to_wstring(mspf);
		wstring windowText = mMainWindowCaption + L" fps: " + fpsStr + L" mspf: " + mspfStr;
		SetWindowText(mMainWindowHandle, windowText.c_str());
		frameCount = 0;
		timeElapsed += 1.0f;
	}
}

int DirectXWindow::Run()
{
	MSG msg = { 0 };
	mTimer.Reset();
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			mTimer.Tick();

			if (mAppPaused)
			{
				Sleep(100);
			}
			else
			{
				CalculateFrameStats();
				Update(mTimer);
				Draw(mTimer);
			}
		}
	}

	return (int)msg.wParam;
}

LRESULT DirectXWindow::MessageProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_ACTIVATE:
		if (LOWORD(wParam) == WA_INACTIVE)
		{
			mAppPaused = true;
			mTimer.Stop();
		}
		else
		{
			mAppPaused = false;
			mTimer.Start();
		}
		return 0;
	case WM_ENTERSIZEMOVE:
		mAppPaused = true;
		mResizing = true;
		mTimer.Stop();
		return 0;
	case WM_EXITSIZEMOVE:
		mAppPaused = false;
		mResizing = false;
		mTimer.Start();
		OnResize();
		return 0;
	case WM_MENUCHAR:
		return MAKELRESULT(0, MNC_CLOSE);
	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 300;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 300;
		return 0;
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE)
		{
			DestroyWindow(mMainWindowHandle);
		}
		return 0;
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
		OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
		OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_MOUSEMOVE:
		OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

ID3D12Resource * DirectXWindow::CurrentBackBuffer() const
{
	return mSwapChainBuffer[mCurrentBackBuffer].Get();
}

float DirectXWindow::AspectRatio()
{
	return static_cast<float>(mWidth) / static_cast<float>(mHeight);
}

void DirectXWindow::FlushCommandQueue()
{
	//advancing the mFence value to mark what commands need to be processed
	mCurrentFence++;
	//tell the command queue to set up a new mFence point 
	ThrowIfFailed(mCommandQueue->Signal(mFence.Get(), mCurrentFence));

	//wait until the gpu has complete all command up until this point
	if (mFence->GetCompletedValue() < mCurrentFence)
	{
		auto eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);

		//fire an event when gpu hits the mFence
		ThrowIfFailed(mFence->SetEventOnCompletion(mCurrentFence, eventHandle));

		//wait until the event is fired
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}
}

void DirectXWindow::OnResize()
{
	assert(mDevice);
	assert(mSwapChain);
	assert(mCommandListAllocator);

	//flush before changing any resource
	FlushCommandQueue();

	ThrowIfFailed(mCommandList->Reset(mCommandListAllocator.Get(), nullptr));

	for (int i = 0; i < SwapChainBufferCount; i++)
	{
		mSwapChainBuffer[i].Reset();
	}

	mDepthStencilBuffer.Reset();
	ThrowIfFailed(mSwapChain->ResizeBuffers(
		SwapChainBufferCount,
		mWidth,
		mHeight,
		mBackBufferFormat,
		DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH
		));
	mCurrentBackBuffer = 0;

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(mRtvHeap->GetCPUDescriptorHandleForHeapStart());
	for (UINT i = 0; i < SwapChainBufferCount; i++)
	{
		ThrowIfFailed(mSwapChain->GetBuffer(i, IID_PPV_ARGS(&mSwapChainBuffer[i])));
		mDevice->CreateRenderTargetView(mSwapChainBuffer[i].Get(), nullptr, rtvHeapHandle);
		rtvHeapHandle.Offset(1, mRtvDescriptorSize);
	}

	CreateDepthStencilView();
	ThrowIfFailed(mCommandList->Close());
	ID3D12CommandList* commandLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

	FlushCommandQueue();

	SetupViewport();
	SetupScissorRectangles();
}
