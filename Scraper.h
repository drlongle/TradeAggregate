#pragma once

#include <memory>
#include <stop_token>
#include <string>
#include <vector>

#include <boost/lockfree/spsc_queue.hpp>

namespace TradeAggregate {

struct ScraperConfig {
    std::string symbol;
    std::string host;
    std::string port;
    std::string path;
    int version;
};

class Scraper {
  public:
    Scraper(const ScraperConfig &config);
    Scraper(const Scraper &) = delete;
    Scraper &operator=(const Scraper &) = delete;

    ~Scraper();

    void run(std::stop_token stopToken);

    static constexpr size_t queue_size = 65536;
    using QueueType = boost::lockfree::spsc_queue<std::string>;
    inline QueueType &getQueue() { return queue; }

    inline std::string &getSymbol() { return config.symbol; }

  private:
    class Impl;
    ScraperConfig config;
    std::unique_ptr<Impl> impl;
    QueueType queue;
};

using ScraperList = std::vector<std::unique_ptr<Scraper>>;

} // namespace TradeAggregate
