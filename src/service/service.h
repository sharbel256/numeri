#ifndef SERVICE_H
#define SERVICE_H

#include <boost/asio/ssl.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <string>
#include <iostream>

using tcp = boost::asio::ip::tcp;
namespace ssl = boost::asio::ssl;
namespace websocket = boost::beast::websocket;
namespace http = boost::beast::http;

class Service {
public:

    

    Service() {

    };

    void sendMessage(std::string message) {
        try {
            std::cout <<"beginning connection" << std::endl;
            std::string host = "ws-feed.prime.coinbase.com";
            std::string port = "443";
            std::string path = "/";

            boost::asio::io_context io_context;

            // ssl::context ssl_context{ssl::context::sslv23_client};
            ssl::context ssl_context{ssl::context::tlsv12_client};

            tcp::resolver resolver(io_context);
            ssl::stream<tcp::socket> ssl_stream(io_context, ssl_context);

            auto const results = resolver.resolve(host, port);
            boost::asio::connect(ssl_stream.lowest_layer(), results);
            ssl_stream.handshake(ssl::stream_base::client);
            std::cout <<"connection established" << std::endl;

            websocket::stream<ssl::stream<tcp::socket>> websocket_stream(std::move(ssl_stream));
            websocket_stream.set_option(websocket::stream_base::decorator([](websocket::request_type& req) {
                req.set(http::field::host, "ws-feed.prime.coinbase.com");
                req.set(http::field::upgrade, "websocket");
                req.set(http::field::connection, "Upgrade");
                req.set(http::field::sec_websocket_key, "x3JJHMbDL1EzLkh9GBhXDw==");
                req.set(http::field::sec_websocket_version, "13");
            }));
            websocket_stream.handshake(host + ":" + port, path);
            std::cout <<"websocket handshake complete" << std::endl;
            websocket_stream.write(boost::asio::buffer(message));
            boost::beast::multi_buffer buffer;
            websocket_stream.read(buffer);
            std::cout << boost::beast::make_printable(buffer.data()) << std::endl;
        } catch (std::exception& e) {
            std::cerr << "Exception: " << e.what() << std::endl;
        }
        
    }
};

#endif