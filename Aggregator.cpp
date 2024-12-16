#include <iostream>
#include <map>
#include <sstream>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include <jsoncpp/json/json.h>

#include "Aggregator.h"
#include "date.h"

std::ostream &operator<<(std::ostream &os, const TradeAggregate::Trade &trade) {
    os << "{ symbol: " << trade.symbol << " trade_id: " << trade.trade_id << " side: " << trade.side
       << " size: " << trade.size << " price: " << trade.price << " time: " << trade.time << " }";
    return os;
}

namespace TradeAggregate {

class Aggregator::Impl {
  public:
    Impl(ScraperList &scrapers_) : scrapers{scrapers_}, queue{queue_size} {}

    static constexpr size_t max_size{1'000'000};
    template <typename CONTAINER> void purge(CONTAINER &container, size_t max_size) {
        while (container.size() > max_size) {
            container.erase(std::begin(container));
        }
    }

    bool poll() {
        std::string str;
        Json::Value json;
        std::chrono::system_clock::time_point timepoint;
        bool result{false};
        std::vector<const Trade *> new_trades;

        for (size_t i{0}, sz{scrapers.size()}; i < sz; ++i) {
            auto &q{scrapers[i]->getQueue()};
            const auto &symbol{scrapers[i]->getSymbol()};
            if (q.pop(str)) {
                result = true;
                if (reader.parse(str, json)) {
                    for (int index{0}, sz{static_cast<int>(json.size())}; index < sz; ++index) {
                        const auto &jv{json[index]};
                        const auto &time_str{jv["time"].asString()};
                        std::istringstream ss(time_str);
                        ss >> date::parse("%Y-%m-%dT%H:%M:%S", timepoint);

                        const uint64_t trade_id{static_cast<uint64_t>(jv["trade_id"].asInt())};
                        auto [it, inserted] = trades[timepoint].emplace(
                            std::piecewise_construct, std::forward_as_tuple(trade_id),
                            std::forward_as_tuple(symbol, trade_id, jv["side"].asString(), jv["size"].asString(),
                                                  jv["price"].asString(), time_str, timepoint));
                        if (inserted) {
                            new_trades.push_back(&(it->second));
                        }
                    }
                }
            }
        }

        for (const auto *ptrade : new_trades) {
            const auto lower_timepoint{ptrade->timepoint - std::chrono::milliseconds(5)};
            const auto upper_timepoint{ptrade->timepoint + std::chrono::milliseconds(5)};
            bool found_match{false};
            for (auto it{trades.lower_bound(lower_timepoint)};
                 !found_match && it != trades.end() && it->first <= upper_timepoint; ++it) {
                for (const auto &[tp, trade] : it->second) {
                    if (trade.trade_id != ptrade->trade_id) {
                        found_match = true;
                        break;
                    }
                }
            }
            if (found_match) {
                queue.push(*ptrade);
            }
        }

        purge(trades, max_size);

        return result;
    }

    inline QueueType &getQueue() { return queue; }

  private:
    ScraperList &scrapers;
    Json::Reader reader;
    QueueType queue;
    std::map<std::chrono::system_clock::time_point, std::unordered_map<uint64_t, Trade>> trades;
};

Aggregator::Aggregator(ScraperList &scrapers) : impl{std::make_unique<Impl>(scrapers)} {}

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
