#pragma once

#include "../Graphics.hpp"
#include <SDL.h>
#pragma warning(disable : 4005)
#include <d3d11.h>
#include <dxgi.h>
#include <D3Dcompiler.h>

namespace Engine
{
	//----------------------------------------------------------------------------//
	// Defs
	//----------------------------------------------------------------------------//

#define gD3D11RenderSystem D3D11RenderSystem::Get()
#define gD3D11Device gD3D11RenderSystem->GetDevice()
#define gD3D11Context gD3D11RenderSystem->GetContext()

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//

	/*enum D3D11RenderCommand
	{
		// [buffer]
		D3D11RC_InitBuffer,
		D3D11RC_DestroyBuffer,
		D3D11RC_
	};

	class D3D11RenderThread
	{
	public:

	protected:
	};*/

	//----------------------------------------------------------------------------//
	// D3D11Buffer
	//----------------------------------------------------------------------------//

	class D3D11Buffer final : public HardwareBuffer
	{
	public:

		static Ptr<D3D11Buffer> Create(HardwareBufferType _type, HardwareBufferUsage _usage, uint _size, uint _elementSize, const void* _data);
		
		uint8* Map(MappingMode _mode, uint _offset, uint _size) override;
		void Unmap(void) override;

		ID3D11Buffer* GetBuffer(void) { return m_buffer; }

	protected:
		friend class D3D11RenderSystem;

		D3D11Buffer(HardwareBufferType _type, HardwareBufferUsage _usage, uint _size, uint _elementSize, const void* _data);
		~D3D11Buffer(void);

		ID3D11Buffer* m_buffer;
		ID3D11Buffer* m_tempBuffer;
		MappingMode m_mapMode;
		uint m_mapOffset;
		uint m_mapSize;
	};

	//----------------------------------------------------------------------------//
	// D3D11VertexFormat
	//----------------------------------------------------------------------------//

	class D3D11VertexFormat : public VertexFormat
	{
	public:

		static D3D11VertexFormat* AddInstance(const VertexFormatDesc& _desc);

		ID3D11InputLayout* GetOrCreateInputLayout(ID3DBlob* _signature);

	protected:

		D3D11VertexFormat(const VertexFormatDesc& _desc);
		~D3D11VertexFormat(void);

		D3D11_INPUT_ELEMENT_DESC m_elements[MAX_VERTEX_ATTRIBS];
		uint m_numElements;
		HashMap<void*, ID3D11InputLayout*> m_layouts;

	protected: // manager
		friend class D3D11RenderSystem;

		static bool _InitMgr(void);
		static void _DestroyMgr(void);

		static HashMap<uint, uint> s_indices; // <crc32, index>
		static Array<D3D11VertexFormat*> s_instances;
		static Mutex s_mutex;
	};

	//----------------------------------------------------------------------------//
	// D3D11ShaderCompiler
	//----------------------------------------------------------------------------//

	class D3D11ShaderCompiler final : public Singleton<D3D11ShaderCompiler>, public ID3DInclude
	{
	public:

		struct Task
		{
			// [in]

			String src;
			String srcName;
			String profile;
			String entry;
			Array<StringPair> defines;
			uint flags = 0;

			// [out]
			bool result = false;
			Condition done;
			String log;
			Ptr<ID3DBlob> microcode;
		};

		bool Compile(const String& _src, const String& _srcName, const String& _entry, ShaderStageType _type, uint _version, const Array<StringPair>& _defines, uint _flags, Ptr<ID3DBlob>& _microcode, String* _log);

		Ptr<ID3D11ShaderReflection> GetRefletion(ID3DBlob* _microcode);

		void Compile(Task& _task);

		void CompileAsync();

	protected:
		friend class D3D11RenderSystem;

		D3D11ShaderCompiler(void);
		~D3D11ShaderCompiler(void);
		bool _Init(void);
		void _Destroy(void);

		virtual HRESULT STDMETHODCALLTYPE Open(THIS_ D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID *ppData, UINT *pBytes) override;
		virtual HRESULT STDMETHODCALLTYPE Close(THIS_ LPCVOID pData) override;

		String m_libName;
		String m_libPath;
		pD3DCompile _D3DCompile;
		decltype(D3DReflect)* _D3DReflect;
	};

	//----------------------------------------------------------------------------//
	// D3D11ShaderStage
	//----------------------------------------------------------------------------//

	class D3D11ShaderStage : public HardwareShaderStage
	{
	public:

	protected:
	};

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//

	typedef Ptr<class D3D11ShaderFile> D3D11ShaderFilePtr;

	class D3D11ShaderBinary : public RefCounted
	{
	public:

	protected:

		time_t m_fileTime;
	};

	class D3D11ShaderFile : public RefCounted
	{
	public:

	protected:

		String m_rawSource;
		String m_preparedSource;

		time_t m_fileTime;
		time_t m_compileTime;
		Array<D3D11ShaderFilePtr> m_includes;
		HashSet<D3D11ShaderFile*> m_dependents;
	};

	class D3D11ShaderDefines
	{
	public:

	protected:
	};

	//----------------------------------------------------------------------------//
	// D3D11RenderContext
	//----------------------------------------------------------------------------//

	class D3D11RenderContext final : public RenderContext
	{
	public:
		ID3D11DeviceContext* GetContext(void) { return m_context; }
		
		void BeginCommands(void) override;
		void EndCommands(void) override;

		void SetVertexFormat(VertexFormat* _format) override;
		void SetVertexBuffer(uint _slot, HardwareBuffer* _buffer, uint _offset, uint _stride) override;
		void SetIndexBuffer(HardwareBuffer* _buffer, IndexFormat _format, uint _offset) override;
		void SetPrimitiveType(PrimitiveType _type) override;

		void Draw(uint _numVertices, uint _numInstances, uint _firstVertex, uint _baseInstance) override;
		void DrawIndexed(uint _numIndices, uint _numInstances, uint _firstIndex, int _baseVertex, uint _baseInstance) override;
		void DrawIndirect(HardwareBuffer* _buffer, uint _offset) override;
		void DrawIndexedIndirect(HardwareBuffer* _buffer, uint _offset) override;

	protected:
		friend class D3D11RenderSystem;

		D3D11RenderContext(ID3D11DeviceContext* _context, bool _deferred);
		~D3D11RenderContext(void);

		void _ApplyChanges(void);

		ID3D11DeviceContext* m_context;

		D3D11VertexFormat* m_currentVertexFormat;
		bool m_vertexFormatUpdated;
		PrimitiveType m_currentPrimitiveType;
	};

	//----------------------------------------------------------------------------//
	// D3D11RenderSystem
	//----------------------------------------------------------------------------//

	class D3D11RenderSystem final : public RenderSystem
	{
	public:
		static D3D11RenderSystem* Get(void) { return static_cast<D3D11RenderSystem*>(s_instance); }

		D3D11RenderSystem(void);
		~D3D11RenderSystem(void);

		ID3D11Device* GetDevice(void) { return m_device; }
		ID3D11DeviceContext* GetContext(void) { return m_context->GetContext(); }
		
		VertexFormat* AddVertexFormat(const VertexFormatDesc& _desc) override;

		HardwareBufferPtr CreateBuffer(HardwareBufferType _type, HardwareBufferUsage _usage, uint _size, uint _elementSize, const void* _data = nullptr) override;



		void BeginCommands(void) override;
		void EndCommands(void) override;

		void SetVertexFormat(VertexFormat* _format) override;
		void SetVertexBuffer(uint _slot, HardwareBuffer* _buffer, uint _offset, uint _stride) override;
		void SetIndexBuffer(HardwareBuffer* _buffer, IndexFormat _format, uint _offset) override;
		void SetPrimitiveType(PrimitiveType _type) override;

		void Draw(uint _numVertices, uint _numInstances, uint _firstVertex, uint _baseInstance) override;
		void DrawIndexed(uint _numIndices, uint _numInstances, uint _firstIndex, int _baseVertex, uint _baseInstance) override;
		void DrawIndirect(HardwareBuffer* _buffer, uint _offset) override;
		void DrawIndexedIndirect(HardwareBuffer* _buffer, uint _offset) override;

	protected:

		bool _InitDriver(void) override;
		void _DestroyDriver(void) override;

		HWND m_windowHandle;
		IDXGISwapChain* m_swapchain;
		ID3D11Device* m_device;
		//ID3D11DeviceContext* m_context;
		D3D11RenderContext* m_context;
	};

	//----------------------------------------------------------------------------//
	//
	//----------------------------------------------------------------------------//
}
