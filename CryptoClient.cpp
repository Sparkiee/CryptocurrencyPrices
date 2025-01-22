// CryptoClient.cpp
#include "CryptoClient.h"
#include <chrono>
#include <iomanip>
#include <sstream>

CryptoClient::CryptoClient(const std::string& apiKey) : apiKey(apiKey) {
    cli.set_connection_timeout(10);
    cli.set_read_timeout(10);
}

std::string CryptoClient::getTimeString(const std::chrono::system_clock::time_point& time) {
    auto itt = std::chrono::system_clock::to_time_t(time);
    std::ostringstream ss;
    ss << std::put_time(std::gmtime(&itt), "%Y-%m-%dT%H:%M:%S.000Z");
    return ss.str();
}

bool CryptoClient::fetchHistoricalData(const std::string& symbol, CryptoData& data) {
    auto now = std::chrono::system_clock::now();
    auto yesterday = now - std::chrono::hours(24);

    std::string url = "/v1/exchangerate/" + symbol + "/USD/history?period_id=1MIN&time_start=" +
                     getTimeString(yesterday) + "&time_end=" + getTimeString(now);

    httplib::Headers headers = {{"X-CoinAPI-Key", apiKey}};
    auto res = cli.Get(url.c_str(), headers);

    if (res && res->status == 200) {
        try {
            auto json = nlohmann::json::parse(res->body);
            std::vector<PricePoint> history;

            for (const auto& point : json) {
                PricePoint pricePoint{
                    point["rate_close"].get<double>(),
                    point["time_period_start"].get<std::string>(),
                    0.0,  // Will be calculated later
                    0.0   // Will be calculated later
                };
                history.push_back(pricePoint);
            }

            data.setHistoricalData(symbol, history);
            return true;
        } catch (const std::exception& e) {
            std::cerr << "JSON parsing error: " << e.what() << std::endl;
            return false;
        }
    }
    return false;
}

bool CryptoClient::fetchPrice(const std::string& symbol, CryptoData& data) {
    // First fetch historical data if we don't have it
    if (data.getHistoricalData(symbol).empty()) {
        fetchHistoricalData(symbol, data);
    }

    std::string url = "/v1/exchangerate/" + symbol + "/USD";
    httplib::Headers headers = {{"X-CoinAPI-Key", apiKey}};
    auto res = cli.Get(url.c_str(), headers);

    if (res && res->status == 200) {
        try {
            auto json = nlohmann::json::parse(res->body);
            double currentPrice = json["rate"].get<double>();

            auto history = data.getHistoricalData(symbol);
            double price24hAgo = (!history.empty()) ? history.front().price : currentPrice;

            double priceChange24h = currentPrice - price24hAgo;
            double percentChange24h = (price24hAgo != 0) ? (priceChange24h / price24hAgo) * 100.0 : 0.0;

            data.updatePrice(symbol, currentPrice, percentChange24h, priceChange24h);
            return true;
        } catch (const std::exception& e) {
            std::cerr << "JSON parsing error: " << e.what() << std::endl;
            return false;
        }
    }
    return false;
}