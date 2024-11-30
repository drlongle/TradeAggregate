#include <algorithm>
#include <atomic>
#include <chrono>
#include <iostream>
#include <memory>
#include <stop_token>
#include <thread>
#include <vector>

#include <json/json.h>

#include "Scraper.h"

int main() {
    std::vector<ScraperConfig> scraper_configs{
        {.host = "api.exchange.coinbase.com",
         .port = "443",
         .path = "/products/BTC-USD/trades",
         .version = 11},
        {.host = "api.exchange.coinbase.com",
         .port = "443",
         .path = "/products/ETH-USD/trades",
         .version = 11},
        {.host = "api.exchange.coinbase.com",
         .port = "443",
         .path = "/products/SOL-USD/trades",
         .version = 11},
    };

    std::vector<std::jthread> jthreads;
    std::vector<std::unique_ptr<Scraper>> scrapers;
    for (const auto &config : scraper_configs) {
        scrapers.emplace_back(std::make_unique<Scraper>(config));
    }

    for (auto &scraper : scrapers) {
        jthreads.emplace_back(&Scraper::run, scraper.get());
    }

    std::this_thread::sleep_for(std::chrono::seconds(10));

    for (auto &jt : jthreads) {
        jt.request_stop();
    }

    return 0;
}
