//------------------------------------------------------------------------------
// 
// WebSocket SSL client, asynchronous
//
//------------------------------------------------------------------------------


#ifndef WEBSOCKETCLIENT_H
#define WEBSOCKETCLIENT_H

#include <cstdlib>
#include <string>
#include <memory>
#include <iostream>
#include <functional>
#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/asio/strand.hpp>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace websocket = beast::websocket; // from <boost/beast/websocket.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
namespace ssl = boost::asio::ssl;       // from <boost/asio/ssl.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

// Report a failure
inline void fail(beast::error_code ec, char const* what) {
    std::cerr << what << ": " << ec.message() << "\n";
}

class session : public std::enable_shared_from_this<session> {

using ReadCallback = std::function<void(const std::string&)>;
    
public:
    explicit session(net::io_context& ioc, ssl::context& ctx);
    ~session();

    void run(char const* host, char const* port, char const* text);
    void close();

    void on_resolve(beast::error_code ec, tcp::resolver::results_type results);
    void on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type ep);
    void on_ssl_handshake(beast::error_code ec);
    void on_handshake(beast::error_code ec);
    void on_read(beast::error_code ec,std::size_t bytes_transferred);
    void on_write(beast::error_code ec, std::size_t bytes_transferred);
    void on_close(beast::error_code ec);
    void setReadCallback(ReadCallback callback);
    
    volatile bool stopped = false;
private:
    std::string host_;
    std::string text_;
    tcp::resolver resolver_;
    beast::flat_buffer buffer_;
    websocket::stream<beast::ssl_stream<beast::tcp_stream>> ws_;

    ReadCallback onReadCallback;
};

#endif // WEBSOCKETCLIENT_H