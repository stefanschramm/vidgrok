#include "src/DataDispatcher.h"
#include <chrono>

bool DataDispatcher::put(Samples data) {
  std::lock_guard lock(mutex);
  if (closed) {
    return false; // ignore passed data
  }
  mData.emplace(data);
  conditionVariable.notify_one();

  return !closed;
}

std::optional<Samples> DataDispatcher::get(std::chrono::milliseconds mReadTimeout) {
  std::unique_lock lk(mutex);
  conditionVariable.wait_for(lk, mReadTimeout, [this] { return mData.has_value(); });
  return mData;
}

void DataDispatcher::clear() {
  std::unique_lock lk(mutex);
  mData.reset();

  conditionVariable.notify_one();
}

void DataDispatcher::close() {
  closed = true;
  clear(); // notifies producer
}
