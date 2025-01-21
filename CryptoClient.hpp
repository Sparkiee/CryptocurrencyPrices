// CryptoClient.hpp
#pragma once
#include <string>
#include "httplib.h"
#include "json.hpp"
#include "CryptoData.hpp"

class CryptoClient {
public:
    CryptoClient(const std::string& apiKey);
    bool fetchPrice(const std::string& symbol, CryptoData& data);

private:
    std::string apiKey;
    httplib::Client cli{"http://rest.coinapi.io"};
    double calculate24hChange(const std::vector<PricePoint>& history);
};