// CrypoData.h

/*
Manages cryptocurrency price data storage
Key features:
    Stores price history
    Tracks current prices
    Provides thread-safe data access
    Can save/load price data to/from files
 */

#pragma once // Ensures header is included only once during compilation
#include <string>
#include <unordered_map>
#include <mutex>
#include <atomic>
#include <vector>
#include "json.hpp"

struct PricePoint {
    double price;           // Cryptocurrency price
    std::string timestamp;  // Time of price record
    double percentChange24h;// 24-hour price percentage change
    double priceChange24h;  // 24-hour price absolute change
};

class CryptoData {
public:
    // Update price information for a specific cryptocurrency
    void updatePrice(const std::string& symbol, double price, double percentChange24h, double priceChange24h);
    // Get comprehensive price data for a cryptocurrency
    PricePoint getPriceData(const std::string& symbol) const;
    // Save price history to a file for persistence
    void saveToFile(const std::string& symbol);
    // Retrieve historical price data for a cryptocurrency
    std::vector<PricePoint> getHistoricalData(const std::string& symbol) const;
    // Get the initial price when tracking began
    double getStartingPrice(const std::string& symbol) const;
    // Saving the data in a vector because we want the history to be indexed and in order
    void setHistoricalData(const std::string& symbol, const std::vector<PricePoint>& data);

private:
    // Mutex for thread-safe data access
    mutable std::mutex dataMutex;
    // unordered_maps to store price histories, current prices, and price data, for quick lookup by  cryptocurrency symbol
    std::unordered_map<std::string, std::vector<PricePoint>> priceHistory;
    // We use atomic here to prevent race condition
    std::unordered_map<std::string, std::atomic<double>> currentPrices;
    std::unordered_map<std::string, PricePoint> currentPriceData;
    std::unordered_map<std::string, double> startingPrices;
};