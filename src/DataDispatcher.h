#pragma once

#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <optional>
#include <span>

// Single element buffer for synchronizing one producer and one consumer.
// When the consumer is too slow for the producer, the producer is blocked.
// When the consumer is faster, it will be blocked until data gets available, but just for a certain time.
template <typename T>
class DataDispatcher final {
public:
  // To be called by producer:
  // Put and wait until data was cleared by consumer.
  auto put(T data) -> void {
    std::unique_lock lk(dataMutex);

    if (closed) {
      return; // ignore passed data
    }

    mData.emplace(data);
    conditionVariable.notify_all();

    conditionVariable.wait(lk, [this] { return !mData.has_value(); });
    // We have to wait after emplacing and notifying because the driver frees the memory when returning.
  }

  // To be called by consumer:
  // Get data. Returns empty std::optional when no data is available and readTimeout was reached.
  std::optional<T> get(std::chrono::milliseconds readTimeout) {
    std::unique_lock lk(dataMutex);
    conditionVariable.wait_for(lk, readTimeout, [this] { return mData.has_value(); });
    return mData;
  }

  // To be called by consumer:
  // Clear data to allow producer to continue passing data.
  auto clear() -> void {
    std::unique_lock lk(dataMutex);
    mData.reset();
    conditionVariable.notify_all();
  }

  // Close channel (producer has no more data or consumer doesn't want any more).
  auto close() -> void {
    std::unique_lock lk(closedMutex);
    closed = true;
    clear(); // notifies producer
  }

  // Determine wether the channel has been closed.
  auto isClosed() -> bool {
    std::unique_lock lk(closedMutex);
    return closed;
  }

private:
  std::mutex dataMutex;
  std::mutex closedMutex;
  std::condition_variable conditionVariable;
  std::optional<T> mData;
  bool closed = false;
};

using Sample = uint8_t;
using Samples = std::span<Sample>;
using SampleDataDispatcher = DataDispatcher<Samples>;
