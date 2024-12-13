
#include <cstdint>
#include <fstream>
#include <string>

#include "FileWriter.h"
#include "Printer.h"

namespace TradeAggregate {

class FileWriter::Impl {
  public:
    Impl(Aggregator &aggregator_, const std::string &filepath)
        : aggregator{aggregator_}, ofs{filepath, std::ios::out | std::ios::trunc} {}

    bool poll() {
        auto &q{aggregator.getQueue()};
        Trade trade;
        bool result{false};

        while (q.pop(trade)) {
            result = true;
            const std::string str{
                compute_trade_metric(trade.symbol, trade.trade_id, trade.time, trade.side, trade.size, trade.price)};
            ofs << str << std::endl;
        }

        return result;
    }

  private:
    Aggregator &aggregator;
    std::ofstream ofs;
};

FileWriter::FileWriter(Aggregator &aggregator, const std::string &filepath)
    : impl{std::make_unique<Impl>(aggregator, filepath)} {}

FileWriter::~FileWriter() = default;

void FileWriter::run(std::stop_token stopToken) {
    while (!stopToken.stop_requested()) {
        if (!impl->poll()) {
            std::this_thread::yield();
        }
    }
}

} // namespace TradeAggregate
