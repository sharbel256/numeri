#ifndef LOGGING_HEADER
#define LOGGING_HEADER

#include <boost/beast/core.hpp>

#include <iostream>

namespace beast = boost::beast; // from <boost/beast.hpp>

inline void fail(beast::error_code ec, char const* what) {
  std::cerr << what << ": " << ec.message() << "\n";
}

#endif