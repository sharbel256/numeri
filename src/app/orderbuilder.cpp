#include <string>
#include <nlohmann/json.hpp>
#include <chrono>
#include <iomanip>
#include <sstream>

using json = nlohmann::json;

class OrderBuilder {
public:
    OrderBuilder() = default;

    OrderBuilder& setClientOrderId(const std::string& clientOrderId) {
        client_order_id_ = clientOrderId;
        return *this;
    }

    OrderBuilder& setProductId(const std::string& productId) {
        product_id_ = productId;
        return *this;
    }

    OrderBuilder& setSide(const std::string& side) {
        side_ = side;
        return *this;
    }

    OrderBuilder& marketMarketIoc(const std::string& quoteSize) {
        order_type_ = "market_market_ioc";
        order_config_["quote_size"] = quoteSize;
        return *this;
    }

    OrderBuilder& limitLimitGtc(const std::string& baseSize, 
                               const std::string& limitPrice, 
                               bool postOnly = false) {
        order_type_ = "limit_limit_gtc";
        order_config_["base_size"] = baseSize;
        order_config_["limit_price"] = limitPrice;
        order_config_["post_only"] = postOnly;
        return *this;
    }

    OrderBuilder& limitLimitGtd(const std::string& baseSize, 
                               const std::string& limitPrice, 
                               const std::string& endTime, 
                               bool postOnly = false) {
        order_type_ = "limit_limit_gtd";
        order_config_["base_size"] = baseSize;
        order_config_["limit_price"] = limitPrice;
        order_config_["end_time"] = endTime;
        order_config_["post_only"] = postOnly;
        return *this;
    }

    OrderBuilder& stopLimitGtc(const std::string& baseSize, 
                              const std::string& limitPrice, 
                              const std::string& stopPrice, 
                              bool postOnly = false) {
        order_type_ = "stop_limit_stop_limit_gtc";
        order_config_["base_size"] = baseSize;
        order_config_["limit_price"] = limitPrice;
        order_config_["stop_price"] = stopPrice;
        order_config_["post_only"] = postOnly;
        return *this;
    }

    OrderBuilder& stopLimitGtd(const std::string& baseSize, 
                              const std::string& limitPrice, 
                              const std::string& stopPrice, 
                              const std::string& endTime, 
                              bool postOnly = false) {
        order_type_ = "stop_limit_stop_limit_gtd";
        order_config_["base_size"] = baseSize;
        order_config_["limit_price"] = limitPrice;
        order_config_["stop_price"] = stopPrice;
        order_config_["end_time"] = endTime;
        order_config_["post_only"] = postOnly;
        return *this;
    }

    std::string build() const {
        if (product_id_.empty() || side_.empty() || order_type_.empty()) {
            throw std::runtime_error("Missing required fields: product_id, side, or order type");
        }

        json j;
        if (!client_order_id_.empty()) {
            j["client_order_id"] = client_order_id_;
        }
        j["product_id"] = product_id_;
        j["side"] = side_;
        j["order_configuration"][order_type_] = order_config_;

        return j.dump(2);
    }

private:
    std::string client_order_id_; // Empty means not set
    std::string product_id_;      // Required
    std::string side_;            // Required
    std::string order_type_;      // Required
    json order_config_;
};

inline std::string getIsoTimestamp(std::chrono::system_clock::time_point time = std::chrono::system_clock::now()) {
    auto tt = std::chrono::system_clock::to_time_t(time);
    std::stringstream ss;
    ss << std::put_time(std::gmtime(&tt), "%Y-%m-%dT%H:%M:%SZ");
    return ss.str();
}