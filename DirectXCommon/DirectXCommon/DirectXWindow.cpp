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

DirectXWindow::DirectXWindow(HINSTANCE instanceHandle) : applicationHandle(instanceHandle)
{
}

DirectXWindow::~DirectXWindow()
{
	if (device != nullptr)
	{
		FlushCommandQueue();
	}
}

bool DirectXWindow::Initialize()
{
	bool success = InitializeWindow() && InitializeDirectX();
	if (success) Init();
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
	wc.hInstance = applicationHandle; // window instance handle thing?
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

	mainWindowHandle = CreateWindow(
		L"BasicWndClass",
		mainWindowCaption.c_str(), // window name
		WS_OVERLAPPEDWINDOW, //what does this mean
		100, //x
		25, // y
		width, // width
		height, // height
		nullptr, // no parent
		nullptr, // no menu
		applicationHandle, // app instance
		nullptr // no extra creation information
		);

	if (mainWindowHandle == nullptr)
	{
		MessageBox(0, L"Create window failed...", 0, 0);
		return false;
	}

	ShowWindow(mainWindowHandle, true);
	UpdateWindow(mainWindowHandle);
	return true;
}

bool DirectXWindow::InitializeDirectX()
{
#if defined(DEBUG) || defined(_DEBUG)
{
	ComPtr<ID3D12Debug> debugController;
	ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
	debugController->EnableDebugLayer();
}
#endif

	ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&factory)));
	auto hardwareResult = D3D12CreateDevice(nullptr, //default adapter
		D3D_FEATURE_LEVEL_11_0, // why 11?
		IID_PPV_ARGS(&device)
		);

	//Fall back to warp device (WARP - Windows Advanced Rasterization Platform)
	if (FAILED(hardwareResult))
	{
		ComPtr<IDXGIAdapter> warpAdapter;
		ThrowIfFailed(factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));
		ThrowIfFailed(D3D12CreateDevice(warpAdapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device)));
	}

	//initialize the fence for CPU/GPU syncronization
	ThrowIfFailed(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));

	rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	dsvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	cbvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
	msQualityLevels.Format = backbufferFormat;
	msQualityLevels.SampleCount = 4;
	msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	msQualityLevels.NumQualityLevels = 1;
	ThrowIfFailed(device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &msQualityLevels, sizeof(msQualityLevels)));
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
	ThrowIfFailed(device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue)));
	ThrowIfFailed(device->CreateCommandAllocator(queueDesc.Type, IID_PPV_ARGS(commandListAllocator.GetAddressOf())));
	ThrowIfFailed(device->CreateCommandList(0, queueDesc.Type, commandListAllocator.Get(), nullptr, IID_PPV_ARGS(commandList.GetAddressOf())));

	//You have to start off closed, this is because the first that we refer to the command list we will reset it. 
	//The only way to reset it is if it is in a closed state.
	commandList->Close();
}

void DirectXWindow::CreateSwapChain()
{
	//DXGI_SWAP_CHAIN_DESC chainDesc = {};
	//chainDesc.BufferDesc.Width = width;
	//chainDesc.BufferDesc.Height = height;
	//chainDesc.BufferDesc.RefreshRate.Numerator = 60;
	//chainDesc.BufferDesc.RefreshRate.Denominator = 1;
	//chainDesc.BufferDesc.Format = backbufferFormat;
	//chainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	//chainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	//chainDesc.SampleDesc.Count =  1;
	//chainDesc.SampleDesc.Quality = 0;
	//chainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	//chainDesc.OutputWindow = mainWindowHandle;
	//chainDesc.Windowed = true;
	//chainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	//chainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	//ThrowIfFailed(factory->CreateSwapChain(commandQueue.Get(), &chainDesc, swapChain.GetAddressOf()));

	DXGI_SWAP_CHAIN_DESC1 chainDesc = {};
	chainDesc.Width = width;
	chainDesc.Height = height;
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

	ThrowIfFailed(factory->CreateSwapChainForHwnd(commandQueue.Get(), mainWindowHandle, &chainDesc, nullptr, nullptr, swapChain.GetAddressOf()));
}

void DirectXWindow::InitializeDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
	rtvHeapDesc.NumDescriptors = 2;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NodeMask = 0;

	ThrowIfFailed(device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(rtvHeap.GetAddressOf())));

	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvHeapDesc.NodeMask = 0;

	ThrowIfFailed(device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(dsvHeap.GetAddressOf())));
}

void DirectXWindow::CreateRenderTargetView()
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(rtvHeap->GetCPUDescriptorHandleForHeapStart());
	for (size_t i = 0; i < swapChainBufferCount; i++)
	{
		ThrowIfFailed(swapChain->GetBuffer(i, IID_PPV_ARGS(swapChainBuffer[i].GetAddressOf())));
		device->CreateRenderTargetView(swapChainBuffer[i].Get(), nullptr, rtvHeapHandle);
		rtvHeapHandle.Offset(1, rtvDescriptorSize);
	}
}

void DirectXWindow::CreateDepthStencilView()
{
	D3D12_RESOURCE_DESC depthStencilDesc;
	depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthStencilDesc.Alignment = 0;
	depthStencilDesc.Width = width;
	depthStencilDesc.Height = height;
	depthStencilDesc.DepthOrArraySize = 1;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.Format = depthStencilFormat;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE optClear;
	optClear.Format = depthStencilFormat;
	optClear.DepthStencil.Depth = 1.0f;
	optClear.DepthStencil.Stencil = 0;
	ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&depthStencilDesc,
		D3D12_RESOURCE_STATE_COMMON,
		&optClear,
		IID_PPV_ARGS(depthStencilBuffer.GetAddressOf())
		));

	device->CreateDepthStencilView(
		depthStencilBuffer.Get(),
		nullptr,
		DepthBufferStencilView());
}

void DirectXWindow::SetupViewport()
{
	viewPort.TopLeftX = 0;
	viewPort.TopLeftY = 0;
	viewPort.Width = static_cast<float>(width);
	viewPort.Height = static_cast<float>(height);
	viewPort.MinDepth = 0.0f;
	viewPort.MaxDepth = 1.0f;
}

void DirectXWindow::SetupScissorRectangles()
{
	scissorRect = { 0,0,width,height };
}

void DirectXWindow::CalculateFrameStats()
{
	static int frameCount = 0;
	static float timeElapsed = 0.0f;

	frameCount++;

	if (timer.TotalTime() - timeElapsed >= 1.0f)
	{
		float fps = (float)frameCount;
		float mspf = 1000.0f / fps;

		wstring fpsStr = to_wstring(fps);
		wstring mspfStr = to_wstring(mspf);
		wstring windowText = mainWindowCaption + L" fps: " + fpsStr + L" mspf: " + mspfStr;
		SetWindowText(mainWindowHandle, windowText.c_str());
		frameCount = 0;
		timeElapsed += 1.0f;
	}
}

int DirectXWindow::Run()
{
	MSG msg = { 0 };
	timer.Reset();
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			timer.Tick();

			if (appPaused)
			{
				Sleep(100);
			}
			else
			{
				CalculateFrameStats();
				Update(timer);
				Draw(timer);
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
			appPaused = true;
			timer.Stop();
		}
		else
		{
			appPaused = false;
			timer.Start();
		}
		return 0;
	case WM_ENTERSIZEMOVE:
		appPaused = true;
		resizing = true;
		timer.Stop();
		return 0;
	case WM_EXITSIZEMOVE:
		appPaused = false;
		resizing = false;
		timer.Start();
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
			DestroyWindow(mainWindowHandle);
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
	return swapChainBuffer[currentBackBuffer].Get();
}

float DirectXWindow::AspectRatio()
{
	return static_cast<float>(width) / static_cast<float>(height);
}

void DirectXWindow::FlushCommandQueue()
{
	//advancing the fence value to mark what commands need to be processed
	currentFence++;
	//tell the command queue to set up a new fence point 
	ThrowIfFailed(commandQueue->Signal(fence.Get(), currentFence));

	//wait until the gpu has complete all command up until this point
	if (fence->GetCompletedValue() < currentFence)
	{
		auto eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);

		//fire an event when gpu hits the fence
		ThrowIfFailed(fence->SetEventOnCompletion(currentFence, eventHandle));

		//wait until the event is fired
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}
}

void DirectXWindow::OnResize()
{
	assert(device);
	assert(swapChain);
	assert(commandListAllocator);

	//flush before changing any resource
	FlushCommandQueue();

	ThrowIfFailed(commandList->Reset(commandListAllocator.Get(), nullptr));

	for (int i = 0; i < swapChainBufferCount; i++)
	{
		swapChainBuffer[i].Reset();
	}

	depthStencilBuffer.Reset();
	ThrowIfFailed(swapChain->ResizeBuffers(
		swapChainBufferCount,
		width,
		height,
		backbufferFormat,
		DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH
		));
	currentBackBuffer = 0;

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(rtvHeap->GetCPUDescriptorHandleForHeapStart());
	for (UINT i = 0; i < swapChainBufferCount; i++)
	{
		ThrowIfFailed(swapChain->GetBuffer(i, IID_PPV_ARGS(&swapChainBuffer[i])));
		device->CreateRenderTargetView(swapChainBuffer[i].Get(), nullptr, rtvHeapHandle);
		rtvHeapHandle.Offset(1, rtvDescriptorSize);
	}

	CreateDepthStencilView();
	ThrowIfFailed(commandList->Close());
	ID3D12CommandList* commandLists[] = { commandList.Get() };
	commandQueue->ExecuteCommandLists(_countof(commandLists), commandLists);

	FlushCommandQueue();

	SetupViewport();
	SetupScissorRectangles();
}
