
#include <chrono>
#include <iostream>
#include <optional>
#include <string>
#include <thread>

#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>

#include "Scraper.h"

class Scraper::Impl {
  public:
    Impl(const std::string &host_, const std::string &port_,
         const std::string &path_, int version_)
        : host{host_}, port{port_}, path{path_}, version{version_},
          ctx{boost::asio::ssl::context::tlsv12_client}, resolver{ioc},
          resolve_results{resolver.resolve(host_, port_)} {
        ctx.set_default_verify_paths();
        ctx.set_verify_mode(boost::asio::ssl::verify_peer);
    }

    std::optional<std::string> fetch();

  private:
    const std::string host;
    const std::string port;
    const std::string path;
    const int version;
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

Scraper::Scraper(const std::string host, const std::string port,
                 const std::string path, int version)
    : impl{std::make_unique<Impl>(host, port, path, version)} {}

Scraper::~Scraper() = default;

void Scraper::run() {
    while (!stop_flag.load(std::memory_order_relaxed)) {
        auto res = impl->fetch();
        if (res.has_value())
            std::cout << res.value() << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void Scraper::stop() { stop_flag.store(true, std::memory_order_relaxed); }
