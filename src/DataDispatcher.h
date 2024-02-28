#pragma once

#include <condition_variable>
#include <mutex>
#include <optional>

using Samples = std::pair<uint8_t*, size_t>;

// Construct for synchronizing one producer and one consumer.
// The producer shall wait until the consumer finished processing the data.
// The consumer shall not be blocked when no data is available.
class DataDispatcher {
public:
  // To be called by producer:
  // Put data into exchange variable and block until it was cleared by the consumer.
  // Returns false when producer is supposed to stop producing.
  bool put(Samples data);

  // To be called by consumer:
  // Get data (non-blocking). Returns empty std::optional when no data is available.
  std::optional<Samples> get();

  // To be called by consumer:
  // Clear data to allow producer to continue passing data.
  void clear();

  // To be called by consumer:
  // Tell producer to stop producing.
  void close();

private:
  std::mutex mutex;
  std::condition_variable conditionVariable;
  std::optional<Samples> mData;
  bool closed = false;
};
