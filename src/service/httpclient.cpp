#include "httpclient.h"
#include <iostream> // Make sure this is included for std::cout

HTTPClient::HTTPClient(net::io_context& ioc, ssl::context& ctx)
    : ioc_(ioc), ctx_(ctx)
{
    
}

HTTPClient::~HTTPClient() {
    std::cout << "TODO: HTTPClient::~HTTPClient()" << std::endl;
}

void HTTPClient::connectTLS(ssl::stream<beast::tcp_stream>& stream, const std::string& host, const std::string& port) {
    beast::error_code ec;
    
    // Set a timeout on the operation
    beast::get_lowest_layer(stream).expires_after(std::chrono::seconds(30));
    
    try {
        tcp::resolver resolver(ioc_);
        auto const results = resolver.resolve(host, port, ec);
        if(ec) {
            std::cerr << "HTTPClient: Resolve failed: " << ec.message() << std::endl;
            throw std::runtime_error("Resolve failed: " + ec.message());
        }

        beast::get_lowest_layer(stream).connect(results, ec);
        if(ec) {
            std::cerr << "HTTPClient: Connect failed: " << ec.message() << std::endl;
            throw std::runtime_error("Connect failed: " + ec.message());
        }

        // Set SNI Hostname (many hosts need this to handshake successfully)
        if(!SSL_set_tlsext_host_name(stream.native_handle(), host.c_str())) {
            beast::error_code ssl_ec(static_cast<int>(::ERR_get_error()), net::error::get_ssl_category());
            std::cerr << "HTTPClient: SNI set failed: " << ssl_ec.message() << std::endl;
            throw std::runtime_error("SNI set failed: " + ssl_ec.message());
        }

        beast::get_lowest_layer(stream).expires_after(std::chrono::seconds(30));
        stream.handshake(ssl::stream_base::client, ec);
        if(ec) {
            std::cerr << "HTTPClient: Handshake failed: " << ec.message() << std::endl;
            throw std::runtime_error("Handshake failed: " + ec.message());
        }
    }
    catch (const std::exception& e) {
        std::cerr << "HTTPClient: Exception in connectTLS: " << e.what() << std::endl;
        throw;
    }
}

std::string HTTPClient::get(const std::string& host, const std::string& port, const std::string& target, 
                            const std::map<std::string,std::string>& extraHeaders)
{
    ssl::stream<beast::tcp_stream> stream(ioc_, ctx_);
    connectTLS(stream, host, port);

    http::request<http::string_body> req{http::verb::get, target, 11};
    req.set(http::field::host, host);
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    req.set(http::field::content_type, "application/json");
    for (auto& h : extraHeaders) {
        req.set(h.first, h.second);
    }

    return sendRequestAndGetResponse(stream, req);
}
std::string HTTPClient::post(const std::string& host, const std::string& port, 
    const std::string& target, const std::string& body,
    const std::map<std::string,std::string>& extraHeaders)
{
    try {
        ssl::stream<beast::tcp_stream> stream(ioc_, ctx_);
        if (!stream.native_handle()) {
            throw std::runtime_error("Failed to create SSL stream");
        }

        connectTLS(stream, host, port);

        http::request<http::string_body> req{http::verb::post, target, 11};
        req.set(http::field::host, host);
        req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
        req.set(http::field::content_type, "application/json");

        for (const auto& h : extraHeaders) {
            if (h.first.empty() || h.second.empty()) {
                std::cerr << "Warning: Empty header key or value detected" << std::endl;
                continue;
            }
            req.set(h.first, h.second);
        }

        req.body() = body;
        req.prepare_payload();
        return sendRequestAndGetResponse(stream, req);
    }
    catch (const std::exception& e) {
        std::cerr << "HTTPClient: Exception in POST request: " << e.what() << std::endl;
        throw;
    }
}

std::string HTTPClient::sendRequestAndGetResponse(ssl::stream<beast::tcp_stream>& stream, http::request<http::string_body>& req)
{
    beast::error_code ec;
    http::write(stream, req, ec);
    if(ec) {
        std::cerr << "HTTPClient: Write failed: " << ec.message() << std::endl;
        throw std::runtime_error("Write failed: " + ec.message());
    }

    beast::flat_buffer buffer;
    http::response<http::string_body> res;
    beast::get_lowest_layer(stream).expires_after(std::chrono::seconds(30));
    http::read(stream, buffer, res, ec);
    if(ec) {
        std::cerr << "HTTPClient: Read failed: " << ec.message() << std::endl;
        throw std::runtime_error("Read failed: " + ec.message());
    }
    
    // Store the response body before shutdown
    std::string response_body = res.body();

    // Gracefully close the connection - handle broken pipe cleanly
    try {
        // First try SSL shutdown
        stream.shutdown(ec);
        if (ec == net::error::eof || ec == net::ssl::error::stream_truncated) {
            // These errors are normal during SSL shutdown
            ec = {}; // Clear the error
        } 
        else if (ec) {
            // Continue despite SSL shutdown errors - we'll still try to close the socket
        }
        
        // Then try socket shutdown
        beast::get_lowest_layer(stream).socket().shutdown(tcp::socket::shutdown_both, ec);
        if (ec == beast::errc::not_connected || ec == boost::asio::error::broken_pipe) {
            // These are expected if the peer has already closed their side
            ec = {}; // Clear the error
        }
        else if (ec) {
            // We don't throw here since we already have the response
        }
    }
    catch (const std::exception& e) {
        // Log but don't throw - we already have our response
        std::cerr << "HTTPClient: Exception during connection shutdown: " << e.what() 
                  << " (this is non-fatal)" << std::endl;
    }

    return response_body;
}

std::string HTTPClient::calculateSignature(const std::string& message, const std::string& secretKey)
{
    unsigned char hmacResult[EVP_MAX_MD_SIZE];
    unsigned int hmacLength;

    HMAC(EVP_sha256(), secretKey.c_str(), (int)secretKey.size(),
         reinterpret_cast<const unsigned char*>(message.c_str()), (int)message.size(),
         hmacResult, &hmacLength);

    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (unsigned int i = 0; i < hmacLength; ++i)
        ss << std::setw(2) << (int)hmacResult[i];

    return ss.str();
}