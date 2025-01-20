// CryptoClient.cpp
#include "CryptoClient.hpp"

CryptoClient::CryptoClient(const std::string& apiKey) : apiKey(apiKey) {
    cli.set_connection_timeout(10);
    cli.set_read_timeout(10);
}

bool CryptoClient::fetchPrice(const std::string& symbol, CryptoData& data) {
    std::string url = "/v1/exchangerate/" + symbol + "/USD";
    httplib::Headers headers = {{"X-CoinAPI-Key", apiKey}};

    auto res = cli.Get(url.c_str(), headers);

    if (res && res->status == 200) {
        try {
            auto json = nlohmann::json::parse(res->body);
            double price = json["rate"].get<double>();
            data.updatePrice(symbol, price);
            return true;
        } catch (const std::exception& e) {
            std::cerr << "JSON parsing error: " << e.what() << std::endl;
            return false;
        }
    }

    return false;
}