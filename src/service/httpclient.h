//------------------------------------------------------------------------------
//
// HTTP client, asynchronous
//
//------------------------------------------------------------------------------

#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <iomanip>
#include "logging.h"

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
namespace ssl = net::ssl;               // from <boost/asio/ssl.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

//------------------------------------------------------------------------------

// // Report a failure
// #ifndef FAIL_FUNC
// #define FAIL_FUNC
// void fail(beast::error_code ec, char const* what)
// {
//     std::cerr << what << ": " << ec.message() << "\n";
// }
// #endif

// Performs an HTTP GET and prints the response
class HTTPClient : public std::enable_shared_from_this<HTTPClient> {

using ReadCallback = std::function<void(const std::string&)>;

public:
    // Objects are constructed with a strand to
    // ensure that handlers do not execute concurrently.
    explicit HTTPClient(net::any_io_executor ex,ssl::context& ctx);
    ~HTTPClient();
    void shutdown();
    void run(const char* host, const char* port);
    void on_resolve(beast::error_code ec, tcp::resolver::results_type results);
    void on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type);
    void on_handshake(beast::error_code ec);
    void on_write(beast::error_code ec, std::size_t bytes_transferred);
    void on_read(beast::error_code ec, std::size_t bytes_transferred);
    void on_shutdown(beast::error_code ec);
    void setReadCallback(ReadCallback callback);
    std::string calculateSignature(const std::string& message, const std::string& secretKey);

private:
    std::string host_;
    tcp::resolver resolver_;
    boost::asio::ssl::stream<beast::tcp_stream> stream_;
    beast::flat_buffer buffer_; // (Must persist between reads)
    http::request<http::empty_body> req_;
    http::response<http::string_body> res_;

    ReadCallback onReadCallback;
};


#endif // HTTPCLIENT_H