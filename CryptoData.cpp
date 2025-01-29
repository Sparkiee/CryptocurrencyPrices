// CryptoData.cpp
#include "CryptoData.h"
#include <fstream>
#include <filesystem>

void CryptoData::updatePrice(const std::string& symbol, double price,
                             double percentChange24h, double priceChange24h) {
    // A safe why for automatic locking/unlocking, for thread synchronization and prevention of data races when multiple threads access shared data
    std::lock_guard<std::mutex> lock(dataMutex);

    // Store the initial price only once when first tracking the symbol
    if (startingPrices.find(symbol) == startingPrices.end()) {
        startingPrices[symbol] = price;
    }
    // Atomically update the current price
    currentPrices[symbol].store(price);

    // Create a new price point with current timestamp and price details
    PricePoint point{
        price,
        std::to_string(std::chrono::system_clock::now().time_since_epoch().count()),
        percentChange24h,
        priceChange24h
    };
    // Update current price data and price history
    currentPriceData[symbol] = point;
    priceHistory[symbol].push_back(point);

    // Keep only last 1440 price points (24 hours at 1-minute intervals)
    if (priceHistory[symbol].size() > 1440) {
        priceHistory[symbol].erase(priceHistory[symbol].begin());
    }
}

// Retrieve comprehensive price data for a specific cryptocurrency
PricePoint CryptoData::getPriceData(const std::string& symbol) const {
    std::lock_guard<std::mutex> lock(dataMutex);
    auto it = currentPriceData.find(symbol);
    // Return price data or a default zero-filled price point
    return (it != currentPriceData.end()) ? it->second : PricePoint{0.0, "", 0.0, 0.0};
}

// Save price history to a file for persistent storage
void CryptoData::saveToFile(const std::string& symbol) {
    std::lock_guard<std::mutex> lock(dataMutex);
    // Create data directory if it doesn't exist
    std::filesystem::create_directories("data");
    // Open file for writing price history
    std::ofstream file("data/" + symbol + "_history.txt");

    if (file.is_open()) {
        // Set precision to ensure full number representation
        file << std::fixed << std::setprecision(10);
        // Write each price point as timestamp,price
        for (const auto& point : priceHistory[symbol]) {
            file << point.timestamp << "," << point.price << "\n";
        }
    }
}

void CryptoData::setHistoricalData(const std::string& symbol, const std::vector<PricePoint>& data) {
    std::lock_guard<std::mutex> lock(dataMutex);
    priceHistory[symbol] = data;
}

double CryptoData::getStartingPrice(const std::string& symbol) const {
    std::lock_guard<std::mutex> lock(dataMutex);
    auto it = startingPrices.find(symbol);
    return (it != startingPrices.end()) ? it->second : 0.0;
}

std::vector<PricePoint> CryptoData::getHistoricalData(const std::string& symbol) const {
    std::lock_guard<std::mutex> lock(dataMutex);
    auto it = priceHistory.find(symbol);
    return (it != priceHistory.end()) ? it->second : std::vector<PricePoint>{};
}

