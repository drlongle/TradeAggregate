#include <algorithm>
#include <atomic>
#include <chrono>
#include <iostream>
#include <memory>
#include <ranges>
#include <stop_token>
#include <thread>
#include <vector>

#include <json/json.h>

#include "Aggregator.h"
#include "Scraper.h"

int main() {
    using namespace TradeAggregate;

    std::vector<ScraperConfig> scraper_configs{
        {.symbol = "BTC-USD",
         .host = "api.exchange.coinbase.com",
         .port = "443",
         .path = "/products/BTC-USD/trades",
         .version = 11},
        {.symbol = "ETH-USD",
         .host = "api.exchange.coinbase.com",
         .port = "443",
         .path = "/products/ETH-USD/trades",
         .version = 11},
        {.symbol = "SOL-USD",
         .host = "api.exchange.coinbase.com",
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

    Aggregator agg{scrapers};
    jthreads.emplace_back(&Aggregator::run, &agg);

    std::this_thread::sleep_for(std::chrono::seconds(100));

    std::cout << "Stopping all threads..." << std::endl;

    // Reverse the threads to stop the aggregator before the scrapers.
    std::ranges::reverse(jthreads);
    for (auto &jt : jthreads) {
        jt.request_stop();
        jt.join();
    }

    return 0;
}
