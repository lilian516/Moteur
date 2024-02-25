#include "App.h"
#include "Utils.h"

#include <WindowsX.h>


using Microsoft::WRL::ComPtr;
using namespace std;
using namespace DirectX;


App::App(HINSTANCE hInstance) : m_hAppInst(hInstance){
	// Only one D3DApp can be constructed.
	assert(m_App == nullptr);
	m_App = this;
}

LRESULT CALLBACK
MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return App::getApp()->messageProc(hwnd, msg, wParam, lParam);
}

App* App::m_App = nullptr;

App* App::getApp()
{
	return m_App;
}

LRESULT App::messageProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		// WM_ACTIVATE is sent when the window is activated or deactivated.  
		// We pause the game when the window is deactivated and unpause it 
		// when it becomes active.  
	case WM_ACTIVATE:
		if (LOWORD(wParam) == WA_INACTIVE)
		{
			m_bAppPaused = true;
			//mTimer.Stop();
		}
		else
		{
			m_bAppPaused = false;
			//mTimer.Start();
		}
		return 0;

		// WM_SIZE is sent when the user resizes the window.  
	case WM_SIZE:
		// Save the new client area dimensions.
		m_iClientWidth = LOWORD(lParam);
		m_iClientHeight = HIWORD(lParam);
		if (m_c3dDevice)
		{
			if (wParam == SIZE_MINIMIZED)
			{
				m_bAppPaused = true;
				m_bMinimized = true;
				m_bMaximized = false;
			}
			else if (wParam == SIZE_MAXIMIZED)
			{
				m_bAppPaused = false;
				m_bMinimized = false;
				m_bMaximized = true;
				onResize();
			}
			else if (wParam == SIZE_RESTORED)
			{

				// Restoring from minimized state?
				if (m_bMinimized)
				{
					m_bAppPaused = false;
					m_bMinimized = false;
					onResize();
				}

				// Restoring from maximized state?
				else if (m_bMaximized)
				{
					m_bAppPaused = false;
					m_bMaximized = false;
					onResize();
				}
				else if (m_bResizing)
				{
					// If user is dragging the resize bars, we do not resize 
					// the buffers here because as the user continuously 
					// drags the resize bars, a stream of WM_SIZE messages are
					// sent to the window, and it would be pointless (and slow)
					// to resize for each WM_SIZE message received from dragging
					// the resize bars.  So instead, we reset after the user is 
					// done resizing the window and releases the resize bars, which 
					// sends a WM_EXITSIZEMOVE message.
				}
				else // API call such as SetWindowPos or mSwapChain->SetFullscreenState.
				{
					onResize();
				}
			}
		}
		return 0;

		// WM_EXITSIZEMOVE is sent when the user grabs the resize bars.
	case WM_ENTERSIZEMOVE:
		m_bAppPaused = true;
		m_bResizing = true;
		//mTimer.Stop();
		return 0;

		// WM_EXITSIZEMOVE is sent when the user releases the resize bars.
		// Here we reset everything based on the new window dimensions.
	case WM_EXITSIZEMOVE:
		m_bAppPaused = false;
		m_bResizing = false;
		//mTimer.Start();
		onResize();
		return 0;

		// WM_DESTROY is sent when the window is being destroyed.
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

		// The WM_MENUCHAR message is sent when a menu is active and the user presses 
		// a key that does not correspond to any mnemonic or accelerator key. 
	case WM_MENUCHAR:
		// Don't beep when we alt-enter.
		return MAKELRESULT(0, MNC_CLOSE);

		// Catch this message so to prevent the window from becoming too small.
	case WM_GETMINMAXINFO:
		((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
		((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
		return 0;

	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
		//OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
		//OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_MOUSEMOVE:
		//OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_KEYUP:
		if (wParam == VK_ESCAPE)
		{
			PostQuitMessage(0);
		}
		else if ((int)wParam == VK_F2)
			set4xMsaaState(!m_b4xMsaaState);

		return 0;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

bool App::initialize() {
	if (!initMainWindow()) {
		return false;
	}
	if (!initializeDirect3d()) {
		return false;
	}
	
	onResize();
	return true;
}


void App::set4xMsaaState(bool value)
{
	if (m_b4xMsaaState != value)
	{
		m_b4xMsaaState = value;

		// Recreate the swapchain and buffers with new multisample settings.
		createSwapChain();
		onResize();
	}
}

int App::run()
{
	MSG msg = { 0 };

	//mTimer.Reset();

	while (msg.message != WM_QUIT)
	{
		// If there are Window messages then process them.
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		// Otherwise, do animation/game stuff.
		else
		{
			//mTimer.Tick();

			if (!m_bAppPaused)
			{
				//CalculateFrameStats();
				update();
				drawWindow();
			}
			else
			{
				Sleep(100);
			}
		}
	}

	return (int)msg.wParam;
}


bool App::initMainWindow()
{
	WNDCLASS wc;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = MainWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = m_hAppInst;
	wc.hIcon = LoadIcon(0, IDI_APPLICATION);
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wc.lpszMenuName = 0;
	wc.lpszClassName = L"MainWnd";

	if (!RegisterClass(&wc))
	{
		MessageBox(0, L"RegisterClass Failed.", 0, 0);
		return false;
	}

	// Compute window rectangle dimensions based on requested client area dimensions.
	RECT R = { 0, 0, m_iClientWidth, m_iClientHeight };
	AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
	int width = R.right - R.left;
	int height = R.bottom - R.top;

	m_hMainWindow = CreateWindow(L"MainWnd", m_wMainWndCaption.c_str(),
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, m_hAppInst, 0);
	if (!m_hMainWindow)
	{
		MessageBox(0, L"CreateWindow Failed.", 0, 0);
		return false;
	}

	ShowWindow(m_hMainWindow, SW_SHOW);
	UpdateWindow(m_hMainWindow);

	return true;
}


bool App::initializeDirect3d() {
	
#if defined(DEBUG) || defined(_DEBUG) 
	// Enable the D3D12 debug layer.
	{
		ComPtr<ID3D12Debug> debugController;
		ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
		debugController->EnableDebugLayer();
	}
#endif

	ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&m_cDxgiFactory)));

	

	// Try to create hardware device.
	HRESULT hardwareResult = D3D12CreateDevice(nullptr,D3D_FEATURE_LEVEL_11_0,IID_PPV_ARGS(&m_c3dDevice));

	// Fallback to WARP device.
	if (FAILED(hardwareResult))
	{
		ComPtr<IDXGIAdapter> pWarpAdapter;
		ThrowIfFailed(m_cDxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&pWarpAdapter)));

		ThrowIfFailed(D3D12CreateDevice(
			pWarpAdapter.Get(),
			D3D_FEATURE_LEVEL_11_0,
			IID_PPV_ARGS(&m_c3dDevice)));
	}

	ThrowIfFailed(m_c3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE,IID_PPV_ARGS(&m_cFence)));

	
	m_iRtvDescriptorSize = m_c3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	m_iDsvDescriptorSize = m_c3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	m_iCbvSrvDescriptorSize = m_c3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS dQualityLevels;
	dQualityLevels.Format = m_fBackBufferFormat;
	dQualityLevels.SampleCount = 4;
	dQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	dQualityLevels.NumQualityLevels = 0;
	ThrowIfFailed(m_c3dDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,&dQualityLevels,sizeof(dQualityLevels)));
	m_i4xMsaaQuality = dQualityLevels.NumQualityLevels;
	assert(m_i4xMsaaQuality > 0 && "Unexpected MSAA quality level.");
	//note pour plus tard peut etre problème avec un if et une fonction 3dutils

#ifdef _DEBUG
	logAdapters();
#endif

	createCommandListQueue();
	createSwapChain();
	createRtvAndDsvDescriptorHeaps();

	return true;
	
}

void App::createCommandListQueue() {
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	ThrowIfFailed(m_c3dDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_cCommandQueue)));

	ThrowIfFailed(m_c3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,IID_PPV_ARGS(m_cDirectCmdListAlloc.GetAddressOf())));

	ThrowIfFailed(m_c3dDevice->CreateCommandList(0,D3D12_COMMAND_LIST_TYPE_DIRECT,
		m_cDirectCmdListAlloc.Get(), // Associated command allocator
		nullptr,                   // Initial PipelineStateObject
		IID_PPV_ARGS(m_cCommandList.GetAddressOf())));


	//Commencez dans un état fermé. C'est parce que la première fois que nous faisons référence
    // à la liste des commandes, nous allons la réinitialiser, et elle doit être fermée avant
    // appelant Reset.
	m_cCommandList->Close();
}

void App::createSwapChain() {
	// Release the previous swapchain we will be recreating.
	m_cSwapChain.Reset();
	DXGI_SWAP_CHAIN_DESC sSwapChainDescription;
	sSwapChainDescription.BufferDesc.Width = m_iClientWidth;
	sSwapChainDescription.BufferDesc.Height = m_iClientHeight;
	sSwapChainDescription.BufferDesc.RefreshRate.Numerator = 60;
	sSwapChainDescription.BufferDesc.RefreshRate.Denominator = 1;
	sSwapChainDescription.BufferDesc.Format = m_fBackBufferFormat;
	sSwapChainDescription.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sSwapChainDescription.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	sSwapChainDescription.SampleDesc.Count = m_b4xMsaaState ? 4 : 1;
	sSwapChainDescription.SampleDesc.Quality = m_b4xMsaaState ? (m_i4xMsaaQuality - 1) : 0;
	sSwapChainDescription.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sSwapChainDescription.BufferCount = m_sSwapChainBufferCount;
	sSwapChainDescription.OutputWindow = m_hMainWindow;
	sSwapChainDescription.Windowed = true;
	sSwapChainDescription.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	sSwapChainDescription.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	// Note: La chaîne d'échange utilise la file d'attente pour effectuer le vidage.
	ThrowIfFailed(m_cDxgiFactory->CreateSwapChain(m_cCommandQueue.Get(),&sSwapChainDescription, m_cSwapChain.GetAddressOf()));
	//note pour plus tard peut etre problème avec throwiffailed
}

void App::createRtvAndDsvDescriptorHeaps() {
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
	rtvHeapDesc.NumDescriptors = m_sSwapChainBufferCount;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NodeMask = 0;
	ThrowIfFailed(m_c3dDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(m_cRtvHeap.GetAddressOf())));
	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvHeapDesc.NodeMask = 0;
	ThrowIfFailed(m_c3dDevice->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(m_cDsvHeap.GetAddressOf())));
}

void App::onResize() {
	assert(m_c3dDevice);
	assert(m_cSwapChain);
	assert(m_cDirectCmdListAlloc);

	// Flush before changing any resources.
	flushCommandQueue();

	ThrowIfFailed(m_cCommandList->Reset(m_cDirectCmdListAlloc.Get(), nullptr));

	// Release the previous resources we will be recreating.
	for (int i = 0; i < m_sSwapChainBufferCount; ++i)
		m_cSwapChainBuffer[i].Reset();
	m_cDepthStencilBuffer.Reset();

	// Resize the swap chain.
	ThrowIfFailed(m_cSwapChain->ResizeBuffers(
		m_sSwapChainBufferCount,
		m_iClientWidth, m_iClientHeight,
		m_fBackBufferFormat,
		DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));

	m_iCurrBackBuffer = 0;
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle(m_cRtvHeap->GetCPUDescriptorHandleForHeapStart());
	for (UINT i = 0; i < m_sSwapChainBufferCount; i++)
	{
		m_cSwapChain->GetBuffer(i, IID_PPV_ARGS(&m_cSwapChainBuffer[i]));
		m_c3dDevice->CreateRenderTargetView(m_cSwapChainBuffer[i].Get(), nullptr, rtvHeapHandle);
		rtvHeapHandle.Offset(1, m_iRtvDescriptorSize);
	}

	// Create the depth/stencil buffer and view.
	D3D12_RESOURCE_DESC depthStencilDesc;
	depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthStencilDesc.Alignment = 0;
	depthStencilDesc.Width = m_iClientWidth;
	depthStencilDesc.Height = m_iClientHeight;
	depthStencilDesc.DepthOrArraySize = 1;
	depthStencilDesc.MipLevels = 1;

	// Correction 11/12/2016: SSAO chapter requires an SRV to the depth buffer to read from 
	// the depth buffer.  Therefore, because we need to create two views to the same resource:
	//   1. SRV format: DXGI_FORMAT_R24_UNORM_X8_TYPELESS
	//   2. DSV Format: DXGI_FORMAT_D24_UNORM_S8_UINT
	// we need to create the depth buffer resource with a typeless format.  
	depthStencilDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;

	depthStencilDesc.SampleDesc.Count = m_b4xMsaaState ? 4 : 1;
	depthStencilDesc.SampleDesc.Quality = m_b4xMsaaState ? (m_i4xMsaaQuality - 1) : 0;
	depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE optClear;
	optClear.Format = m_fDepthStencilFormat;
	optClear.DepthStencil.Depth = 1.0f;
	optClear.DepthStencil.Stencil = 0;
	CD3DX12_HEAP_PROPERTIES cHeapProperties(D3D12_HEAP_TYPE_DEFAULT);
	m_c3dDevice->CreateCommittedResource(&cHeapProperties,D3D12_HEAP_FLAG_NONE,&depthStencilDesc,D3D12_RESOURCE_STATE_COMMON,&optClear,IID_PPV_ARGS(m_cDepthStencilBuffer.GetAddressOf()));

	// Create descriptor to mip level 0 of entire resource using the format of the resource.
	D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
	dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Format = m_fDepthStencilFormat;
	dsvDesc.Texture2D.MipSlice = 0;
	m_c3dDevice->CreateDepthStencilView(m_cDepthStencilBuffer.Get(), &dsvDesc, depthStencilView());

	// Transition the resource from its initial state to be used as a depth buffer.

	CD3DX12_RESOURCE_BARRIER cTransitionBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
		m_cDepthStencilBuffer.Get(),
		D3D12_RESOURCE_STATE_COMMON,
		D3D12_RESOURCE_STATE_DEPTH_WRITE
	);
	m_cCommandList->ResourceBarrier(1, &cTransitionBarrier);

	// Execute the resize commands.
	ThrowIfFailed(m_cCommandList->Close());
	ID3D12CommandList* cmdsLists[] = { m_cCommandList.Get() };
	m_cCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// Wait until resize is complete.
	flushCommandQueue();

	// Update the viewport transform to cover the client area.
	m_vScreenViewport.TopLeftX = 0;
	m_vScreenViewport.TopLeftY = 0;
	m_vScreenViewport.Width = static_cast<float>(m_iClientWidth);
	m_vScreenViewport.Height = static_cast<float>(m_iClientHeight);
	m_vScreenViewport.MinDepth = 0.0f;
	m_vScreenViewport.MaxDepth = 1.0f;

	m_rScissorRect = { 0, 0, m_iClientWidth, m_iClientHeight };
}

void App::flushCommandQueue()
{
	// Advance the fence value to mark commands up to this fence point.
	m_iCurrentFence++;

	// Add an instruction to the command queue to set a new fence point.  Because we 
	// are on the GPU timeline, the new fence point won't be set until the GPU finishes
	// processing all the commands prior to this Signal().
	ThrowIfFailed(m_cCommandQueue->Signal(m_cFence.Get(), m_iCurrentFence));

	// Wait until the GPU has completed commands up to this fence point.
	if (m_cFence->GetCompletedValue() < m_iCurrentFence)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);

		// Fire event when GPU hits current fence.  
		ThrowIfFailed(m_cFence->SetEventOnCompletion(m_iCurrentFence, eventHandle));

		// Wait until the GPU hits current fence event is fired.
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}
}


D3D12_CPU_DESCRIPTOR_HANDLE App::depthStencilView()const
{
	return m_cDsvHeap->GetCPUDescriptorHandleForHeapStart();
}


void App::drawWindow()
{
	// Réutilisez la mémoire associée à l'enregistrement des commandes.
	// Nous ne pouvons réinitialiser que lorsque les listes de commandes associées ont terminé leur exécution sur le GPU.
	ThrowIfFailed(m_cDirectCmdListAlloc->Reset());

	// A command list can be reset after it has been added to the command queue via ExecuteCommandList.
	// Reusing the command list reuses memory.
	ThrowIfFailed(m_cCommandList->Reset(m_cDirectCmdListAlloc.Get(), nullptr));

	// Indicate a state transition on the resource usage.
	CD3DX12_RESOURCE_BARRIER rIntermediate = CD3DX12_RESOURCE_BARRIER::Transition(currentBackBuffer(),D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	m_cCommandList->ResourceBarrier(1, &rIntermediate);

	// Set the viewport and scissor rect.  This needs to be reset whenever the command list is reset.
	m_cCommandList->RSSetViewports(1, &m_vScreenViewport);
	m_cCommandList->RSSetScissorRects(1, &m_rScissorRect);

	// Clear the back buffer and depth buffer.
	m_cCommandList->ClearRenderTargetView(currentBackBufferView(), Colors::LightSteelBlue, 0, nullptr);
	m_cCommandList->ClearDepthStencilView(depthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	// Specify the buffers we are going to render to.
	D3D12_CPU_DESCRIPTOR_HANDLE CurrentBBView = currentBackBufferView();
	D3D12_CPU_DESCRIPTOR_HANDLE DSview = depthStencilView();
	m_cCommandList->OMSetRenderTargets(1, &CurrentBBView, true, &DSview);

	// Indicate a state transition on the resource usage.
	CD3DX12_RESOURCE_BARRIER rValue = CD3DX12_RESOURCE_BARRIER::Transition(currentBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	m_cCommandList->ResourceBarrier(1, &rValue);

	// Done recording commands.
	ThrowIfFailed(m_cCommandList->Close());

	// Add the command list to the queue for execution.
	ID3D12CommandList* cmdsLists[] = { m_cCommandList.Get() };
	m_cCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// swap the back and front buffers
	ThrowIfFailed(m_cSwapChain->Present(0, 0));
	m_iCurrBackBuffer = (m_iCurrBackBuffer + 1) % m_sSwapChainBufferCount;

	// Wait until frame commands are complete.  This waiting is inefficient and is
	// done for simplicity.  Later we will show how to organize our rendering code
	// so we do not have to wait per frame.
	flushCommandQueue();
}


ID3D12Resource* App::currentBackBuffer()const
{
	return m_cSwapChainBuffer[m_iCurrBackBuffer].Get();
}

D3D12_CPU_DESCRIPTOR_HANDLE App::currentBackBufferView()const
{
	return CD3DX12_CPU_DESCRIPTOR_HANDLE(
		m_cRtvHeap->GetCPUDescriptorHandleForHeapStart(),
		m_iCurrBackBuffer,
		m_iRtvDescriptorSize);
}
void App::logAdapters()
{
	UINT i = 0;
	IDXGIAdapter* adapter = nullptr;
	std::vector<IDXGIAdapter*> adapterList;
	while (m_cDxgiFactory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_ADAPTER_DESC desc;
		adapter->GetDesc(&desc);

		std::wstring text = L"***Adapter: ";
		text += desc.Description;
		text += L"\n";

		OutputDebugString(text.c_str());

		adapterList.push_back(adapter);

		++i;
	}

	for (size_t i = 0; i < adapterList.size(); ++i)
	{
		logAdapterOutputs(adapterList[i]);
		ReleaseCom(adapterList[i]);
	}
}

void App::logAdapterOutputs(IDXGIAdapter* adapter)
{
	UINT i = 0;
	IDXGIOutput* output = nullptr;
	while (adapter->EnumOutputs(i, &output) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_OUTPUT_DESC desc;
		output->GetDesc(&desc);

		std::wstring text = L"***Output: ";
		text += desc.DeviceName;
		text += L"\n";
		OutputDebugString(text.c_str());

		logOutputDisplayModes(output, m_fBackBufferFormat);

		ReleaseCom(output);

		++i;
	}
}

void App::logOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format)
{
	UINT count = 0;
	UINT flags = 0;

	// Call with nullptr to get list count.
	output->GetDisplayModeList(format, flags, &count, nullptr);

	std::vector<DXGI_MODE_DESC> modeList(count);
	output->GetDisplayModeList(format, flags, &count, &modeList[0]);

	for (auto& x : modeList)
	{
		UINT n = x.RefreshRate.Numerator;
		UINT d = x.RefreshRate.Denominator;
		std::wstring text =
			L"Width = " + std::to_wstring(x.Width) + L" " +
			L"Height = " + std::to_wstring(x.Height) + L" " +
			L"Refresh = " + std::to_wstring(n) + L"/" + std::to_wstring(d) +
			L"\n";

		::OutputDebugString(text.c_str());
	}
}

void App::update() {

}

App::~App() {

}