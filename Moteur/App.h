#pragma once


#include "Utils.h"

// Link necessary d3d12 libraries.
#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")

class App
{
public:
	App(HINSTANCE hInstance);
	bool initialize();
	bool initMainWindow();
	bool initializeDirect3d();
	void createCommandListQueue();
	void createSwapChain();
	void createRtvAndDsvDescriptorHeaps();
	virtual void onResize();
	void flushCommandQueue();
	int run();
	void logAdapters();
	D3D12_CPU_DESCRIPTOR_HANDLE depthStencilView()const;
	LRESULT messageProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void set4xMsaaState(bool value);
	void logAdapterOutputs(IDXGIAdapter* adapter);
	void logOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format);
	static App* getApp();
	virtual void drawWindow();
	virtual void update();
	ID3D12Resource* currentBackBuffer()const;
	D3D12_CPU_DESCRIPTOR_HANDLE currentBackBufferView()const;
	~App();

protected:
	static App* m_App;
	Microsoft::WRL::ComPtr<ID3D12Device> m_c3dDevice;
	Microsoft::WRL::ComPtr<ID3D12Fence> m_cFence;
	UINT m_iRtvDescriptorSize = 0;
	UINT m_iDsvDescriptorSize = 0;
	UINT m_iCbvSrvDescriptorSize = 0;

	DXGI_FORMAT m_fBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

	bool      m_b4xMsaaState = false;
	UINT      m_i4xMsaaQuality = 0;

	Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_cCommandQueue;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_cDirectCmdListAlloc;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_cCommandList;
	Microsoft::WRL::ComPtr<IDXGISwapChain> m_cSwapChain;

	int m_iClientWidth = 800;
	int m_iClientHeight = 600;

	static const int m_sSwapChainBufferCount = 2;
	HWND      m_hMainWnd = nullptr; // main window handle

	Microsoft::WRL::ComPtr<IDXGIFactory4> m_cDxgiFactory;

	Microsoft::WRL::ComPtr<ID3D12Resource> m_cSwapChainBuffer[m_sSwapChainBufferCount];
	Microsoft::WRL::ComPtr<ID3D12Resource> m_cDepthStencilBuffer;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_cRtvHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_cDsvHeap;

	UINT64 m_iCurrentFence = 0;
	bool      m_bAppPaused = false;
	
	int m_iCurrBackBuffer = 0;
	DXGI_FORMAT m_fDepthStencilFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	
	
	bool      m_bMinimized = false;
	bool      m_bMaximized = false;  // is the application maximized?
	bool      m_bResizing = false;

	HINSTANCE m_hAppInst = nullptr;
	HWND      m_hMainWindow = nullptr;
	std::wstring m_wMainWndCaption = L"d3d App";
	D3D12_VIEWPORT m_vScreenViewport;
	D3D12_RECT m_rScissorRect;
	


	
};

