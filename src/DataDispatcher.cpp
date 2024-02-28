#include "src/DataDispatcher.h"
#include <stdexcept>

bool DataDispatcher::put(Samples data) {
  if (closed) {
    throw std::runtime_error("DataDispatcher::put() called, but dispatcher has already been closed.");
  }
  {
    std::lock_guard lock(mutex);
    mData.emplace(data);
  }
  {
    std::unique_lock lk(mutex);
    conditionVariable.wait(lk, [this] { return !mData.has_value(); });
  }

  return !closed;
}

std::optional<Samples> DataDispatcher::get() {
  std::unique_lock lk(mutex);
  return mData;
}

void DataDispatcher::clear() {
  std::unique_lock lk(mutex);
  mData.reset();
  lk.unlock();

  conditionVariable.notify_one();
}

void DataDispatcher::close() {
  closed = true;
  clear(); // notifies producer
}
