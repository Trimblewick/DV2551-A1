#include "DX12Renderer.h"


DX12Renderer::DX12Renderer()
{

}

DX12Renderer::~DX12Renderer()
{
}


LRESULT CALLBACK DX12Renderer::EventHandler(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	//If one case is hit the code will execute everything down until a break;
	int stopper = 0;
	switch (message)
	{

	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE) {
			if (MessageBox(0, L"Are you sure you want to exit?",
				L"Really?", MB_YESNO | MB_ICONQUESTION) == IDYES)
			{
				DestroyWindow(hWnd);
			}
		}
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

Material * DX12Renderer::makeMaterial(const std::string & name)
{
	return nullptr;
}

Mesh * DX12Renderer::makeMesh()
{
	return nullptr;
}

VertexBuffer * DX12Renderer::makeVertexBuffer(size_t size, VertexBuffer::DATA_USAGE usage)
{
	return nullptr;
}

Texture2D * DX12Renderer::makeTexture2D()
{
	return nullptr;
}

Sampler2D * DX12Renderer::makeSampler2D()
{
	return nullptr;
}

RenderState * DX12Renderer::makeRenderState()
{
	return nullptr;
}

std::string DX12Renderer::getShaderPath()
{
	return std::string();
}

std::string DX12Renderer::getShaderExtension()
{
	return std::string();
}

ConstantBuffer * DX12Renderer::makeConstantBuffer(std::string NAME, unsigned int location)
{
	return nullptr;
}

Technique * DX12Renderer::makeTechnique(Material *, RenderState *)
{
	return nullptr;
}

int DX12Renderer::initialize(unsigned int width, unsigned int height)
{
	m_pWindow = new Window(width, height, EventHandler);
	m_pD3DFactory = new D3DFactory();

	m_pCommandQueue = m_pD3DFactory->CreateCQ();

	DXGI_MODE_DESC descMode = {};
	descMode.Width = width;
	descMode.Height = height;
	descMode.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	DXGI_SAMPLE_DESC descSample = {};
	descSample.Count = 1;

	DXGI_SWAP_CHAIN_DESC descSwapChain = {};
	descSwapChain.BufferCount = g_iBackBufferCount;
	descSwapChain.BufferDesc = descMode;
	descSwapChain.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	descSwapChain.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	descSwapChain.OutputWindow = m_pWindow->GetWindowHandle();
	descSwapChain.SampleDesc = descSample;
	descSwapChain.Windowed = true;

	m_pSwapChain = m_pD3DFactory->CreateSwapChain(descSwapChain, m_pCommandQueue);

	m_pDHrenderTargets = m_pD3DFactory->CreateDH(g_iBackBufferCount, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, false);
	int iDHIncrementSize = m_pD3DFactory->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	D3D12_CPU_DESCRIPTOR_HANDLE handleDH = m_pDHrenderTargets->GetCPUDescriptorHandleForHeapStart();

	for (int i = 0; i < g_iBackBufferCount; ++i)
	{
		m_pFenceValues[i] = 0;
		m_ppFenceFrame[i] = m_pD3DFactory->CreateFence();
		m_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&m_ppRenderTargets[i]));

		m_pD3DFactory->GetDevice()->CreateRenderTargetView(m_ppRenderTargets[i], nullptr, handleDH);
		handleDH.ptr += iDHIncrementSize;
		
		m_ppCommandAllocators[i] = m_pD3DFactory->CreateCA();
		m_ppCommandLists[i] = m_pD3DFactory->CreateCL(m_ppCommandAllocators[i]);
		m_ppCommandLists[i]->Close();
	}



	//Create resources
	ID3DBlob* pVSblob = m_pD3DFactory->CompileShader(L"VertexShader.hlsl", "main", "vs_5_1");
	ID3DBlob* pPSblob = m_pD3DFactory->CompileShader(L"PixelShader.hlsl", "main", "ps_5_1");

	std::vector<D3D12_ROOT_PARAMETER> rootParameters;

	D3D12_ROOT_PARAMETER tempRP;
	tempRP.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	


	D3D12_ROOT_SIGNATURE_DESC descRS = {};
	//descRS.NumParameters = 0;// rootParameters.size();
	//descRS.pParameters = 0;// rootParameters.data();
	//descRS.NumStaticSamplers = 1;
	//descRS.pStaticSamplers = &CD3DX12_STATIC_SAMPLER_DESC(0);

	m_pRS = m_pD3DFactory->CreateRS(&descRS);


	D3D12_GRAPHICS_PIPELINE_STATE_DESC descPSO = {};
	descPSO.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	descPSO.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	descPSO.DSVFormat = DXGI_FORMAT_D32_FLOAT;
	descPSO.NumRenderTargets = 1;
	descPSO.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	descPSO.VS = { pVSblob->GetBufferPointer(), pVSblob->GetBufferSize() };
	descPSO.PS = { pPSblob->GetBufferPointer(), pPSblob->GetBufferSize() };
	descPSO.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	descPSO.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	descPSO.SampleDesc = descSample;
	descPSO.SampleMask = 0xffffffff;
	descPSO.pRootSignature = m_pRS;
	
	
	m_pPSO = m_pD3DFactory->CreatePSO(&descPSO);

	return 0;
}

void DX12Renderer::setWinTitle(const char * title)
{
	size_t wn = mbsrtowcs(NULL, &title, 0, NULL);
	wchar_t * buf = new wchar_t[wn + 1]();
	mbsrtowcs(buf, &title, wn + 1, NULL);
	

	m_pWindow->SetTitle(buf);
	delete[] buf;
}

void DX12Renderer::present()
{
	int iFrameIndex = m_pSwapChain->GetCurrentBackBufferIndex();
	m_ppCommandLists[iFrameIndex]->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_ppRenderTargets[iFrameIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
	m_ppCommandLists[iFrameIndex]->Close();

	ID3D12CommandList* ppCLs[] = { m_ppCommandLists[iFrameIndex] };
	m_pCommandQueue->ExecuteCommandLists(1, ppCLs);
	m_pCommandQueue->Signal(m_ppFenceFrame[iFrameIndex], m_pFenceValues[iFrameIndex]);

	m_pSwapChain->Present(0, 0);

	
	iFrameIndex = m_pSwapChain->GetCurrentBackBufferIndex();
	if (m_ppFenceFrame[iFrameIndex]->GetCompletedValue() < m_pFenceValues[iFrameIndex])
	{
		m_ppFenceFrame[iFrameIndex]->SetEventOnCompletion(m_pFenceValues[iFrameIndex], m_handleFence);
		WaitForSingleObject(m_handleFence, INFINITE);
	}
	m_pFenceValues[iFrameIndex]++;
}

int DX12Renderer::shutdown()
{
	for (int i = 0; i < g_iBackBufferCount; ++i)
	{
		m_pFenceValues[i]++;											
		m_pCommandQueue->Signal(m_ppFenceFrame[i], m_pFenceValues[i]);

		m_ppFenceFrame[i]->SetEventOnCompletion(m_pFenceValues[i], m_handleFence);
		WaitForSingleObject(m_handleFence, INFINITE);
	}

	for (int i = 0; i < g_iBackBufferCount; ++i)
	{
		SAFE_RELEASE(m_ppCommandAllocators[i]);
		SAFE_RELEASE(m_ppCommandLists[i]);
		SAFE_RELEASE(m_ppFenceFrame[i]);
		SAFE_RELEASE(m_ppRenderTargets[i]);
		
	}
	SAFE_RELEASE(m_pPSO);
	SAFE_RELEASE(m_pRS);
	SAFE_RELEASE(m_pSwapChain);
	SAFE_RELEASE(m_pCommandQueue);

	if (m_pD3DFactory)
	{
		delete m_pD3DFactory;
		m_pD3DFactory = nullptr;
	}
	if (m_pWindow)
	{
		delete m_pWindow;
		m_pWindow = nullptr;
	}
	return 0;
}

void DX12Renderer::setClearColor(float r, float g, float b, float a)
{
	m_pClearColor[0] = r;
	m_pClearColor[1] = g;
	m_pClearColor[2] = b;
	m_pClearColor[3] = a;
}

void DX12Renderer::clearBuffer(unsigned int flag = -1)
{
	int iFrameIndex = m_pSwapChain->GetCurrentBackBufferIndex();

	HRESULT hr = m_ppCommandAllocators[iFrameIndex]->Reset();
	hr = m_ppCommandLists[iFrameIndex]->Reset(m_ppCommandAllocators[iFrameIndex], nullptr);
	
	m_ppCommandLists[iFrameIndex]->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_ppRenderTargets[iFrameIndex], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	int iIncrementSizeRTV = m_pD3DFactory->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	D3D12_CPU_DESCRIPTOR_HANDLE handleDH = m_pDHrenderTargets->GetCPUDescriptorHandleForHeapStart();
	handleDH.ptr += iIncrementSizeRTV * iFrameIndex;

	m_ppCommandLists[iFrameIndex]->ClearRenderTargetView(handleDH, m_pClearColor, NULL, nullptr);
}

void DX12Renderer::setRenderState(RenderState * ps)
{

}

void DX12Renderer::submit(Mesh * mesh)
{
}

void DX12Renderer::frame()
{
	int iFrameIndex = m_pSwapChain->GetCurrentBackBufferIndex();
	
	//ID3D12DescriptorHeap* ppDescriptorHeaps[] = { m_pDHrenderTargets };
	//m_ppCommandLists[iFrameIndex]->SetDescriptorHeaps(1, ppDescriptorHeaps);
	int iIncrementSizeRTV = m_pD3DFactory->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	D3D12_CPU_DESCRIPTOR_HANDLE handleDH = m_pDHrenderTargets->GetCPUDescriptorHandleForHeapStart();
	handleDH.ptr += iIncrementSizeRTV * iFrameIndex;

	//m_ppCommandLists[iFrameIndex]->OMSetRenderTargets(1, &handleDH, NULL, nullptr);
	//m_ppCommandLists[iFrameIndex]->SetGraphicsRootSignature(m_pRS);
	//m_ppCommandLists[iFrameIndex]->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	//m_ppCommandLists[iFrameIndex]->DrawInstanced(4, 1, 0, 0);
}
