#include "stdafx.h"

#include "D3D11Hook.h"

#include <D3D11.h>
#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")

#include "Module.h"
#include "VMTHook.h"

VMTHook *D3D11SwapChainVMTHook;

typedef HRESULT(__stdcall* D3D11Present_t)(IDXGISwapChain*, UINT, UINT);
D3D11Present_t D3D11Present_o;

const std::string shaderDefines =
R"(
struct VertexInputType
{
	float4 position : POSITION;
	float2 tex : TEXCOORD0;
};

struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
};
)";

const std::string pixelShaderProgram =
R"(
Texture2D shaderTexture;
SamplerState sampleType;
float4 MyPixelShader(PixelInputType input) : SV_TARGET
{
	float4 textureColor;
	textureColor = shaderTexture.Sample(sampleType, input.tex);
	return textureColor;
}
)";

const std::string vertexShaderProgram =
R"(
PixelInputType MyVertexShader(VertexInputType input)
{
	PixelInputType output;

	output.position = input.position;
	output.tex = input.tex;

	return output;
}
)";

ID3D11DeviceContext *deferredContext;
ID3D11Texture2D *texture;
ID3D11ShaderResourceView *shaderResourceView;
ID3D11VertexShader *vertexShader;
ID3D11PixelShader *pixelShader;
ID3D11Buffer *vertexBuffer, *indexBuffer;

struct Vertex
{
	float x, y, z; // Pos
	float u, v; // Tex
};

bool fillTexture()
{
	D3D11_MAPPED_SUBRESOURCE mappedTexture;

	HRESULT hr = deferredContext->Map(texture, D3D11CalcSubresource(0, 0, 1), D3D11_MAP_WRITE_DISCARD, 0, &mappedTexture);

	if (FAILED(hr))
	{
		return false;
	}

	memset(mappedTexture.pData, 0xFF, 640 * 480 * 4);

	deferredContext->Unmap(texture, D3D11CalcSubresource(0, 0, 1));

	return true;
}

bool D3D11Init(ID3D11Device *device, IDXGISwapChain *swapChain)
{

	HRESULT hr;

	// Our deferred Renderer
	hr = device->CreateDeferredContext(0, &deferredContext);

	if (FAILED(hr))
	{
		return false;
	}

	ID3D11Texture2D *backBuffer;
	hr = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);

	if (FAILED(hr))
	{
		return false;
	}

	D3D11_VIEWPORT viewPort
	{
		0.0f, 0.0f,
		640.0f, 480.0f,
		0.0f, 1.0f
	};
	deferredContext->RSSetViewports(1, &viewPort);

	ID3D11RenderTargetView *renderTargetView;
	hr = device->CreateRenderTargetView(backBuffer, 0, &renderTargetView);

	if (FAILED(hr))
	{
		return false;
	}

	deferredContext->OMSetRenderTargets(1, &renderTargetView, 0);

	D3D11_BLEND_DESC blend;
	memset(&blend, 0, sizeof(blend));
	blend.RenderTarget[0].BlendEnable = TRUE;
	blend.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	blend.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blend.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blend.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blend.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
	blend.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blend.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	ID3D11BlendState *blendState;
	device->CreateBlendState(&blend, &blendState);

	// Compile vertex shader and pixel shader
	ID3DBlob *compiledVertexShader, *vertexShaderError, *compiledPixelShader, *pixelShaderError;

	// Append the struct definitions
	std::string
		_vertexShader = shaderDefines + vertexShaderProgram,
		_pixelShader = shaderDefines + pixelShaderProgram;

	DebugBreak();

	hr = D3DCompile(_vertexShader.c_str(), _vertexShader.length(), nullptr, nullptr, nullptr, "MyVertexShader", "vs_5_0", 0, 0, &compiledVertexShader, &vertexShaderError);
	if (FAILED(hr))
	{
		return false;
	}

	hr = D3DCompile(_pixelShader.c_str(), _pixelShader.length(), nullptr, nullptr, nullptr, "MyPixelShader", "ps_5_0", 0, 0, &compiledPixelShader, &pixelShaderError);
	if (FAILED(hr))
	{
		return false;
	}

	hr = device->CreateVertexShader(compiledVertexShader->GetBufferPointer(), compiledVertexShader->GetBufferSize(),
		nullptr, &vertexShader);
	if (FAILED(hr))
	{
		return false;
	}

	hr = device->CreatePixelShader(compiledPixelShader->GetBufferPointer(), compiledPixelShader->GetBufferSize(),
		nullptr, &pixelShader);
	if (FAILED(hr))
	{
		return false;
	}

	// Vertex input layout

	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	ID3D11InputLayout *inputLayout;
	hr = device->CreateInputLayout(layout, 2, compiledVertexShader->GetBufferPointer(), compiledVertexShader->GetBufferSize(), &inputLayout);

	if (FAILED(hr))
	{
		return false;
	}

	deferredContext->IASetInputLayout(inputLayout);

	// Create vertexbuffer and indexbuffer
	Vertex vertices[] = 
	{
		{-1.0f,	-1.0f, 1.0f, 0.0f, 0.0f},
		{1.0f, -1.0f, 1.0f, 1.0f, 0.0f},
		{1.0f, 1.0f, 1.0f, 0.0f, 1.0f},
		{-1.0f, 1.0f, 1.0f, 1.0f, 1.0f}
	};

	D3D11_BUFFER_DESC bufferDesc = {0};
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = 4 * sizeof(Vertex);
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	
	D3D11_SUBRESOURCE_DATA initData = {0};
	initData.pSysMem = vertices;

	hr = device->CreateBuffer(&bufferDesc, &initData, &vertexBuffer);
	if (FAILED(hr))
	{
		return false;
	}

	DWORD indices[] =
	{
		0, 1, 3,
		1, 2, 3
	};

	bufferDesc = {0};
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	bufferDesc.ByteWidth = 6 * sizeof(DWORD);
	bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDesc.CPUAccessFlags = 0;

	initData = {0};
	initData.pSysMem = indices;

	hr = device->CreateBuffer(&bufferDesc, &initData, &indexBuffer);

	if (FAILED(hr))
	{
		return false;
	}

	deferredContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	deferredContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Create Texture
	
	D3D11_TEXTURE2D_DESC textureDesc = {0};

	textureDesc.Width = 640;
	textureDesc.Height = 480;
	textureDesc.MipLevels = textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DYNAMIC;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	hr = device->CreateTexture2D(&textureDesc, nullptr, &texture);

	if (FAILED(hr))
	{
		return false;
	}

	fillTexture();

	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	ZeroMemory(&shaderResourceViewDesc, sizeof(shaderResourceViewDesc));
	shaderResourceViewDesc.Format = textureDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = textureDesc.MipLevels;

	hr = device->CreateShaderResourceView(texture, &shaderResourceViewDesc, &shaderResourceView);

	if (FAILED(hr))
	{
		return false;
	}
	

	return true;
}



HRESULT __stdcall D3D11Present_hk(IDXGISwapChain *pSwapChain, UINT SyncInterval, UINT Flags)
{
	__asm pushad

	ID3D11Device *device;
	pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&device);
	ID3D11DeviceContext *deviceContext;
	device->GetImmediateContext(&deviceContext);

	static bool once = false;
	if (!once)
	{
		D3D11Init(device, pSwapChain);
		once = true;
	}

	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	deferredContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);

	deferredContext->VSSetShader(vertexShader, nullptr, 0);
	deferredContext->GSSetShader(nullptr, nullptr, 0);
	deferredContext->PSSetShaderResources(0, 1, &shaderResourceView);
	deferredContext->PSSetShader(pixelShader, nullptr, 1);
	deferredContext->DrawIndexed(6, 0, 0);
	
	ID3D11CommandList *commandList;
	deferredContext->FinishCommandList(TRUE, &commandList);
	
	deviceContext->ExecuteCommandList(commandList, TRUE);

	commandList->Release();
	deviceContext->Release();
	device->Release();

	__asm popad;

	return D3D11Present_o(pSwapChain, SyncInterval, Flags);
}

struct D3D11Structures
{
	IDXGISwapChain *swapChain;
	ID3D11Device *device;
	ID3D11DeviceContext *deviceContext;
};

D3D11Structures getSimpleSampleD3D11Structures()
{
	D3D11Structures d3d11Structures;

	DWORD_PTR moduleBase = Module(L"SimpleSample11.exe").getModuleAddress();
	DWORD_PTR offset = 0x68E2C;

	DWORD_PTR ptrToState = moduleBase + offset;
	DWORD_PTR state = *(DWORD_PTR*)ptrToState;

	d3d11Structures.device = *(ID3D11Device**)(state + 0x188);
	d3d11Structures.swapChain = *(IDXGISwapChain**)(state + 0x16C);
	d3d11Structures.deviceContext = *(ID3D11DeviceContext**)(state + 0x18C);

	Logger::getInstance() << "getSimpleSampleD3D11Structures():\n"
		<< "pSwapChain = " << d3d11Structures.swapChain << "\n"
		<< "device = " << d3d11Structures.device << "\n"
		<< "deviceContext = " << d3d11Structures.deviceContext << "\n";

	return d3d11Structures;
}

void HookD3D11()
{
	D3D11Structures d3d11Structures = getSimpleSampleD3D11Structures();

	D3D11SwapChainVMTHook = new VMTHook((DWORD_PTR)d3d11Structures.swapChain);
	D3D11Present_o = (D3D11Present_t) D3D11SwapChainVMTHook->exchangeFunction(8, (DWORD_PTR)&D3D11Present_hk);
	D3D11SwapChainVMTHook->hookVMT();

}

void UnhookD3D11()
{
	D3D11SwapChainVMTHook->unhookVMT();

	delete D3D11SwapChainVMTHook;
}
