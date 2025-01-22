// CryptoData.cpp
#include "CryptoData.h"
#include <fstream>
#include <filesystem>

void CryptoData::updatePrice(const std::string& symbol, double price,
                             double percentChange24h, double priceChange24h) {
    std::lock_guard<std::mutex> lock(dataMutex);

    // Store the starting price only once
    if (startingPrices.find(symbol) == startingPrices.end()) {
        startingPrices[symbol] = price;
    }

    currentPrices[symbol].store(price);

    PricePoint point{
        price,
        std::to_string(std::chrono::system_clock::now().time_since_epoch().count()),
        percentChange24h,
        priceChange24h
    };

    currentPriceData[symbol] = point;
    priceHistory[symbol].push_back(point);

    // Keep only last 1440 price points (24 hours at 1-minute intervals)
    if (priceHistory[symbol].size() > 1440) {
        priceHistory[symbol].erase(priceHistory[symbol].begin());
    }
}

PricePoint CryptoData::getPriceData(const std::string& symbol) const {
    std::lock_guard<std::mutex> lock(dataMutex);
    auto it = currentPriceData.find(symbol);
    return (it != currentPriceData.end()) ? it->second : PricePoint{0.0, "", 0.0, 0.0};
}

double CryptoData::getPrice(const std::string& symbol) const {
    std::lock_guard<std::mutex> lock(dataMutex);
    auto it = currentPrices.find(symbol);
    return (it != currentPrices.end()) ? it->second.load() : 0.0;
}

void CryptoData::saveToFile(const std::string& symbol) {
    std::lock_guard<std::mutex> lock(dataMutex);
    std::filesystem::create_directories("data");
    std::ofstream file("data/" + symbol + "_history.txt");

    if (file.is_open()) {
        for (const auto& point : priceHistory[symbol]) {
            file << point.timestamp << "," << point.price << "\n";
        }
    }
}

void CryptoData::loadFromFile(const std::string& symbol) {
    std::lock_guard<std::mutex> lock(dataMutex);
    std::ifstream file("data/" + symbol + "_history.txt");

    if (file.is_open()) {
        priceHistory[symbol].clear();
        std::string line;
        while (std::getline(file, line)) {
            size_t comma = line.find(',');
            if (comma != std::string::npos) {
                PricePoint point{
                    std::stod(line.substr(comma + 1)),
                    line.substr(0, comma)
                };
                priceHistory[symbol].push_back(point);
            }
        }
    }
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

