#include <iostream>
#include <map>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

#include <jsoncpp/json/json.h>

#include "Aggregator.h"
#include "date.h"

std::ostream &operator<<(std::ostream &os, const TradeAggregate::Trade &trade) {
    os << "{ symbol: " << trade.symbol << " trade_id: " << trade.id
       << " side: " << trade.side << " size: " << trade.size
       << " price: " << trade.price << " time: " << trade.tp << " }";
    return os;
}

namespace TradeAggregate {

class Aggregator::Impl {
  public:
    Impl(ScraperList &scrapers_) : scrapers{scrapers_}, queue{queue_size} {}

    bool poll() {
        std::string str;
        Json::Value json;
        std::chrono::system_clock::time_point tp;
        std::vector<Trade> trades;
        bool result{false};

        for (size_t i{0}, sz{scrapers.size()}; i < sz; ++i) {
            std::cout << " ---------------------------------" << std::endl;
            auto &q{scrapers[i]->getQueue()};
            const auto &symbol{scrapers[i]->getSymbol()};
            std::cout << "symbol: " << symbol << std::endl;
            if (q.pop(str)) {
                result = true;
                if (reader.parse(str, json)) {
                    for (int index{0}, sz{static_cast<int>(json.size())};
                         index < sz; ++index) {
                        const auto &jv{json[index]};
                        std::istringstream ss(jv["time"].asString());
                        ss >> date::parse("%Y-%m-%dT%H:%M:%S", tp);
                        trades.emplace_back(symbol, jv["trade_id"].asInt(),
                                            jv["side"].asString(),
                                            jv["size"].asString(),
                                            jv["price"].asString(), tp);
                        std::cout << trades.back() << std::endl;
                    }
                }
            }
        }

        return result;
    }

    inline QueueType &getQueue() { return queue; }

  private:
    ScraperList &scrapers;
    Json::Reader reader;
    QueueType queue;
    std::map<uint64_t, std::unordered_map<uint64_t, Trade>> trades;
};

Aggregator::Aggregator(ScraperList &scrapers)
    : impl{std::make_unique<Impl>(scrapers)} {}

Aggregator::~Aggregator() = default;

void Aggregator::run(std::stop_token stopToken) {
    while (!stopToken.stop_requested()) {
        if (!impl->poll()) {
            std::this_thread::yield();
        }
    }
}

Aggregator::QueueType &Aggregator::getQueue() { return impl->getQueue(); }

} // namespace TradeAggregate
