// CryptoClient.h

/*
Handles API interactions with CoinAPI
Responsibilities:
    Fetches current cryptocurrency prices
    Retrieves historical price data
    Converts timestamps
    Handles HTTP requests
 */

#pragma once // Ensures header is included only once during compilation
#include <string>
#include "httplib.h"
#include "json.hpp"
#include "CryptoData.h"

class CryptoClient {
public:
    // Constructor with API key for authentication
    CryptoClient(const std::string& apiKey);
    // Fetch current price for a cryptocurrency
    bool fetchPrice(const std::string& symbol, CryptoData& data);
    // Retrieve historical price data
    bool fetchHistoricalData(const std::string& symbol, CryptoData& data);

private:
    std::string apiKey;
    // HTTP client configured to connect to CoinAPI
    httplib::Client cli{"http://rest.coinapi.io"};
    // Convert time point to formatted string for API requests
    std::string getTimeString(const std::chrono::system_clock::time_point& time);
};