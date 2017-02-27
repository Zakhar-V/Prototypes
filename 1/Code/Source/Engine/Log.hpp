#pragma once

#include "String.hpp"

namespace ge
{
	//----------------------------------------------------------------------------//
	// Defs
	//----------------------------------------------------------------------------//

#define LOG_INFO(msg, ...) ge::LogNode::Message(ge::LL_Info, msg, ##__VA_ARGS__)
#define LOG_WARNING(msg, ...) ge::LogNode::Message(ge::LL_Warning, msg, ##__VA_ARGS__)
#define LOG_ERROR(msg, ...)	ge::LogNode::Message(ge::LL_Error, msg, ##__VA_ARGS__)
#define LOG_FATAL(msg, ...)	ge::LogNode::Message(ge::LL_Fatal, msg, ##__VA_ARGS__)
#define LOG_DEBUG(msg, ...) ge::LogNode::Message(ge::LL_Debug, msg, ##__VA_ARGS__)
#define LOG_NODE(title) ge::LogNode _logNode_(true, __FUNCTION__, title)
#define LOG_NODE_DEFERRED(title) ge::LogNode _logNode_(false, __FUNCTION__, title)
#define LOG_DISABLE(mask) ge::LogNode::SetMask(ge::LogNode::GetMask() & ~mask)
#define LOG_ENABLE(mask) ge::LogNode::SetMask(ge::LogNode::GetMask() | mask)

	enum LogLevel : uint
	{
		LL_Info = 0x1,
		LL_Warning = 0x2,
		LL_Error = 0x4,
		LL_Fatal = 0x8,
		LL_Debug = 0x10,
		LL_RenderDebugOutput = 0x20,

		LL_All = 0xffff,
	};

	//----------------------------------------------------------------------------//
	// LogNode
	//----------------------------------------------------------------------------//

	class ENGINE_API LogNode : public NonCopyable
	{
	public:

		LogNode(bool _writeNow, const char* _func, const char* _title = "", uint _mask = LL_All);
		~LogNode(void);

		const char* GetTitle(void) { return m_title; }
		const char* GetFunc(void) { return m_func; }

		static void Message(int _level, const char* _msg, ...);
		static LogNode* GetTop(void);
		static void SetMask(uint _mask);
		static uint GetMask(void);

	private:
		void _Write(void);

		LogNode* m_prev;
		const char* m_func;
		const char* m_title;
		uint m_mask;
		int m_depth;
		bool m_written;
	};

	//----------------------------------------------------------------------------//
	// 
	//----------------------------------------------------------------------------//
}
