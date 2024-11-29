#pragma once

#include <atomic>
#include <memory>
#include <string>

#include <boost/lockfree/spsc_queue.hpp>

class Scraper {
  public:
    Scraper(const std::string host, std::string port, const std::string path,
            int version);

    ~Scraper();

    void run();

    void stop();

  private:
    class Impl;

    std::atomic<bool> stop_flag{false};
    std::unique_ptr<Impl> impl;
};
