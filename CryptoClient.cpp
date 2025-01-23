// CryptoClient.cpp
#include "CryptoClient.h"
#include <chrono> // Time related utiliti, provides duration and time points
#include <iomanip> // Formats time output
#include <sstream> // Used to construct time strings

// Constructor initializes HTTP client with connection timeouts
CryptoClient::CryptoClient(const std::string& apiKey) : apiKey(apiKey) {
    // Set connection and read timeouts to prevent hanging
    cli.set_connection_timeout(10);
    cli.set_read_timeout(10);
}

// Convert time point to formatted string for API requests
std::string CryptoClient::getTimeString(const std::chrono::system_clock::time_point& time) {
    // Convert time point to time_t
    auto itt = std::chrono::system_clock::to_time_t(time);
    // Use string stream to format time
    std::ostringstream ss;
    // Using iomanip to create  formatted timestamps
    ss << std::put_time(std::gmtime(&itt), "%Y-%m-%dT%H:%M:%S.000Z");
    return ss.str();
}

// Fetch historical price data for a cryptocurrency
bool CryptoClient::fetchHistoricalData(const std::string& symbol, CryptoData& data) {
    // Calculate time range for last 24 hours
    auto now = std::chrono::system_clock::now();
    auto yesterday = now - std::chrono::hours(24);

    // Construct API URL with time range
    std::string url = "/v1/exchangerate/" + symbol + "/USD/history?period_id=1MIN&time_start=" +
                     getTimeString(yesterday) + "&time_end=" + getTimeString(now);

    // Set API key in request headers
    httplib::Headers headers = {{"X-CoinAPI-Key", apiKey}};
    // Send GET request to CoinAPI
    auto res = cli.Get(url.c_str(), headers);

    if (res && res->status == 200) {
        try {
            // Parse JSON response
            auto json = nlohmann::json::parse(res->body);
            std::vector<PricePoint> history;

            // Convert JSON data to PricePoint vector
            for (const auto& point : json) {
                PricePoint pricePoint{
                    point["rate_close"].get<double>(),
                    point["time_period_start"].get<std::string>(),
                    0.0,  // Will be calculated later
                    0.0   // Will be calculated later
                };
                history.push_back(pricePoint);
            }

            // Store historical data in CryptoData
            data.setHistoricalData(symbol, history);
            return true;
        } catch (const std::exception& e) {
            // Log any JSON parsing errors
            std::cerr << "JSON parsing error: " << e.what() << std::endl;
            return false;
        }
    }
    return false;
}

// Fetch current price for a cryptocurrency
bool CryptoClient::fetchPrice(const std::string& symbol, CryptoData& data) {
    // First fetch historical data if we don't have it
    if (data.getHistoricalData(symbol).empty()) {
        fetchHistoricalData(symbol, data);
    }

    // Construct API URL for current exchange rate
    std::string url = "/v1/exchangerate/" + symbol + "/USD";
    httplib::Headers headers = {{"X-CoinAPI-Key", apiKey}};
    // Send GET request to CoinAPI
    auto res = cli.Get(url.c_str(), headers);

    if (res && res->status == 200) {
        try {
            // Parse JSON response
            auto json = nlohmann::json::parse(res->body);
            double currentPrice = json["rate"].get<double>();

            // Get historical data for 24h comparison
            auto history = data.getHistoricalData(symbol);
            double price24hAgo = (!history.empty()) ? history.front().price : currentPrice;

            // Calculate price changes
            double priceChange24h = currentPrice - price24hAgo;
            double percentChange24h = (price24hAgo != 0) ? (priceChange24h / price24hAgo) * 100.0 : 0.0;

            // Update price in CryptoData
            data.updatePrice(symbol, currentPrice, percentChange24h, priceChange24h);
            return true;
        } catch (const std::exception& e) {
            std::cerr << "JSON parsing error: " << e.what() << std::endl;
            return false;
        }
    }
    return false;
}