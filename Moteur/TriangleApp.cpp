#include "TriangleApp.h"

#include "Utils.h"
#include "Camera.h"

#include "UploadBuffer.h"

const float Pi = 3.1415926535f;
struct Vertex
{
	XMFLOAT3 Pos;
	XMFLOAT4 Color;
};

TriangleApp::TriangleApp(HINSTANCE hInstance) : App(hInstance)
{
	m_oCamera = new Camera();
	m_oCamera->initCamera(m_iClientWidth, m_iClientHeight);
}

bool TriangleApp::initTriangle() {
	
	if (!initialize()) {
		return false;
	}
	ThrowIfFailed(m_cCommandList->Reset(m_cDirectCmdListAlloc.Get(), nullptr));

	buildDescriptorHeaps();
	buildConstantBuffers();
	buildRootSignature();
	buildShadersAndInputLayout();
	buildBoxGeometry();
	buildPSO();

	// Execute the initialization commands.
	ThrowIfFailed(m_cCommandList->Close());
	ID3D12CommandList* cmdsLists[] = { m_cCommandList.Get() };
	m_cCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// Wait until initialization is complete.
	flushCommandQueue();

	return true;


	
}

void TriangleApp::buildDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc;
	cbvHeapDesc.NumDescriptors = 1;
	cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	cbvHeapDesc.NodeMask = 0;
	ThrowIfFailed(m_c3dDevice->CreateDescriptorHeap(&cbvHeapDesc,
		IID_PPV_ARGS(&m_dCbvHeap)));
}

void TriangleApp::buildConstantBuffers()
{
	m_uObjectCB = std::make_unique<UploadBuffer<ObjectConstants>>(m_c3dDevice.Get(), 1, true);

	UINT objCBByteSize = CalcConstantBufferByteSize(sizeof(ObjectConstants));

	D3D12_GPU_VIRTUAL_ADDRESS cbAddress = m_uObjectCB->Resource()->GetGPUVirtualAddress();
	// Offset to the ith object constant buffer in the buffer.
	int boxCBufIndex = 0;
	cbAddress += boxCBufIndex * objCBByteSize;

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
	cbvDesc.BufferLocation = cbAddress;
	cbvDesc.SizeInBytes = CalcConstantBufferByteSize(sizeof(ObjectConstants));

	m_c3dDevice->CreateConstantBufferView(
		&cbvDesc,
		m_dCbvHeap->GetCPUDescriptorHandleForHeapStart());
}

void TriangleApp::buildRootSignature()
{
	// Shader programs typically require resources as input (constant buffers,
	// textures, samplers).  The root signature defines the resources the shader
	// programs expect.  If we think of the shader programs as a function, and
	// the input resources as function parameters, then the root signature can be
	// thought of as defining the function signature.  

	// Root parameter can be a table, root descriptor or root constants.
	CD3DX12_ROOT_PARAMETER slotRootParameter[1];

	// Create a single descriptor table of CBVs.
	CD3DX12_DESCRIPTOR_RANGE cbvTable;
	cbvTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	slotRootParameter[0].InitAsDescriptorTable(1, &cbvTable);

	// A root signature is an array of root parameters.
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(1, slotRootParameter, 0, nullptr,
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	if (errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	ThrowIfFailed(m_c3dDevice->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(&m_dRootSignature)));
}

void TriangleApp::buildShadersAndInputLayout()
{
	HRESULT hr = S_OK;

	m_vsByteCode = Utils::CompileShader(L"Shaders\\color.hlsl", nullptr, "VS", "vs_5_0");
	m_psByteCode = Utils::CompileShader(L"Shaders\\color.hlsl", nullptr, "PS", "ps_5_0");

	m_vInputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};
}


void TriangleApp::buildBoxGeometry()
{
	std::array<Vertex, 3> vertices =
	{
		Vertex({ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(Colors::White) }),
		Vertex({ XMFLOAT3(-1.0f, +1.0f, -1.0f), XMFLOAT4(Colors::Black) }),
		Vertex({ XMFLOAT3(+1.0f, +1.0f, -1.0f), XMFLOAT4(Colors::Red) }),
		
	};

	std::array<std::uint16_t, 36> indices =
	{
		// front face
		0, 1, 2,
		//0, 2, 3,

		//// back face
		//5, 2, 1,
		//2, 5, 3,

		//// left face
		//5, 1, 0,
		//0, 3, 5,

		//////// right face
		//3, 2, 6,
		//3, 6, 7,

		//////// top face
		//1, 5, 6,
		//1, 6, 2,

		//////// bottom face
		//4, 0, 3,
		//4, 3, 7
	};

	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	m_mBoxGeo = std::make_unique<MeshGeometry>();
	m_mBoxGeo->Name = "boxGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &m_mBoxGeo->VertexBufferCPU));
	CopyMemory(m_mBoxGeo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &m_mBoxGeo->IndexBufferCPU));
	CopyMemory(m_mBoxGeo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	m_mBoxGeo->VertexBufferGPU = Utils::CreateDefaultBuffer(m_c3dDevice.Get(),
		m_cCommandList.Get(), vertices.data(), vbByteSize, m_mBoxGeo->VertexBufferUploader);

	m_mBoxGeo->IndexBufferGPU = Utils::CreateDefaultBuffer(m_c3dDevice.Get(),
		m_cCommandList.Get(), indices.data(), ibByteSize, m_mBoxGeo->IndexBufferUploader);

	m_mBoxGeo->VertexByteStride = sizeof(Vertex);
	m_mBoxGeo->VertexBufferByteSize = vbByteSize;
	m_mBoxGeo->IndexFormat = DXGI_FORMAT_R16_UINT;
	m_mBoxGeo->IndexBufferByteSize = ibByteSize;

	SubmeshGeometry submesh;
	submesh.IndexCount = (UINT)indices.size();
	submesh.StartIndexLocation = 0;
	submesh.BaseVertexLocation = 0;

	m_mBoxGeo->DrawArgs["box"] = submesh;
}

void TriangleApp::buildPSO()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
	ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	psoDesc.InputLayout = { m_vInputLayout.data(), (UINT)m_vInputLayout.size() };
	psoDesc.pRootSignature = m_dRootSignature.Get();
	psoDesc.VS =
	{
		reinterpret_cast<BYTE*>(m_vsByteCode->GetBufferPointer()),
		m_vsByteCode->GetBufferSize()
	};
	psoDesc.PS =
	{
		reinterpret_cast<BYTE*>(m_psByteCode->GetBufferPointer()),
		m_psByteCode->GetBufferSize()
	};
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = m_fBackBufferFormat;
	psoDesc.SampleDesc.Count = m_b4xMsaaState ? 4 : 1;
	psoDesc.SampleDesc.Quality = m_b4xMsaaState ? (m_i4xMsaaQuality - 1) : 0;
	psoDesc.DSVFormat = m_fDepthStencilFormat;
	ThrowIfFailed(m_c3dDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_cPSO)));
}

void TriangleApp::update()
{
	
	XMMATRIX mMatrixView = m_oCamera->update();
	// Convert Spherical to Cartesian coordinates.
	float x = m_fRadius * sinf(m_fPhi) * cosf(m_fTheta);
	float z = m_fRadius * sinf(m_fPhi) * sinf(m_fTheta);
	float y = m_fRadius * cosf(m_fPhi);

	// Build the view matrix.
	XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX view = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&m_fView, view);

	XMMATRIX world = XMLoadFloat4x4(&m_fWorld);
	XMMATRIX proj = XMLoadFloat4x4(&m_fProj);
	XMMATRIX worldViewProj = world * mMatrixView * proj;

	// Update the constant buffer with the latest worldViewProj matrix.
	ObjectConstants objConstants;
	XMStoreFloat4x4(&objConstants.WorldViewProj, XMMatrixTranspose(worldViewProj));
	m_uObjectCB->CopyData(0, objConstants);
}

void TriangleApp::drawWindow()
{
	// Reuse the memory associated with command recording.
	// We can only reset when the associated command lists have finished execution on the GPU.
	ThrowIfFailed(m_cDirectCmdListAlloc->Reset());

	// A command list can be reset after it has been added to the command queue via ExecuteCommandList.
	// Reusing the command list reuses memory.
	ThrowIfFailed(m_cCommandList->Reset(m_cDirectCmdListAlloc.Get(), m_cPSO.Get()));

	m_cCommandList->RSSetViewports(1, &m_vScreenViewport);
	m_cCommandList->RSSetScissorRects(1, &m_rScissorRect);

	// Indicate a state transition on the resource usage.
	CD3DX12_RESOURCE_BARRIER iValue5 = CD3DX12_RESOURCE_BARRIER::Transition(currentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
	m_cCommandList->ResourceBarrier(1, &iValue5);

	// Clear the back buffer and depth buffer.
	m_cCommandList->ClearRenderTargetView(currentBackBufferView(), Colors::LightSteelBlue, 0, nullptr);
	m_cCommandList->ClearDepthStencilView(depthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	// Specify the buffers we are going to render to.
	D3D12_CPU_DESCRIPTOR_HANDLE iValue1 = currentBackBufferView();
	D3D12_CPU_DESCRIPTOR_HANDLE iValue2 = depthStencilView();
	m_cCommandList->OMSetRenderTargets(1, &iValue1, true, &iValue2);

	ID3D12DescriptorHeap* descriptorHeaps[] = { m_dCbvHeap.Get() };
	m_cCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	m_cCommandList->SetGraphicsRootSignature(m_dRootSignature.Get());
	D3D12_VERTEX_BUFFER_VIEW iValue3 = m_mBoxGeo->VertexBufferView();
	m_cCommandList->IASetVertexBuffers(0, 1, &iValue3);
	D3D12_INDEX_BUFFER_VIEW iValue4 = m_mBoxGeo->IndexBufferView();
	m_cCommandList->IASetIndexBuffer(&iValue4);
	m_cCommandList->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	m_cCommandList->SetGraphicsRootDescriptorTable(0, m_dCbvHeap->GetGPUDescriptorHandleForHeapStart());

	m_cCommandList->DrawIndexedInstanced(
		m_mBoxGeo->DrawArgs["box"].IndexCount,
		1, 0, 0, 0);

	// Indicate a state transition on the resource usage.
	CD3DX12_RESOURCE_BARRIER iValue6 = CD3DX12_RESOURCE_BARRIER::Transition(currentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	m_cCommandList->ResourceBarrier(1, &iValue6);

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

void TriangleApp::onResize()
{
	App::onResize();

	// The window resized, so update the aspect ratio and recompute the projection matrix.
	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f * Pi, aspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&m_fProj, P);
}


float TriangleApp::aspectRatio()const
{
	return static_cast<float>(m_iClientWidth) / m_iClientHeight;
}
