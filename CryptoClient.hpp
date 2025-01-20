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
    const std::string api_key = "bdbc3a83-8c73-4f95-8564-4fb4b51a17d1";
    std::string apiKey;
    httplib::Client cli{"https://rest.coinapi.io/v1/exchangerate/BTC/USD"};
};