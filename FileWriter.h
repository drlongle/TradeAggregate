#pragma once

#include <memory>
#include <stop_token>
#include <string>

#include "Aggregator.h"

namespace TradeAggregate {

class FileWriter {
  public:
    FileWriter(Aggregator &aggregator, const std::string &filepath);
    FileWriter(const FileWriter &) = delete;
    FileWriter &operator=(const FileWriter &) = delete;

    ~FileWriter();

    void run(std::stop_token stopToken);

  private:
    class Impl;
    std::unique_ptr<Impl> impl;
};

} // namespace TradeAggregate
