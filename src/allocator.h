#ifndef ALLOCATOR_H_
#define ALLOCATOR_H_

#include "platform.h"
#include <stdlib.h>

namespace ncnn {

// the alignment of all the allocated buffers
#if NCNN_AVX512
#define NCNN_MALLOC_ALIGN 64
#elif NCNN_AVX
#define NCNN_MALLOC_ALIGN 32
#else
#define NCNN_MALLOC_ALIGN 16
#endif

// we have some optimized kernels that may overread buffer a bit in loop
// it is common to interleave next-loop data load with arithmetic instructions
// allocating more bytes keeps us safe from SEGV_ACCERR failure
#define NCNN_MALLOC_OVERREAD 64

template <typename _Tp>
static inline _Tp *alignPtr(_Tp *ptr, int n = (int)sizeof(_Tp)) {
  return (_Tp *)(((size_t)ptr + n - 1) & -n);
}

// Aligns a buffer size to the specified number of bytes
// The function returns the minimum number that is greater or equal to sz and is
// divisible by n sz Buffer size to align n Alignment size that must be a power
// of two
static inline size_t alignSize(size_t sz, int n) { return (sz + n - 1) & -n; }

static inline void *fastMalloc(size_t size) {
#if _MSC_VER
  return _aligned_malloc(size, NCNN_MALLOC_ALIGN);
#elif (defined(__unix__) || defined(__APPLE__)) &&                             \
        _POSIX_C_SOURCE >= 200112L ||                                          \
    (__ANDROID__ && __ANDROID_API__ >= 17)
  void *ptr = 0;
  if (posix_memalign(&ptr, NCNN_MALLOC_ALIGN, size + NCNN_MALLOC_OVERREAD))
    ptr = 0;
  return ptr;
#elif __ANDROID__ && __ANDROID_API__ < 17
  return memalign(NCNN_MALLOC_ALIGN, size + NCNN_MALLOC_OVERREAD);
#else
  unsigned char *udata = (unsigned char *)malloc(
      size + sizeof(void *) + NCNN_MALLOC_ALIGN + NCNN_MALLOC_OVERREAD);
  if (!udata)
    return 0;
  unsigned char **adata =
      alignPtr((unsigned char **)udata + 1, NCNN_MALLOC_ALIGN);
  adata[-1] = udata;
  return adata;
#endif
}

inline void fastFree(void *ptr) {
  if (ptr) {
#if _MSC_VER
    _aligned_free(ptr);
#elif (defined(__unix__) || defined(__APPLE__)) &&                             \
        _POSIX_C_SOURCE >= 200112L ||                                          \
    (__ANDROID__ && __ANDROID_API__ >= 17)
    free(ptr);
#elif __ANDROID__ && __ANDROID_API__ < 17
    free(ptr);
#else
    unsigned char *udata = ((unsigned char **)ptr)[-1];
    free(udata);
#endif
  }
}

#if NCNN_THREADS
// exchange-add operation for atomic operations on reference counters
#if defined __riscv && !defined __riscv_atomic
// riscv target without A extension
 NCNN_FORCEINLINE int NCNN_XADD(int *addr, int delta) {
  int tmp = *addr;
  *addr += delta;
  return tmp;
}
#elif defined __INTEL_COMPILER && !(defined WIN32 || defined _WIN32)
// atomic increment on the linux version of the Intel(tm) compiler
#define NCNN_XADD(addr, delta)                                                 \
  (int)_InterlockedExchangeAdd(                                                \
      const_cast<void *>(reinterpret_cast<volatile void *>(addr)), delta)
#elif defined __GNUC__
#if defined __clang__ && __clang_major__ >= 3 && !defined __ANDROID__ &&       \
    !defined __EMSCRIPTEN__ && !defined(__CUDACC__)
#ifdef __ATOMIC_ACQ_REL
#define NCNN_XADD(addr, delta)                                                 \
  __c11_atomic_fetch_add((_Atomic(int) *)(addr), delta, __ATOMIC_ACQ_REL)
#else
#define NCNN_XADD(addr, delta)                                                 \
  __atomic_fetch_add((_Atomic(int) *)(addr), delta, 4)
#endif
#else
#if defined __ATOMIC_ACQ_REL && !defined __clang__
// version for gcc >= 4.7
#define NCNN_XADD(addr, delta)                                                 \
  (int)__atomic_fetch_add((unsigned *)(addr), (unsigned)(delta),               \
                          __ATOMIC_ACQ_REL)
#else
#define NCNN_XADD(addr, delta)                                                 \
  (int)__sync_fetch_and_add((unsigned *)(addr), (unsigned)(delta))
#endif
#endif
#elif defined _MSC_VER && !defined RC_INVOKED
#define NCNN_XADD(addr, delta)                                                 \
  (int)_InterlockedExchangeAdd((long volatile *)addr, delta)
#else
// thread-unsafe branch
 NCNN_FORCEINLINE int NCNN_XADD(int *addr, int delta) {
  int tmp = *addr;
  *addr += delta;
  return tmp;
}
#endif
#else  // NCNN_THREADS
NCNN_FORCEINLINE int NCNN_XADD(int *addr, int delta) {
  int tmp = *addr;
  *addr += delta;
  return tmp;
}
#endif // NCNN_THREADS

class NCNN_EXPORT Allocator {
public:
  virtual ~Allocator();
  virtual void *fastMalloc(size_t size) = 0;
  virtual void fastFree(void *ptr) = 0;
};

class PoolAllocatorPrivate;
class NCNN_EXPORT PoolAllocator : public Allocator {
public:
  PoolAllocator();
  ~PoolAllocator();

  // ratio range 0 ~ 1
  // default cr = 0
  void set_size_compare_ratio(float scr);

  // budget drop threshold
  // default threshold = 10
  void set_size_drop_threshold(size_t);

  // release all budgets immediately
  void clear();

  virtual void *fastMalloc(size_t size);
  virtual void fastFree(void *ptr);

private:
  PoolAllocator(const PoolAllocator &);
  PoolAllocator &operator=(const PoolAllocator &);

private:
  PoolAllocatorPrivate *const d;
};

class UnlockedPoolAllocatorPrivate;
class NCNN_EXPORT UnlockedPoolAllocator : public Allocator {
public:
  UnlockedPoolAllocator();
  ~UnlockedPoolAllocator();

  // ratio range 0 ~ 1
  // default cr = 0
  void set_size_compare_ratio(float scr);

  // budget drop threshold
  // default threshold = 10
  void set_size_drop_threshold(size_t);

  // release all budgets immediately
  void clear();

  virtual void *fastMalloc(size_t size);
  virtual void fastFree(void *ptr);

private:
  UnlockedPoolAllocator(const UnlockedPoolAllocator &);
  UnlockedPoolAllocator &operator=(const UnlockedPoolAllocator &);

private:
  UnlockedPoolAllocatorPrivate *const d;
};

} // namespace ncnn

#endif