#pragma once
#include "App.h"
#include "Utils.h"
#include <WindowsX.h>
#include "UploadBuffer.h"


using Microsoft::WRL::ComPtr;
using namespace std;
using namespace DirectX;

static DirectX::XMFLOAT4X4 Identity4x4()
{
	static DirectX::XMFLOAT4X4 I(
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);

	return I;
}

struct ObjectConstants
{
	XMFLOAT4X4 WorldViewProj = Identity4x4();
};




class TriangleApp : public App
{
public :
	TriangleApp(HINSTANCE hInstance);
	bool initTriangle();
	void buildDescriptorHeaps();
	void buildConstantBuffers();
	void buildRootSignature();
	void buildShadersAndInputLayout();
	void buildBoxGeometry();
	void buildPSO();
	void drawWindow() override;
	void update() override;
	void onResize() override;
	float aspectRatio()const;
private :
	ComPtr<ID3D12DescriptorHeap> m_dCbvHeap = nullptr;
	std::unique_ptr<UploadBuffer<ObjectConstants>> m_uObjectCB = nullptr;
	ComPtr<ID3D12RootSignature> m_dRootSignature = nullptr;
	std::unique_ptr<MeshGeometry> m_mBoxGeo = nullptr;
	ComPtr<ID3DBlob> m_vsByteCode = nullptr;
	ComPtr<ID3DBlob> m_psByteCode = nullptr;
	std::vector<D3D12_INPUT_ELEMENT_DESC> m_vInputLayout;
	ComPtr<ID3D12PipelineState> m_cPSO = nullptr;
	float m_fRadius = 5.0f;
	float m_fTheta = 1.5f * XM_PI;
	float m_fPhi = XM_PIDIV4;
	XMFLOAT4X4 m_fWorld = Identity4x4();
	XMFLOAT4X4 m_fView = Identity4x4();
	XMFLOAT4X4 m_fProj = Identity4x4();
	
};

