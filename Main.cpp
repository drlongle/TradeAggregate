
#include <algorithm>
#include <atomic>
#include <bit>
#include <bitset>
#include <cassert>
#include <climits>
#include <cmath>
#include <condition_variable>
#include <functional>
#include <future>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <list>
#include <map>
#include <memory>
#include <numeric>
#include <queue>
#include <random>
#include <ranges>
#include <regex>
#include <set>
#include <sstream>
#include <stack>
#include <string_view>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <json/json.h>

#include "Scraper.h"

int main() {
    // poll("api.exchange.coinbase.com", "443", "/products/BTC-USD/trades", 11);
    // poll("api.exchange.coinbase.com", "443", "/products/ETH-USD/trades", 11);
    // poll("api.exchange.coinbase.com", "443", "/products/SOL-USD/trades", 11);

    Scraper scraper{"api.exchange.coinbase.com", "443",
                    "/products/BTC-USD/trades", 11};
    scraper.run();

    return 0;
}
