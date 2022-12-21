#include "allocator.h"

namespace ncnn {

Allocator::~Allocator() {}

class PoolAllocatorPrivate {
public:
  Mutex budgets_lock;
  Mutex payouts_lock;
  unsigned int size_compare_ratio; // 0~256
  size_t size_drop_threshold;
  std::list<std::pair<size_t, void *>> budgets;
  std::list<std::pair<size_t, void *>> payouts;
};

PoolAllocator::PoolAllocator() : Allocator(), d(new PoolAllocatorPrivate) {
  d->size_compare_ratio = 0;
  d->size_drop_threshold = 10;
}

PoolAllocator::~PoolAllocator() {
  clear();

  if (!d->payouts.empty()) {
    NCNN_LOGE("FATAL ERROR! pool allocator destroyed too early");
#if NCNN_STDIO
    std::list<std::pair<size_t, void *>>::iterator it = d->payouts.begin();
    for (; it != d->payouts.end(); ++it) {
      void *ptr = it->second;
      NCNN_LOGE("%p still in use", ptr);
    }
#endif
  }

  delete d;
}

PoolAllocator::PoolAllocator(const PoolAllocator &) : d(0) {}

PoolAllocator &PoolAllocator::operator=(const PoolAllocator &) { return *this; }

void PoolAllocator::clear() {
  d->budgets_lock.lock();

  std::list<std::pair<size_t, void *>>::iterator it = d->budgets.begin();
  for (; it != d->budgets.end(); ++it) {
    void *ptr = it->second;
    ncnn::fastFree(ptr);
  }
  d->budgets.clear();

  d->budgets_lock.unlock();
}

void PoolAllocator::set_size_compare_ratio(float scr) {
  if (scr < 0.f || scr > 1.f) {
    NCNN_LOGE("invalid size compare ratio %f", scr);
    return;
  }

  d->size_compare_ratio = (unsigned int)(scr * 256);
}

void PoolAllocator::set_size_drop_threshold(size_t threshold) {
  d->size_drop_threshold = threshold;
}

void *PoolAllocator::fastMalloc(size_t size) {
  d->budgets_lock.lock();

  // find free budget
  std::list<std::pair<size_t, void *>>::iterator it = d->budgets.begin(),
                                                 it_max = d->budgets.begin(),
                                                 it_min = d->budgets.begin();
  for (; it != d->budgets.end(); ++it) {
    size_t bs = it->first;

    // size_compare_ratio ~ 100%
    if (bs >= size && ((bs * d->size_compare_ratio) >> 8) <= size) {
      void *ptr = it->second;

      d->budgets.erase(it);

      d->budgets_lock.unlock();

      d->payouts_lock.lock();

      d->payouts.push_back(std::make_pair(bs, ptr));

      d->payouts_lock.unlock();

      return ptr;
    }

    if (bs < it_min->first) {
      it_min = it;
    }
    if (bs > it_max->first) {
      it_max = it;
    }
  }

  if (d->budgets.size() >= d->size_drop_threshold) {
    // All chunks in pool are not chosen. Then try to drop some outdated
    // chunks and return them to OS.
    if (it_max->first < size) {
      // Current query is asking for a chunk larger than any cached chunks.
      // Then remove the smallest one.
      ncnn::fastFree(it_min->second);
      d->budgets.erase(it_min);
    } else if (it_min->first > size) {
      // Current query is asking for a chunk smaller than any cached chunks.
      // Then remove the largest one.
      ncnn::fastFree(it_max->second);
      d->budgets.erase(it_max);
    }
  }

  d->budgets_lock.unlock();

  // new
  void *ptr = ncnn::fastMalloc(size);

  d->payouts_lock.lock();

  d->payouts.push_back(std::make_pair(size, ptr));

  d->payouts_lock.unlock();

  return ptr;
}

void PoolAllocator::fastFree(void *ptr) {
  d->payouts_lock.lock();

  // return to budgets
  std::list<std::pair<size_t, void *>>::iterator it = d->payouts.begin();
  for (; it != d->payouts.end(); ++it) {
    if (it->second == ptr) {
      size_t size = it->first;

      d->payouts.erase(it);

      d->payouts_lock.unlock();

      d->budgets_lock.lock();

      d->budgets.push_back(std::make_pair(size, ptr));

      d->budgets_lock.unlock();

      return;
    }
  }

  d->payouts_lock.unlock();

  NCNN_LOGE("FATAL ERROR! pool allocator get wild %p", ptr);
  ncnn::fastFree(ptr);
}

class UnlockedPoolAllocatorPrivate {
public:
  unsigned int size_compare_ratio; // 0~256
  size_t size_drop_threshold;
  std::list<std::pair<size_t, void *>> budgets;
  std::list<std::pair<size_t, void *>> payouts;
};

UnlockedPoolAllocator::UnlockedPoolAllocator()
    : Allocator(), d(new UnlockedPoolAllocatorPrivate) {
  d->size_compare_ratio = 0;
  d->size_drop_threshold = 10;
}

UnlockedPoolAllocator::~UnlockedPoolAllocator() {
  clear();

  if (!d->payouts.empty()) {
    NCNN_LOGE("FATAL ERROR! unlocked pool allocator destroyed too early");
#if NCNN_STDIO
    std::list<std::pair<size_t, void *>>::iterator it = d->payouts.begin();
    for (; it != d->payouts.end(); ++it) {
      void *ptr = it->second;
      NCNN_LOGE("%p still in use", ptr);
    }
#endif
  }

  delete d;
}

UnlockedPoolAllocator::UnlockedPoolAllocator(const UnlockedPoolAllocator &)
    : d(0) {}

UnlockedPoolAllocator &
UnlockedPoolAllocator::operator=(const UnlockedPoolAllocator &) {
  return *this;
}

void UnlockedPoolAllocator::clear() {
  std::list<std::pair<size_t, void *>>::iterator it = d->budgets.begin();
  for (; it != d->budgets.end(); ++it) {
    void *ptr = it->second;
    ncnn::fastFree(ptr);
  }
  d->budgets.clear();
}

void UnlockedPoolAllocator::set_size_compare_ratio(float scr) {
  if (scr < 0.f || scr > 1.f) {
    NCNN_LOGE("invalid size compare ratio %f", scr);
    return;
  }

  d->size_compare_ratio = (unsigned int)(scr * 256);
}

void UnlockedPoolAllocator::set_size_drop_threshold(size_t threshold) {
  d->size_drop_threshold = threshold;
}

void *UnlockedPoolAllocator::fastMalloc(size_t size) {
  // find free budget
  std::list<std::pair<size_t, void *>>::iterator it = d->budgets.begin(),
                                                 it_max = d->budgets.begin(),
                                                 it_min = d->budgets.begin();
  for (; it != d->budgets.end(); ++it) {
    size_t bs = it->first;

    // size_compare_ratio ~ 100%
    if (bs >= size && ((bs * d->size_compare_ratio) >> 8) <= size) {
      void *ptr = it->second;

      d->budgets.erase(it);

      d->payouts.push_back(std::make_pair(bs, ptr));

      return ptr;
    }

    if (bs > it_max->first) {
      it_max = it;
    }
    if (bs < it_min->first) {
      it_min = it;
    }
  }

  if (d->budgets.size() >= d->size_drop_threshold) {
    if (it_max->first < size) {
      ncnn::fastFree(it_min->second);
      d->budgets.erase(it_min);
    } else if (it_min->first > size) {
      ncnn::fastFree(it_max->second);
      d->budgets.erase(it_max);
    }
  }

  // new
  void *ptr = ncnn::fastMalloc(size);

  d->payouts.push_back(std::make_pair(size, ptr));

  return ptr;
}

void UnlockedPoolAllocator::fastFree(void *ptr) {
  // return to budgets
  std::list<std::pair<size_t, void *>>::iterator it = d->payouts.begin();
  for (; it != d->payouts.end(); ++it) {
    if (it->second == ptr) {
      size_t size = it->first;

      d->payouts.erase(it);

      d->budgets.push_back(std::make_pair(size, ptr));

      return;
    }
  }

  NCNN_LOGE("FATAL ERROR! unlocked pool allocator get wild %p", ptr);
  ncnn::fastFree(ptr);
}

} // namespace ncnn