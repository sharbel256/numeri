#include "websocketclient.h"


// Resolver and socket require an io_context
WebSocketClient::WebSocketClient(net::io_context& ioc, ssl::context& ctx)
    : resolver_(net::make_strand(ioc))
    , ws_(net::make_strand(ioc), ctx)
{}

WebSocketClient::~WebSocketClient() {
    // @TODO: must implement proper WebSocketClient closure
    std::cout << "TODO: WebSocketClient::~WebSocketClient()" << std::endl;
}

// Start the asynchronous operation
void WebSocketClient::run(std::string host, std::string port, std::string text) {
    // Save these for later
    host_ = host;
    text_ = text;

    if (!onReadCallback) {
        std::cout << "WebSocketClient::run() onReadCallback is not set, returning. " << std::endl;
        return;
    }

    // Look up the domain name
    resolver_.async_resolve(host,port,
        beast::bind_front_handler(&WebSocketClient::on_resolve,shared_from_this()));
}

void WebSocketClient::close() {
    std::cout << "WebSocketClient::close()" << std::endl;
    
    ws_.async_close(websocket::close_code::normal,
        beast::bind_front_handler(&WebSocketClient::on_close,shared_from_this()));
}

void WebSocketClient::on_resolve(beast::error_code ec, tcp::resolver::results_type results) {
    if(ec) return fail(ec, "resolve");

    // Set a timeout on the operation
    beast::get_lowest_layer(ws_).expires_after(std::chrono::seconds(30));

    // Make the connection on the IP address we get from a lookup
    beast::get_lowest_layer(ws_).async_connect( results,
        beast::bind_front_handler(&WebSocketClient::on_connect,shared_from_this()));
}

void WebSocketClient::on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type ep) {
    if(ec) return fail(ec, "connect");

    // Set a timeout on the operation
    beast::get_lowest_layer(ws_).expires_after(std::chrono::seconds(30));

    // Set SNI Hostname (many hosts need this to handshake successfully)
    if(! SSL_set_tlsext_host_name(ws_.next_layer().native_handle(),
            host_.c_str()))
    {
        ec = beast::error_code(static_cast<int>(::ERR_get_error()),
            net::error::get_ssl_category());
        return fail(ec, "connect");
    }

    // Update the host_ string. This will provide the value of the
    // Host HTTP header during the WebSocket handshake.
    // See https://tools.ietf.org/html/rfc7230#section-5.4
    host_ += ':' + std::to_string(ep.port());
    
    // Perform the SSL handshake
    ws_.next_layer().async_handshake(ssl::stream_base::client,
        beast::bind_front_handler(&WebSocketClient::on_ssl_handshake,shared_from_this()));
}

void WebSocketClient::on_ssl_handshake(beast::error_code ec) {
    if(ec) return fail(ec, "ssl_handshake");

    // Turn off the timeout on the tcp_stream, because
    // the websocket stream has its own timeout system.
    beast::get_lowest_layer(ws_).expires_never();

    // Set suggested timeout settings for the websocket
    ws_.set_option(
        websocket::stream_base::timeout::suggested(beast::role_type::client));

    // Set a decorator to change the User-Agent of the handshake
    ws_.set_option(websocket::stream_base::decorator(
        [](websocket::request_type& req)
        {
            req.set(http::field::user_agent,
                std::string(BOOST_BEAST_VERSION_STRING) +
                    " websocket-client-async-ssl");
        }));

    // Perform the websocket handshake
    ws_.async_handshake(host_, "/",
        beast::bind_front_handler(&WebSocketClient::on_handshake,shared_from_this()));
}

void WebSocketClient::on_handshake(beast::error_code ec) {
    if(ec) return fail(ec, "handshake");

    // Send the message
    ws_.async_write( net::buffer(text_), 
            beast::bind_front_handler(&WebSocketClient::on_write,shared_from_this()));
}

void WebSocketClient::on_write(beast::error_code ec, std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);

    if(ec) return fail(ec, "write");

    // Read a message into our buffer
    ws_.async_read(buffer_,
        beast::bind_front_handler(&WebSocketClient::on_read,shared_from_this()));
}

void WebSocketClient::on_read(beast::error_code ec,std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);

    if(ec) return fail(ec, "read");

    std::cout << beast::make_printable(buffer_.data()) << std::endl;

    if (onReadCallback) {
        onReadCallback(beast::buffers_to_string(buffer_.data()));
    }
    buffer_.consume(buffer_.size());

    // Read a single message into our buffer
    ws_.async_read(buffer_,
        beast::bind_front_handler(&WebSocketClient::on_read,shared_from_this()));
}

void WebSocketClient::on_close(beast::error_code ec) {
    if(ec) return fail(ec, "close");

    stopped = true;

    // If we get here then the connection is closed gracefully
    std::cout << "WebSocketClient::on_close()" << std::endl;
}

void WebSocketClient::setReadCallback(ReadCallback callback) {
    this->onReadCallback = callback;
}
