#pragma once

#include <memory>
#include <string>
#include <stop_token>

#include <boost/lockfree/spsc_queue.hpp>

struct ScraperConfig {
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

    using QueueType = boost::lockfree::spsc_queue<Json::Value>;
    inline QueueType &getQueue() { return queue; }

    static constexpr size_t queue_size = 8192;

  private:
    class Impl;
    ScraperConfig config;
    std::unique_ptr<Impl> impl;
    boost::lockfree::spsc_queue<Json::Value> queue;
};
