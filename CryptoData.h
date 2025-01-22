// CrypoData.h
#pragma once
#include <string>
#include <unordered_map>
#include <mutex>
#include <atomic>
#include <vector>
#include "json.hpp"

struct PricePoint {
    double price;
    std::string timestamp;
    double percentChange24h;
    double priceChange24h;
};

class CryptoData {
public:
    void updatePrice(const std::string& symbol, double price, double percentChange24h, double priceChange24h);
    double getPrice(const std::string& symbol) const;
    PricePoint getPriceData(const std::string& symbol) const;
    void saveToFile(const std::string& symbol);
    void loadFromFile(const std::string& symbol);
    std::vector<PricePoint> getHistoricalData(const std::string& symbol) const;
    double getStartingPrice(const std::string& symbol) const;
    void setHistoricalData(const std::string& symbol, const std::vector<PricePoint>& data);

private:
    mutable std::mutex dataMutex;
    std::unordered_map<std::string, std::vector<PricePoint>> priceHistory;
    std::unordered_map<std::string, std::atomic<double>> currentPrices;
    std::unordered_map<std::string, PricePoint> currentPriceData;
    std::unordered_map<std::string, double> startingPrices;

};