// CryptoClient.h
#pragma once
#include <string>
#include "httplib.h"
#include "json.hpp"
#include "CryptoData.h"

class CryptoClient {
public:
    CryptoClient(const std::string& apiKey);
    bool fetchPrice(const std::string& symbol, CryptoData& data);
    bool fetchHistoricalData(const std::string& symbol, CryptoData& data);

private:
    std::string apiKey;
    httplib::Client cli{"http://rest.coinapi.io"};
    std::string getTimeString(const std::chrono::system_clock::time_point& time);
};