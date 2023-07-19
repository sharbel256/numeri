#include "httpclient.h"


HTTPClient::HTTPClient(net::any_io_executor ex,ssl::context& ctx)
    : resolver_(ex)
    , stream_(ex, ctx)
    {}

HTTPClient::~HTTPClient() {
    // @todo: implement proper HTTPClient closure
    std::cout << "TODO: HTTPClient::~HTTPClient()" << std::endl;
}

void HTTPClient::shutdown() {
    std::cout << "HTTPClient::shutdown()" << std::endl;

    // Gracefully close the stream
    stream_.async_shutdown(
            beast::bind_front_handler(&HTTPClient::on_shutdown,shared_from_this()));

    // not_connected happens sometimes so don't bother reporting it.
    // if(ec && ec != beast::errc::not_connected) return fail(ec, "shutdown");
}

// Start the asynchronous operation
void HTTPClient::run(const char* host, const char* port)
{
    // Set SNI Hostname (many hosts need this to handshake successfully)
    if(! SSL_set_tlsext_host_name(stream_.native_handle(), host))
    {
        beast::error_code ec{static_cast<int>(::ERR_get_error()), net::error::get_ssl_category()};
        std::cerr << ec.message() << "\n";
        return;
    }

    std::string method = "GET";
    std::string requestPath = "/api/v3/brokerage/accounts";
    std::string body = "";

    // Load your API key, signature, and timestamp into variables
    std::string apiKey = std::getenv("COINBASE_API_KEY");
    std::string secretKey = std::getenv("COINBASE_SECRET_KEY");

    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
    std::string timestampString = std::to_string(timestamp);

    std::string message = timestampString + method + requestPath + body;
    std::string signature = calculateSignature(message, secretKey);

    req_.method(http::verb::get);
    req_.target("/api/v3/brokerage/accounts");
    req_.version(11); // HTTP/1.1
    req_.set(http::field::host, "api.coinbase.com");
    req_.set(http::field::content_type, "application/json");
    req_.set("CB-ACCESS-KEY", apiKey);
    req_.set("CB-VERSION", "2021-01-09");
    req_.set("CB-ACCESS-TIMESTAMP", timestampString);
    req_.set("CB-ACCESS-SIGN", signature);

    // Look up the domain name
    resolver_.async_resolve(host, port,
        beast::bind_front_handler(&HTTPClient::on_resolve,shared_from_this()));
}

void HTTPClient::on_resolve(beast::error_code ec, tcp::resolver::results_type results) {
    if(ec) return fail(ec, "resolve");

    // Set a timeout on the operation
    beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(30));

    // Make the connection on the IP address we get from a lookup
    beast::get_lowest_layer(stream_).async_connect(
        results,
        beast::bind_front_handler(&HTTPClient::on_connect,shared_from_this()));
}

void HTTPClient::on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type) {
        if(ec) return fail(ec, "connect");

        // Perform the SSL handshake
        stream_.async_handshake(ssl::stream_base::client,
            beast::bind_front_handler(&HTTPClient::on_handshake,shared_from_this()));
}

void HTTPClient::on_handshake(beast::error_code ec)
{
    if(ec) return fail(ec, "handshake");

    // Set a timeout on the operation
    beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(30));

    // Send the HTTP request to the remote host
    http::async_write(stream_, req_,
        beast::bind_front_handler(&HTTPClient::on_write,shared_from_this()));
}

void HTTPClient::on_write(beast::error_code ec, std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);

    if(ec) return fail(ec, "write");

    // Receive the HTTP response
    http::async_read(stream_, buffer_, res_,
        beast::bind_front_handler(&HTTPClient::on_read, shared_from_this()));
}

void HTTPClient::on_read(beast::error_code ec, std::size_t bytes_transferred) {
    boost::ignore_unused(bytes_transferred);

    if(ec) return fail(ec, "read");

    // Write the message to standard out
    std::cout << res_ << std::endl;

    if (onReadCallback) {
        onReadCallback(res_.body());
    }

    // Set a timeout on the operation
    beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(30));
}

void HTTPClient::on_shutdown(beast::error_code ec) {
    if(ec) return fail(ec, "shutdown");

    // If we get here then the connection is closed gracefully
    stopped = true;
}

std::string HTTPClient::calculateSignature(const std::string& message, const std::string& secretKey)
{
    unsigned char hmacResult[EVP_MAX_MD_SIZE];
    unsigned int hmacLength;

    HMAC(EVP_sha256(), secretKey.c_str(), secretKey.length(),
         reinterpret_cast<const unsigned char*>(message.c_str()), message.length(), 
         hmacResult, &hmacLength);

    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for(unsigned int i = 0; i < hmacLength; ++i)
        ss << std::setw(2) << static_cast<unsigned>(hmacResult[i]);

    return ss.str();
}

void HTTPClient::setReadCallback(ReadCallback callback) {
    this->onReadCallback = callback;
}