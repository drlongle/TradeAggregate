#include <chrono>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <thread>

#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>

#include "Scraper.h"

namespace TradeAggregate {

class Scraper::Impl {
  public:
    Impl(const ScraperConfig &config_)
        : config{config_}, ctx{boost::asio::ssl::context::tlsv12_client},
          resolver{ioc}, resolve_results{
                             resolver.resolve(config_.host, config_.port)} {
        ctx.set_default_verify_paths();
        ctx.set_verify_mode(boost::asio::ssl::verify_peer);
    }

    std::optional<std::string> fetch();

  private:
    ScraperConfig config;
    boost::asio::io_context ioc;
    boost::asio::ssl::context ctx;
    boost::asio::ip::tcp::resolver resolver;
    const boost::asio::ip::tcp::resolver::results_type resolve_results;
};

std::optional<std::string> Scraper::Impl::fetch() {
    namespace beast = boost::beast;
    namespace http = beast::http;
    namespace net = boost::asio;
    namespace ssl = net::ssl;

    const auto &host{config.host};
    const auto &path{config.path};
    const auto version{config.version};
    std::optional<std::string> result;

    try {
        ssl::stream<beast::tcp_stream> stream(ioc, ctx);

        if (!SSL_set_tlsext_host_name(stream.native_handle(), host.c_str())) {
            beast::error_code ec{static_cast<int>(::ERR_get_error()),
                                 net::error::get_ssl_category()};
            throw beast::system_error{ec};
        }

        beast::get_lowest_layer(stream).connect(resolve_results);

        stream.handshake(ssl::stream_base::client);

        http::request<http::string_body> req{http::verb::get, path, version};
        req.set(http::field::host, host);
        req.set(http::field::user_agent, "New Agent");

        http::write(stream, req);

        beast::flat_buffer buffer;
        http::response<http::dynamic_body> res;

        http::read(stream, buffer, res);

        result = boost::beast::buffers_to_string(res.body().data());

        beast::error_code ec;
        stream.shutdown(ec);

        if (ec != net::ssl::error::stream_truncated)
            throw beast::system_error{ec};
    } catch (std::exception const &e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return result;
}

Scraper::Scraper(const ScraperConfig &conf)
    : config{conf}, impl{std::make_unique<Impl>(conf)}, queue{queue_size} {}

Scraper::~Scraper() = default;

void Scraper::run(std::stop_token stopToken) {

    while (!stopToken.stop_requested()) {
        auto res = impl->fetch();
        if (res.has_value()) {
            queue.push(res.value());
        }
    }
}

} // namespace TradeAggregate
