//==============================================================================
// HTTPClient - asynchronous-capable HTTPS client using Boost.Beast
//==============================================================================
#pragma once

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <openssl/evp.h>
#include <openssl/hmac.h>

#include <chrono>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
namespace ssl = net::ssl;

using tcp = net::ip::tcp;

class HTTPClient {
public:
  HTTPClient(net::io_context& ioc, ssl::context& ctx);
  ~HTTPClient();

  std::string get(const std::string& host,
                  const std::string& port,
                  const std::string& target,
                  const std::map<std::string, std::string>& extraHeaders = {});

  std::string post(const std::string& host,
                   const std::string& port,
                   const std::string& target,
                   const std::string& body,
                   const std::map<std::string, std::string>& extraHeaders = {});

  // HMAC-SHA256 signature (hex encoded)
  static std::string calculateSignature(const std::string& message, const std::string& secretKey);

private:
  net::io_context& ioc_;
  ssl::context& ctx_;

  void connectTLS(ssl::stream<beast::tcp_stream>& stream,
                  const std::string& host,
                  const std::string& port);

  std::string sendRequestAndGetResponse(ssl::stream<beast::tcp_stream>& stream,
                                        http::request<http::string_body>& req);
};

//==============================================================================
// implementation
//==============================================================================

HTTPClient::HTTPClient(net::io_context& ioc, ssl::context& ctx) : ioc_(ioc), ctx_(ctx) {
}

HTTPClient::~HTTPClient() {
  // std::cout << "HTTPClient destroyed" << std::endl;
}

void HTTPClient::connectTLS(ssl::stream<beast::tcp_stream>& stream,
                            const std::string& host,
                            const std::string& port) {
  beast::error_code ec;

  // set a timeout on the operation
  beast::get_lowest_layer(stream).expires_after(std::chrono::seconds(30));

  try {
    tcp::resolver resolver(ioc_);
    auto const results = resolver.resolve(host, port, ec);
    if (ec) {
      std::cerr << "HTTPClient: Resolve failed: " << ec.message() << std::endl;
      throw std::runtime_error("Resolve failed: " + ec.message());
    }

    beast::get_lowest_layer(stream).connect(results, ec);
    if (ec) {
      std::cerr << "HTTPClient: Connect failed: " << ec.message() << std::endl;
      throw std::runtime_error("Connect failed: " + ec.message());
    }

    // Set SNI Hostname (many hosts need this)
    if (!SSL_set_tlsext_host_name(stream.native_handle(), host.c_str())) {
      beast::error_code ssl_ec{static_cast<int>(::ERR_get_error()), net::error::get_ssl_category()};
      throw std::runtime_error("SNI set failed: " + ssl_ec.message());
    }

    stream.handshake(ssl::stream_base::client, ec);
    if (ec) {
      std::cerr << "HTTPClient: Handshake failed: " << ec.message() << std::endl;
      throw std::runtime_error("Handshake failed: " + ec.message());
    }
  } catch (const std::exception& e) {
    std::cerr << "HTTPClient: Exception in connectTLS: " << e.what() << std::endl;
    throw;
  }
}

std::string HTTPClient::get(const std::string& host,
                            const std::string& port,
                            const std::string& target,
                            const std::map<std::string, std::string>& extraHeaders) {
  ssl::stream<beast::tcp_stream> stream(ioc_, ctx_);
  connectTLS(stream, host, port);

  http::request<http::string_body> req{http::verb::get, target, 11};
  req.set(http::field::host, host);
  req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
  req.set(http::field::accept, "application/json");

  for (const auto& h : extraHeaders)
    req.set(h.first, h.second);

  return sendRequestAndGetResponse(stream, req);
}

std::string HTTPClient::post(const std::string& host,
                             const std::string& port,
                             const std::string& target,
                             const std::string& body,
                             const std::map<std::string, std::string>& extraHeaders) {
  ssl::stream<beast::tcp_stream> stream(ioc_, ctx_);
  connectTLS(stream, host, port);

  http::request<http::string_body> req{http::verb::post, target, 11};
  req.set(http::field::host, host);
  req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
  req.set(http::field::content_type, "application/json");

  for (const auto& h : extraHeaders) {
    if (!h.first.empty() && !h.second.empty())
      req.set(h.first, h.second);
  }

  req.body() = body;
  req.prepare_payload();

  return sendRequestAndGetResponse(stream, req);
}

std::string HTTPClient::sendRequestAndGetResponse(ssl::stream<beast::tcp_stream>& stream,
                                                  http::request<http::string_body>& req) {
  beast::error_code ec;

  http::write(stream, req, ec);
  if (ec) {
    throw std::runtime_error("Write failed: " + ec.message());
  }

  beast::flat_buffer buffer;
  http::response<http::string_body> res;

  beast::get_lowest_layer(stream).expires_after(std::chrono::seconds(30));
  http::read(stream, buffer, res, ec);
  if (ec) {
    throw std::runtime_error("Read failed: " + ec.message());
  }

  // Capture response body before any shutdown
  std::string response_body = res.body();

  // Graceful shutdown (best-effort)
  stream.shutdown(ec); // ignore common harmless errors
  beast::get_lowest_layer(stream).close();

  return response_body;
}

std::string HTTPClient::calculateSignature(const std::string& message, const std::string& secretKey) {
  unsigned char result[EVP_MAX_MD_SIZE];
  unsigned int result_len = 0;

  HMAC(EVP_sha256(), secretKey.data(), static_cast<int>(secretKey.size()),
       reinterpret_cast<const unsigned char*>(message.data()), static_cast<int>(message.size()),
       result, &result_len);

  std::ostringstream oss;
  oss << std::hex << std::setfill('0');
  for (unsigned int i = 0; i < result_len; ++i)
    oss << std::setw(2) << static_cast<unsigned>(result[i]);

  return oss.str();
}