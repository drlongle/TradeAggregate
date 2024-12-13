#include <cstdint>
#include <sstream>
#include <string>

std::string compute_trade_metric(const std::string &symbol, uint64_t trade_id, const std::string &time,
                                 const std::string &side, const std::string &size, const std::string &price) {
    std::ostringstream oss;
    oss << "{ symbol: " << symbol << " trade_id: " << trade_id << " side: " << side << " size: " << size
        << " price: " << price << " time: " << time << " }";
    return oss.str();
}
