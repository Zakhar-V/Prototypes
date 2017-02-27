/*template <size_t S> struct GLCommandAllocator
{
typedef void(*FreePfn)(void*);

struct Chunk
{
//FreePfn freeFunc;
Chunk* next;
};

enum : size_t
{
MAX_SIZE = 1024 * 1024,	// 1Mb
ChunkSize = S,
};

static void* Alloc(void)
{
void* _p;
s_size += S;
s_lock.Lock();
if (s_head)
{
_p = s_head;
s_head = s_head->next;
}
else
{
_p = new uint8[S];
}
s_lock.Unlock();
return _p;
}
static void Free(void* _p)
{
if (_p)
{
s_size -= S;
if (s_size > MAX_SIZE)
{
delete[] reinterpret_cast<uint8*>(_p);
}
else
{
s_lock.Lock();
reinterpret_cast<Chunk*>(_p)->next = s_head;
s_head = reinterpret_cast<Chunk*>(_p);
s_lock.Unlock();
}
}
}
static Chunk* s_head;
//static Chunk* s_pool16;
//static Chunk* s_pool32;
static size_t s_size;
static SpinLock s_lock;
};

template <size_t S> typename GLCommandAllocator<S>::Chunk* GLCommandAllocator<S>::s_head = nullptr;
template <size_t S> size_t GLCommandAllocator<S>::s_size = 0;
template <size_t S> SpinLock GLCommandAllocator<S>::s_lock;

template <class T> struct GLCommandPool
{
typedef GLCommandAllocator<sizeof(T) + 16 - sizeof(T) % 16> Allocator;
};

#define GL_COMMAND_ALLOCATOR(T) void* operator new(size_t _s){ return GLCommandPool<T>::Allocator::Alloc(); } \
void operator delete(void* _p) { GLCommandPool<T>::Allocator::Free(_p); }
*/
