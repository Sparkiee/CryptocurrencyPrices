// CryptoClient.cpp
#include "CryptoClient.hpp"

CryptoClient::CryptoClient(const std::string& apiKey) : apiKey(apiKey) {
    cli.set_connection_timeout(10);
    cli.set_read_timeout(10);
}

bool CryptoClient::fetchPrice(const std::string& symbol, CryptoData& data) {
    // Get current price
    std::string url = "/v1/exchangerate/" + symbol + "/USD";
    httplib::Headers headers = {{"X-CoinAPI-Key", apiKey}};

    auto res = cli.Get(url.c_str(), headers);

    if (res && res->status == 200) {
        try {
            auto json = nlohmann::json::parse(res->body);
            double currentPrice = json["rate"].get<double>();

            // Get historical data for 24h ago
            auto history = data.getHistoricalData(symbol);
            double price24hAgo = (!history.empty() && history.size() > 1440) ?
                                history[history.size() - 1440].price : currentPrice;

            // Calculate 24h changes
            double priceChange24h = currentPrice - price24hAgo;
            double percentChange24h = (price24hAgo != 0) ?
                                    (priceChange24h / price24hAgo) * 100.0 : 0.0;

            // Update the data with all values
            data.updatePrice(symbol, currentPrice, percentChange24h, priceChange24h);
            return true;
        } catch (const std::exception& e) {
            std::cerr << "JSON parsing error: " << e.what() << std::endl;
            return false;
        }
    }

    return false;
}