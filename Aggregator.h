#pragma once

#include <chrono>
#include <cstdint>
#include <iosfwd>
#include <memory>
#include <stop_token>
#include <string>

#include <boost/lockfree/spsc_queue.hpp>

#include "Scraper.h"

namespace TradeAggregate {

struct Trade {
    std::string symbol;
    std::uint64_t trade_id;
    std::string side;
    std::string size;
    std::string price;
    std::string time;
    std::chrono::system_clock::time_point timepoint;
};

class Aggregator {
  public:
    Aggregator(ScraperList &scrapers);
    Aggregator(const Aggregator &) = delete;
    Aggregator &operator=(const Aggregator &) = delete;

    ~Aggregator();

    void run(std::stop_token stopToken);

    static constexpr size_t queue_size = 65536;
    using QueueType = boost::lockfree::spsc_queue<Trade>;
    QueueType &getQueue();

  private:
    class Impl;
    std::unique_ptr<Impl> impl;
};

} // namespace TradeAggregate

std::ostream &operator<<(std::ostream &os, const TradeAggregate::Trade &trade);
