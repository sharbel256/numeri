//------------------------------------------------------------------------------
//
// HTTP client, asynchronous
//
//------------------------------------------------------------------------------
#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>

#include <chrono>
#include <iomanip>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
namespace ssl = boost::asio::ssl;
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

  std::string calculateSignature(const std::string& message, const std::string& secretKey);

private:
  net::io_context& ioc_;
  ssl::context& ctx_;

  void connectTLS(ssl::stream<beast::tcp_stream>& stream,
                  const std::string& host,
                  const std::string& port);
  std::string sendRequestAndGetResponse(ssl::stream<beast::tcp_stream>& stream,
                                        http::request<http::string_body>& req);
};

#endif // HTTPCLIENT_H