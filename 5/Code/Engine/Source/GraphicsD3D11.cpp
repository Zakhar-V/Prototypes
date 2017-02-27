#include "GraphicsD3D11.hpp"
#include "../SDL2/src/video/windows/SDL_windowswindow.h" // for SDL_WindowData

namespace Engine
{
	//----------------------------------------------------------------------------//
	// debug
	//----------------------------------------------------------------------------//

	String HResultToStr(HRESULT _r)
	{
		switch (_r)
		{
		case E_FAIL:
			return "E_FAIL";
		case E_INVALIDARG:
			return "E_INVALIDARG";
		case E_NOINTERFACE:
			return "E_NOINTERFACE";
		case E_NOTIMPL:
			return "E_NOTIMPL";
		case E_OUTOFMEMORY:
			return "E_OUTOFMEMORY";
		case S_OK:
			return "S_OK";
		}
		char _buff[32];
		sprintf(_buff, "0x%08x", _r);
		return _buff;
	}

#if _DEBUG
#	define LOG_HRESULT(r, N) LOG_ERROR("%s error generated in %s [%s]", *HResultToStr(r), N, __FUNCTION__)
#else
#	define LOG_HRESULT(r, N) 
#endif
#define CHECK_HRESULT(r, N) if(r < 0) { LOG_HRESULT(r, N); }
#define CHECK_HRESULT_R(r, N, R) if(r < 0) { LOG_HRESULT(r, N); return R; }

	//----------------------------------------------------------------------------//
	// D3D11Buffer
	//----------------------------------------------------------------------------//

	uint D3D11BufferMiscFlags[] =
	{
		0, // HBT_Vertex
		0, // HBT_Index
		D3D11_RESOURCE_MISC_BUFFER_STRUCTURED, // HBT_Uniform
		D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS, // HBT_DrawIndirect
		0, // HBT_Texture

	};
	uint D3D11BufferBindFlags[] = 
	{
		D3D11_BIND_VERTEX_BUFFER, // HBT_Vertex
		D3D11_BIND_INDEX_BUFFER, // HBT_Index
		D3D11_BIND_CONSTANT_BUFFER | D3D11_BIND_SHADER_RESOURCE, // HBT_Uniform
		D3D11_BIND_SHADER_RESOURCE, // HBT_DrawIndirect
		D3D11_BIND_SHADER_RESOURCE, // HBT_Texture
	};

	uint D3D11MappingMode[] =
	{
		0, // MM_None
		D3D11_MAP_READ, // MM_Read
		D3D11_MAP_WRITE, // MM_Write
		D3D11_MAP_WRITE, // MM_ReadWrite
		D3D11_MAP_WRITE_DISCARD, // MM_WriteDiscard
	};

	//----------------------------------------------------------------------------//
	Ptr<D3D11Buffer> D3D11Buffer::Create(HardwareBufferType _type, HardwareBufferUsage _usage, uint _size, uint _elementSize, const void* _data)
	{
		return new D3D11Buffer(_type, _usage, _size, _elementSize, _data);
	}
	//----------------------------------------------------------------------------//
	D3D11Buffer::D3D11Buffer(HardwareBufferType _type, HardwareBufferUsage _usage, uint _size, uint _elementSize, const void* _data) :
		HardwareBuffer(_type, _usage, _size, _elementSize),
		m_buffer(nullptr),
		m_tempBuffer(nullptr),
		m_mapMode(MM_None),
		m_mapOffset(0),
		m_mapSize(0)
	{
		D3D11_BUFFER_DESC _desc =
		{
			_size,
			_usage == HBU_DynamicWrite ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT,
			D3D11BufferBindFlags[_type],
			_usage == HBU_DynamicWrite ? D3D11_CPU_ACCESS_WRITE : 0u,
			D3D11BufferMiscFlags[_type],
			_elementSize,
		};
		D3D11_SUBRESOURCE_DATA _initData = { _data, 0, 0 };

		HRESULT _r;
		_r = gD3D11Device->CreateBuffer(&_desc, _data ? &_initData : nullptr, &m_buffer);
		CHECK_HRESULT(_r, "ID3D11Device::CreateBuffer");
	}
	//----------------------------------------------------------------------------//
	D3D11Buffer::~D3D11Buffer(void)
	{
		SAFE_RELEASE(m_buffer);
		SAFE_RELEASE(m_tempBuffer);
	}
	//----------------------------------------------------------------------------//
	uint8* D3D11Buffer::Map(MappingMode _mode, uint _offset, uint _size)
	{
		if (_mode == MM_None || m_mapMode != MM_None)
			return nullptr;

		if (_size == 0 || _offset + _size > m_size)
			return nullptr;

		HRESULT _r;
		D3D11_MAPPED_SUBRESOURCE _ptr;

		// do map dynamic buffer
		if (m_usage == HBU_DynamicWrite && !(_mode & MM_Read))
		{
			_r = gD3D11Context->Map(m_buffer, 0, (D3D11_MAP)D3D11MappingMode[_mode], 0, &_ptr);
			if (_r < 0)
			{
				LOG_HRESULT(_r, "ID3D11Context::Map");
				return nullptr;
			}
		}
		else
		{
			// create temp buffer
			if (!m_tempBuffer)
			{
				uint _access = _mode == MM_Read ? D3D11_CPU_ACCESS_READ : D3D11_CPU_ACCESS_WRITE;
				D3D11_BUFFER_DESC _desc =
				{
					m_usage == HBU_DynamicRead ? m_size : _size,
					D3D11_USAGE_STAGING,
					0,
					D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE,
					0,
					0,
				};

				_r = gD3D11Device->CreateBuffer(&_desc, nullptr, &m_tempBuffer);
				if (_r < 0)
				{
					LOG_HRESULT(_r, "ID3D11Device::CreateBuffer");
					return nullptr;
				}
			}

			// update temp buffer
			if (_mode != MM_WriteDiscard)
			{
				D3D11_BOX _src =
				{
					_offset, 0, 0,
					_offset + _size, 1, 1
				};
				gD3D11Context->CopySubresourceRegion(m_tempBuffer, 0, m_usage == HBU_DynamicRead ? _offset : 0, 0, 0, m_buffer, 0, &_src);
			}

			// do map temp buffer
			_r = gD3D11Context->Map(m_tempBuffer, 0, (D3D11_MAP)D3D11MappingMode[_mode == MM_WriteDiscard ? MM_Write : _mode], 0, &_ptr);
			if (_r < 0)
			{
				LOG_HRESULT(_r, "ID3D11Context::Map");

				if (m_usage != HBU_DynamicRead)
				{
					m_tempBuffer->Release();
					m_tempBuffer = nullptr;
				}
				return nullptr;
			}
		}

		m_mapMode = _mode;
		m_mapOffset = _offset;
		m_mapSize = _size;
		return (uint8*)_ptr.pData;
	}
	//----------------------------------------------------------------------------//
	void D3D11Buffer::Unmap(void)
	{
		if (m_mapMode != MM_None)
		{
			ASSERT(m_tempBuffer != nullptr);

			if (m_usage == HBU_DynamicWrite && !(m_mapMode & MM_Read))
			{
				gD3D11Context->Unmap(m_buffer, 0);
			}
			else
			{
				gD3D11Context->Unmap(m_tempBuffer, 0);

				// update buffer
				if (m_mapMode & MM_Write)
				{
					uint _offset = m_usage == HBU_DynamicRead ? m_mapOffset : 0;
					D3D11_BOX _src =
					{
						_offset, 0, 0,
						_offset + m_mapSize, 1, 1
					};
					gD3D11Context->CopySubresourceRegion(m_buffer, 0, m_mapOffset, 0, 0, m_tempBuffer, 0, &_src);
				}

				// delete temp buffer
				if (m_usage != HBU_DynamicRead)
				{
					m_tempBuffer->Release();
					m_tempBuffer = nullptr;
				}
			}

			m_mapMode = MM_None;
		}
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// D3D11VertexFormat
	//----------------------------------------------------------------------------//

	TODO_EX("D3D11VertexFormat", "semantic name");

	struct D3D11VertexAttrib
	{
		const char* name;
		uint index;
	}
	const D3D11VertexAttribs[] =
	{
		{ "", 0 }, // VA_Position
		{ "", 0 }, // VA_Normal
		{ "", 0 }, // VA_Tangent
		{ "", 0 }, // VA_Color
		{ "", 0 }, // VA_BoneIndices
		{ "", 0 }, // VA_BoneWeights
		{ "", 0 }, // VA_TexCoord0
		{ "", 0 }, // VA_TexCoord1
		{ "", 0 }, // VA_TexCoord2
		{ "", 0 }, // VA_TexCoord3
		{ "", 0 }, // VA_Aux0
		{ "", 0 }, // VA_Aux1
		{ "", 0 }, // VA_Aux2
		{ "", 0 }, // VA_Aux3
		{ "", 0 }, // VA_Aux4
		{ "", 0 }, // VA_Aux5
	};

	DXGI_FORMAT D3D11VertexAttribType[] =
	{
		DXGI_FORMAT_UNKNOWN, // VAT_Unknown
		DXGI_FORMAT_R16G16_FLOAT, // VAT_Half2
		DXGI_FORMAT_R16G16B16A16_FLOAT, // VAT_Half4
		DXGI_FORMAT_R32_FLOAT, // VAT_Float
		DXGI_FORMAT_R32G32_FLOAT, // VAT_Float2
		DXGI_FORMAT_R32G32B32_FLOAT, // VAT_Float3
		DXGI_FORMAT_R32G32B32A32_FLOAT, // VAT_Float4
		DXGI_FORMAT_R8G8B8A8_UINT, // VAT_UByte4
		DXGI_FORMAT_R8G8B8A8_UNORM, // VAT_UByte4N
		DXGI_FORMAT_R8G8B8A8_SNORM, // VAT_Byte4N
		DXGI_FORMAT_R16G16_UINT, // VAT_UShort2
		DXGI_FORMAT_R16G16_UNORM, // VAT_UShort2N
		DXGI_FORMAT_R16G16B16A16_UINT, // VAT_UShort4
		DXGI_FORMAT_R16G16B16A16_UNORM, // VAT_UShort4N
		DXGI_FORMAT_R16G16B16A16_SNORM, // VAT_Short4N
	};
	
	HashMap<uint, uint> D3D11VertexFormat::s_indices;
	Array<D3D11VertexFormat*> D3D11VertexFormat::s_instances;
	Mutex D3D11VertexFormat::s_mutex;

	//----------------------------------------------------------------------------//
	D3D11VertexFormat::D3D11VertexFormat(const VertexFormatDesc& _desc) :
		VertexFormat(_desc),
		m_numElements(0)
	{
		memset(m_elements, 0, sizeof(m_elements));

		for (uint i = 0; i < MAX_VERTEX_ATTRIBS; ++i)
		{
			const VertexAttribDesc& _attrib = m_desc[i];
			if (_attrib.type == VAT_Unknown)
				continue;

			D3D11_INPUT_ELEMENT_DESC& _e = m_elements[m_numElements++];
			_e.SemanticName = D3D11VertexAttribs[i].name;
			_e.SemanticIndex = D3D11VertexAttribs[i].index;
			_e.Format = D3D11VertexAttribType[_attrib.type];
			_e.InputSlot = _attrib.stream;
			_e.AlignedByteOffset = _attrib.offset;
			_e.InputSlotClass = _attrib.divisor > 0 ? D3D11_INPUT_PER_INSTANCE_DATA : D3D11_INPUT_PER_VERTEX_DATA;
			_e.InstanceDataStepRate = _attrib.divisor;
		}
	}
	//----------------------------------------------------------------------------//
	D3D11VertexFormat::~D3D11VertexFormat(void)
	{
		for (auto& _layout : m_layouts)
			SAFE_RELEASE(_layout.second);
	}
	//----------------------------------------------------------------------------//
	ID3D11InputLayout* D3D11VertexFormat::GetOrCreateInputLayout(ID3DBlob* _signature)
	{
		ASSERT(_signature != nullptr);

		auto _exists = m_layouts.find(_signature);
		if (_exists != m_layouts.end())
			return _exists->second;

		ID3D11InputLayout* _newLayout = nullptr;

		HRESULT _r = gD3D11Device->CreateInputLayout(m_elements, m_numElements, _signature->GetBufferPointer(), _signature->GetBufferSize(), &_newLayout);
		CHECK_HRESULT(_r, "ID3D11Device::CreateInputLayout");
		
		m_layouts[_signature] = _newLayout;
		return _newLayout;
	}
	//----------------------------------------------------------------------------//
	D3D11VertexFormat* D3D11VertexFormat::AddInstance(const VertexFormatDesc& _desc)
	{
		uint _hash = Crc32(_desc);

		SCOPE_LOCK(s_mutex);

		auto _exists = s_indices.find(_hash);
		if (_exists != s_indices.end())
			return s_instances[_exists->second];

		D3D11VertexFormat* _newFormat = new D3D11VertexFormat(_desc);

		s_indices[_hash] = (uint)s_instances.size();
		s_instances.push_back(_newFormat);

		return _newFormat;
	}
	//----------------------------------------------------------------------------//
	bool D3D11VertexFormat::_InitMgr(void)
	{
		AddInstance(VertexFormatDesc::Empty);

		return true;
	}
	//----------------------------------------------------------------------------//
	void D3D11VertexFormat::_DestroyMgr(void)
	{
		s_instances.clear();
		s_indices.clear();
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// D3D11ShaderCompiler
	//----------------------------------------------------------------------------//

	const char* D3D11ShaderProfile[] = 
	{
		"vs", // SST_Vertex
		"ps", // SST_Fragment
		"gs", // SST_Geometry
	};

	GUID _IID_ID3D11ShaderReflection = { 0x0a233719, 0x3960, 0x4578,{ 0x9d, 0x7c, 0x20, 0x3b, 0x8b, 0x1d, 0x9c, 0xc1 } };

	//----------------------------------------------------------------------------//
	D3D11ShaderCompiler::D3D11ShaderCompiler(void)
	{
	}
	//----------------------------------------------------------------------------//
	D3D11ShaderCompiler::~D3D11ShaderCompiler(void)
	{
	}
	//----------------------------------------------------------------------------//
	bool D3D11ShaderCompiler::_Init(void)
	{
		// load library

		const char* _names[] = 
		{
			"d3dcompiler.dll",
			"d3dcompiler_47.dll",
			"d3dcompiler_43.dll", // in Microsoft DirectX SDK (June 2010)
			nullptr,
		};

		HMODULE _d3dcompiler = nullptr;
		for (uint i = 0; _names[i]; ++i)
		{
			_d3dcompiler = LoadLibraryA(_names[i]);
			if (_d3dcompiler)
			{
				char _buff[1024];
				uint _length = GetModuleFileNameA(_d3dcompiler, _buff, sizeof(_buff));
				_buff[_length] = 0;
				m_libPath = _buff;
				m_libName = _names[i];
				break;
			}
		}

		if (!_d3dcompiler)
		{
			LOG_ERROR("Couldn't find D3Dcompiler.dll");
			return false;
		}

		LOG_INFO("Shader compiler: '%s'", *m_libPath);

		// load functions

		_D3DCompile = (pD3DCompile)GetProcAddress(_d3dcompiler, "D3DCompile");
		_D3DReflect = (decltype(_D3DReflect))GetProcAddress(_d3dcompiler, "D3DReflect");

		return true;
	}
	//----------------------------------------------------------------------------//
	void D3D11ShaderCompiler::_Destroy(void)
	{
	}
	//----------------------------------------------------------------------------//
	HRESULT STDMETHODCALLTYPE D3D11ShaderCompiler::Open(THIS_ D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes)
	{
		return S_OK;
	}
	//----------------------------------------------------------------------------//
	HRESULT STDMETHODCALLTYPE D3D11ShaderCompiler::Close(THIS_ LPCVOID pData)
	{
		return S_OK;
	}
	//----------------------------------------------------------------------------//
	bool D3D11ShaderCompiler::Compile(const String& _src, const String& _srcName, const String& _entry, ShaderStageType _type, uint _version, const Array<StringPair>& _defines, uint _flags, Ptr<ID3DBlob>& _microcode, String* _log)
	{
		Array<D3D_SHADER_MACRO> _d3ddefines;
		for (const auto& _def : _defines)
		{
			_d3ddefines.push_back({ _def.first, _def.second });
		}

		String _marker = "COMPILE" + String::ToLower(D3D11ShaderProfile[_type]);
		_d3ddefines.push_back({ _marker, nullptr });

		// ...
		_d3ddefines.push_back({ nullptr, nullptr });

		String _profile = String::Format("%s_%d_%d", D3D11ShaderProfile[_type], _version / 10, _version % 10);
		String _entryPoint = _entry.NonEmpty() ? _entry : "main";
		Ptr<ID3DBlob> _errors;
						 
		HRESULT _r = _D3DCompile(_src.Ptr(), _src.Length(), _srcName, &_d3ddefines[0], this, _entryPoint, _profile, _flags, 0, &_microcode._Ptr(), &_errors._Ptr());
		
		if (_log && _errors)
			_log->Clear().Append((const char*)_errors->GetBufferPointer(), (int)_errors->GetBufferSize());

		return _r >= 0;
	}
	//----------------------------------------------------------------------------//
	Ptr<ID3D11ShaderReflection> D3D11ShaderCompiler::GetRefletion(ID3DBlob* _microcode)
	{
		Ptr<ID3D11ShaderReflection> _result;
		if (_microcode)
			_D3DReflect(_microcode->GetBufferPointer(), _microcode->GetBufferSize(), _IID_ID3D11ShaderReflection, (void**)&_result._Ptr());
		return _result;
	}
	//----------------------------------------------------------------------------//
	void D3D11ShaderCompiler::Compile(Task& _task)
	{
		Array<D3D_SHADER_MACRO> _defines;
		//D3DCompile()
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// D3D11ShaderStage
	//----------------------------------------------------------------------------//


	//----------------------------------------------------------------------------//
	// D3D11RenderContext
	//----------------------------------------------------------------------------//
	
	//----------------------------------------------------------------------------//
	D3D11RenderContext::D3D11RenderContext(ID3D11DeviceContext* _context, bool _deferred) :
		RenderContext(_deferred),
		m_context(_context),
		m_currentVertexFormat(nullptr),
		m_vertexFormatUpdated(false),
		m_currentPrimitiveType(PT_Points)
	{
	}
	//----------------------------------------------------------------------------//
	D3D11RenderContext::~D3D11RenderContext(void)
	{
		m_context->Release();
	}
	//----------------------------------------------------------------------------//
	void D3D11RenderContext::BeginCommands(void)
	{
		//m_context->Begin();
	}
	//----------------------------------------------------------------------------//
	void D3D11RenderContext::EndCommands(void)
	{
		//m_context->End();
	}
	//----------------------------------------------------------------------------//
	void D3D11RenderContext::SetVertexFormat(VertexFormat* _format)
	{
		TODO("Check parameters");
		if (!_format)
		{

		}
		if (m_currentVertexFormat != _format)
		{
			m_currentVertexFormat = static_cast<D3D11VertexFormat*>(_format);
			m_vertexFormatUpdated = false;
		}
	}
	//----------------------------------------------------------------------------//
	void D3D11RenderContext::SetVertexBuffer(uint _slot, HardwareBuffer* _buffer, uint _offset, uint _stride)
	{
		TODO("Check parameters");
		ID3D11Buffer* _handle = static_cast<D3D11Buffer*>(_buffer)->GetBuffer();
		m_context->IASetVertexBuffers(_slot, 1, &_handle, &_stride, &_offset);
	}
	//----------------------------------------------------------------------------//
	void D3D11RenderContext::SetIndexBuffer(HardwareBuffer* _buffer, IndexFormat _format, uint _offset)
	{
		TODO("Check parameters");
	}
	//----------------------------------------------------------------------------//
	void D3D11RenderContext::SetPrimitiveType(PrimitiveType _type)
	{
		m_currentPrimitiveType = _type;
		// ...
	}
	//----------------------------------------------------------------------------//
	void D3D11RenderContext::Draw(uint _numVertices, uint _numInstances, uint _firstVertex, uint _baseInstance)
	{
		TODO("Check parameters");

		_ApplyChanges();
		m_context->DrawInstanced(_numVertices, _numInstances, _firstVertex, _baseInstance);
	}
	//----------------------------------------------------------------------------//
	void D3D11RenderContext::DrawIndexed(uint _numIndices, uint _numInstances, uint _firstIndex, int _baseVertex, uint _baseInstance)
	{
		TODO("Check parameters");

		_ApplyChanges();
		m_context->DrawIndexedInstanced(_numIndices, _numInstances, _firstIndex, _baseVertex, _baseInstance);
	}
	//----------------------------------------------------------------------------//
	void D3D11RenderContext::DrawIndirect(HardwareBuffer* _buffer, uint _offset)
	{
		TODO("Check parameters");

		_ApplyChanges();
		m_context->DrawInstancedIndirect(static_cast<D3D11Buffer*>(_buffer)->GetBuffer(), _offset);
	}
	//----------------------------------------------------------------------------//
	void D3D11RenderContext::DrawIndexedIndirect(HardwareBuffer* _buffer, uint _offset)
	{
		TODO("Check parameters");

		_ApplyChanges();
		m_context->DrawIndexedInstancedIndirect(static_cast<D3D11Buffer*>(_buffer)->GetBuffer(), _offset);
	}
	//----------------------------------------------------------------------------//
	void D3D11RenderContext::_ApplyChanges(void)
	{
		// ...

		if (!m_vertexFormatUpdated)
		{
			m_vertexFormatUpdated = true;
			//m_context->IASetInputLayout(m_currentVertexFormat->GetOrCreateInputLayout());
		}
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// D3D11RenderSystem
	//----------------------------------------------------------------------------//
	
	void _CreateD3D11RenderSystem(void)
	{
		new D3D11RenderSystem;
	}

	//----------------------------------------------------------------------------//
	D3D11RenderSystem::D3D11RenderSystem(void) :
		m_windowHandle(nullptr),
		m_swapchain(nullptr),
		m_device(nullptr),
		m_context(nullptr)
	{
		m_features.type = RST_D3D11;
		m_features.shaderLang = NSL_HLSL;
	}
	//----------------------------------------------------------------------------//
	D3D11RenderSystem::~D3D11RenderSystem(void)
	{
	}
	//----------------------------------------------------------------------------//
	bool D3D11RenderSystem::_InitDriver(void)
	{
		LOG_EVENT("Initialize D3D11RenderSystem");

		// create window
		m_window = SDL_CreateWindow("D3D11Window", 0, 0, 1, 1, SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN);
		if (!m_window)
		{
			LOG_ERROR("Couldn't create window : %s", SDL_GetError());
			return false;
		}

		// get window handle
		SDL_WindowData* _wd = (SDL_WindowData*)m_window->driverdata;
		m_windowHandle = _wd->hwnd;

		// create factory
		HMODULE _dxgi = LoadLibraryA("dxgi.dll");
		if (!_dxgi)
		{
			LOG_ERROR("dxgi.dll not was found");
			return false;
		}

		HRESULT(WINAPI* _CreateDXGIFactory)(REFIID riid, void **ppFactory) = (HRESULT(WINAPI*)(REFIID, void**))GetProcAddress(_dxgi, "CreateDXGIFactory");
		if (!_CreateDXGIFactory)
		{
			LOG_ERROR("No CreateDXGIFactory in dxgi.dll");
			return false;
		}

		Ptr<IDXGIFactory> _factory;
		if (_CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&_factory._Ptr()) < 0)
		{
			LOG_ERROR("Couldn't create IDXGIFactory");
			return false;
		}

		// get adapter and output
		Ptr<IDXGIAdapter> _adapter;
		if (_factory->EnumAdapters(0, &_adapter._Ptr()) < 0)
		{
			LOG_ERROR("Couldn't get default adapter");
			return false;
		}

		Ptr<IDXGIOutput> _output;
		if (_adapter->EnumOutputs(0, &_output._Ptr()) < 0)
		{
			LOG_ERROR("Couldn't get default monitor");
			return false;
		}

		// create device and swapchain
		D3D_FEATURE_LEVEL _desiredLevels[] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1 };
		D3D_FEATURE_LEVEL _level;
		DXGI_SWAP_CHAIN_DESC _desc;
		ZeroMemory(&_desc, sizeof(_desc));
		_desc.BufferDesc.Width = 0;
		_desc.BufferDesc.Height = 0;
		_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		_desc.BufferDesc.RefreshRate.Numerator = 60; // ...
		_desc.BufferDesc.RefreshRate.Denominator = 1;
		_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // DXGI_USAGE_BACK_BUFFER;
		_desc.BufferCount = 1;
		_desc.OutputWindow = m_windowHandle;
		_desc.Windowed = true;
		_desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		_desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
		_desc.SampleDesc.Count = 1;
		_desc.SampleDesc.Quality = 0;

		HMODULE _d3d11 = LoadLibraryA("d3d11.dll");
		if (!_d3d11)
		{
			LOG_ERROR("d3d11.dll not was found");
			return false;
		}

		PFN_D3D11_CREATE_DEVICE_AND_SWAP_CHAIN _D3D11CreateDeviceAndSwapChain = (PFN_D3D11_CREATE_DEVICE_AND_SWAP_CHAIN)GetProcAddress(_d3d11, "D3D11CreateDeviceAndSwapChain");
		if (!_D3D11CreateDeviceAndSwapChain)
		{
			LOG_ERROR("No D3D11CreateDeviceAndSwapChain in d3d11.dll");
			return false;
		}

		uint _deviceFlags = 0; //D3D11_CREATE_DEVICE_DEBUG
		ID3D11DeviceContext* _context = nullptr;
		HRESULT _r = _D3D11CreateDeviceAndSwapChain(_adapter, D3D_DRIVER_TYPE_UNKNOWN, nullptr, _deviceFlags, _desiredLevels, 2, D3D11_SDK_VERSION, &_desc, &m_swapchain, &m_device, &_level, &_context);
		if (_r < 0)
		{
			LOG_ERROR("Couldn't create d3d11 device");
			return false;
		}

		_factory->MakeWindowAssociation(m_windowHandle, DXGI_MWA_NO_WINDOW_CHANGES | DXGI_MWA_NO_ALT_ENTER);

		// create context
		m_context = new D3D11RenderContext(_context, false);

		// features
		switch (_level)
		{
		case D3D_FEATURE_LEVEL_10_1:
			m_features.version = RSV_10_1;
			m_features.shaderVersion = 41;
			break;

		case D3D_FEATURE_LEVEL_11_0:
			m_features.version = RSV_11_0;
			m_features.shaderVersion = 50;
			break;

		default:
			LOG_ERROR("Unknown feature level");
			return false;
		}

		DXGI_ADAPTER_DESC _ad;
		_adapter->GetDesc(&_ad);
		m_features.adapterName = String::EncodeUtf8(_ad.Description);
		m_features.dedicatedVideoMemory = _ad.DedicatedVideoMemory;

		m_features.maxVertexStreams = 16;
		m_features.maxVertexAttribDivisor = (uint)-1;

		LOG_INFO("D3D feature level: %d.%d", m_features.version / 10, m_features.version % 10);
		LOG_INFO("Adapter: %s", *m_features.adapterName);
		LOG_INFO("Dedicated video memory: %d mb", m_features.dedicatedVideoMemory / (1024 * 1024));

		// create shader compiler
		new D3D11ShaderCompiler;
		if (!D3D11ShaderCompiler::Get()->_Init())
		{
			LOG_ERROR("Couldn't initialize shader compiler");
			D3D11ShaderCompiler::Get()->_Destroy();
			delete D3D11ShaderCompiler::Get();
			return false;
		}

		// initialize vertex format manager
		if (!D3D11VertexFormat::_InitMgr())
		{
			LOG_ERROR("Couldn't initialize vertex format manager");
			return false;
		}


		return true;
	}
	//----------------------------------------------------------------------------//
	void D3D11RenderSystem::_DestroyDriver(void)
	{
		// ...

		D3D11VertexFormat::_DestroyMgr();

		if (D3D11ShaderCompiler::Get())
		{
			D3D11ShaderCompiler::Get()->_Destroy();
			delete D3D11ShaderCompiler::Get();
		}

		if (m_context)
		{
			delete m_context;
		}

		SAFE_RELEASE(m_device);
		SAFE_RELEASE(m_swapchain);
	}
	//----------------------------------------------------------------------------//
	VertexFormat* D3D11RenderSystem::AddVertexFormat(const VertexFormatDesc& _desc)
	{
		return D3D11VertexFormat::AddInstance(_desc);
	}
	//----------------------------------------------------------------------------//
	HardwareBufferPtr D3D11RenderSystem::CreateBuffer(HardwareBufferType _type, HardwareBufferUsage _usage, uint _size, uint _elementSize, const void* _data)
	{
		if (_type == HBT_Texture)
			return nullptr;

		return D3D11Buffer::Create(_type, _usage, _size, _elementSize, _data).Get();
	}
	//----------------------------------------------------------------------------//
	void D3D11RenderSystem::BeginCommands(void)
	{
		m_context->D3D11RenderContext::BeginCommands();
	}
	//----------------------------------------------------------------------------//
	void D3D11RenderSystem::EndCommands(void)
	{
		m_context->D3D11RenderContext::EndCommands();
	}
	//----------------------------------------------------------------------------//
	void D3D11RenderSystem::SetVertexFormat(VertexFormat* _format)
	{
		m_context->D3D11RenderContext::SetVertexFormat(_format);
	}
	//----------------------------------------------------------------------------//
	void D3D11RenderSystem::SetVertexBuffer(uint _slot, HardwareBuffer* _buffer, uint _offset, uint _stride)
	{
		m_context->D3D11RenderContext::SetVertexBuffer(_slot, _buffer, _offset, _stride);
	}
	//----------------------------------------------------------------------------//
	void D3D11RenderSystem::SetIndexBuffer(HardwareBuffer* _buffer, IndexFormat _format, uint _offset)
	{
		m_context->D3D11RenderContext::SetIndexBuffer(_buffer, _format, _offset);
	}
	//----------------------------------------------------------------------------//
	void D3D11RenderSystem::SetPrimitiveType(PrimitiveType _type)
	{
		m_context->D3D11RenderContext::SetPrimitiveType(_type);
	}
	//----------------------------------------------------------------------------//
	void D3D11RenderSystem::Draw(uint _numVertices, uint _numInstances, uint _firstVertex, uint _baseInstance)
	{
		m_context->D3D11RenderContext::Draw(_numVertices, _numInstances, _firstVertex, _baseInstance);
	}
	//----------------------------------------------------------------------------//
	void D3D11RenderSystem::DrawIndexed(uint _numIndices, uint _numInstances, uint _firstIndex, int _baseVertex, uint _baseInstance)
	{
		m_context->D3D11RenderContext::DrawIndexed(_numIndices, _numInstances, _firstIndex, _baseVertex, _baseInstance);
	}
	//----------------------------------------------------------------------------//
	void D3D11RenderSystem::DrawIndirect(HardwareBuffer* _buffer, uint _offset)
	{
		m_context->D3D11RenderContext::DrawIndirect(_buffer, _offset);
	}
	//----------------------------------------------------------------------------//
	void D3D11RenderSystem::DrawIndexedIndirect(HardwareBuffer* _buffer, uint _offset)
	{
		m_context->D3D11RenderContext::DrawIndexedIndirect(_buffer, _offset);
	}
	//----------------------------------------------------------------------------//

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//
}
