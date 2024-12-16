#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <memory>
#include <ranges>
#include <stop_token>
#include <string>
#include <thread>
#include <vector>

#include <boost/program_options.hpp>
#include <json/json.h>

#include "Aggregator.h"
#include "FileWriter.h"
#include "Scraper.h"

struct Config {
    std::string input_file;
    std::string output_file;
};

Config parseConfig(int argc, const char *argv[]) {
    namespace po = boost::program_options;
    Config config;

    po::options_description desc("Allowed options");
    desc.add_options()("help,h", "Show help message")("input,i", po::value<std::string>(&config.input_file)->required(),
                                                      "Input file")(
        "output,o", po::value<std::string>(&config.output_file)->required(), "Output file");

    // Parse command-line options
    po::variables_map vm;
    try {
        po::store(po::parse_command_line(argc, argv, desc), vm);
        po::notify(vm);
    } catch (const po::error &ex) {
        std::cerr << "Error: " << ex.what() << '\n';
        exit(1);
    }

    // Handle options
    if (vm.count("help")) {
        std::cout << desc << "\n";
        exit(0);
    }

    return config;
}

std::vector<TradeAggregate::ScraperConfig> parseInputFile(const std::string &input_file) {
    using namespace TradeAggregate;

    std::vector<ScraperConfig> configs;
    std::ifstream ifs{input_file};

    Json::Value json;
    ifs >> json;

    const auto &endpoints{json["endpoints"]};

    for (int index{0}, sz{static_cast<int>(endpoints.size())}; index < sz; ++index) {
        const auto &jv{endpoints[index]};
        configs.emplace_back(jv["symbol"].asString(), jv["host"].asString(), jv["port"].asString(),
                             jv["path"].asString(), jv["http_version"].asInt());
    }

    return configs;
}

std::atomic<bool> stop_flag{false};

void signalHandler(int signal) {
    stop_flag.store(true, std::memory_order_relaxed);
    stop_flag.notify_all();
}

int main(int argc, const char *argv[]) {

    using namespace TradeAggregate;

    Config config{parseConfig(argc, argv)};

    const auto scraper_configs{parseInputFile(config.input_file)};

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

    FileWriter writer{agg, config.output_file};
    jthreads.emplace_back(&FileWriter::run, &writer);

    std::signal(SIGTERM, signalHandler);
    std::signal(SIGINT, signalHandler);
    stop_flag.wait(false, std::memory_order_relaxed);

    std::cout << "Stopping all threads..." << std::endl;

    // Stop the threads in reverse orders because of their dependency.
    std::ranges::reverse(jthreads);
    for (auto &jt : jthreads) {
        jt.request_stop();
        jt.join();
    }

    return 0;
}
