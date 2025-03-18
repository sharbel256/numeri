// constants.h
#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <string>

namespace API
{
    /*
    preview an order (POST https://api.coinbase.com/api/v3/brokerage/orders/preview)
    create an order (POST https://api.coinbase.com/api/v3/brokerage/orders)
    get order (GET https://api.coinbase.com/api/v3/brokerage/orders/historical/{order_id})
    list orders (GET https://api.coinbase.com/api/v3/brokerage/orders/historical/batch)
    list fills (GET https://api.coinbase.com/api/v3/brokerage/orders/historical/fills)
    batch cancel order (POST https://api.coinbase.com/api/v3/brokerage/orders/batch_cancel) 
    edit order (POST https://api.coinbase.com/api/v3/brokerage/orders/edit)
    edit order preview (POST https://api.coinbase.com/api/v3/brokerage/orders/edit_preview)
    close position (POST https://api.coinbase.com/api/v3/brokerage/orders/close_position)
     */

    // create constants for the above API endpoints
    const std::string HOST = "api.coinbase.com";
    const std::string PREVIEW_ORDER = "/api/v3/brokerage/orders/preview";
    const std::string CREATE_ORDER = "/api/v3/brokerage/orders";
    const std::string GET_ORDER = "/api/v3/brokerage/orders/historical/";
    const std::string LIST_ORDERS = "/api/v3/brokerage/orders/historical/batch";
    const std::string LIST_FILLS = "/api/v3/brokerage/orders/historical/fills";
    const std::string BATCH_CANCEL_ORDER = "/api/v3/brokerage/orders/batch_cancel";
    const std::string EDIT_ORDER = "/api/v3/brokerage/orders/edit";
    const std::string EDIT_ORDER_PREVIEW = "/api/v3/brokerage/orders/edit_preview";
    const std::string CLOSE_POSITION = "/api/v3/brokerage/orders/close_position";
}

#endif // CONSTANTS_H