#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <atomic>
#include <fstream>
#include <filesystem>
#include <chrono>
#include "httplib.h"
#include "json.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>

// Structure to hold cryptocurrency data
struct CryptoData {
    double price;
    std::string timestamp;
    std::vector<double> historical_prices;
};

class CryptoDashboard {
private:
    // Data storage
    std::unordered_map<std::string, CryptoData> crypto_prices;
    std::mutex prices_mutex;
    std::atomic<bool> should_stop{false};

    // Thread for data fetching
    std::thread fetch_thread;

    // API details
    const std::string api_key = "bdbc3a83-8c73-4f95-8564-4fb4b51a17d1";
    const std::string base_url = "rest.coinapi.io";

    // File handling
    const std::string data_directory = "crypto_data/";

    void saveToFile(const std::string& symbol, const CryptoData& data) {
        std::filesystem::create_directories(data_directory);
        std::ofstream file(data_directory + symbol + "_history.csv", std::ios::app);
        if (file.is_open()) {
            file << data.timestamp << "," << data.price << "\n";
        }
    }

    void fetchPrices() {
        httplib::Client cli(base_url);
        httplib::Headers headers = {{"X-CoinAPI-Key", api_key}};

        while (!should_stop) {
            // Fetch data for multiple cryptocurrencies
            std::vector<std::string> symbols = {"BTC", "ETH", "DOGE"};

            for (const auto& symbol : symbols) {
                auto result = cli.Get("/v1/exchangerate/" + symbol + "/USD", headers);

                if (result && result->status == 200) {
                    try {
                        auto json_response = nlohmann::json::parse(result->body);

                        // Update prices with mutex protection
                        std::lock_guard<std::mutex> lock(prices_mutex);
                        crypto_prices[symbol].price = json_response["rate"].get<double>();
                        crypto_prices[symbol].timestamp = json_response["time"].get<std::string>();
                        crypto_prices[symbol].historical_prices.push_back(json_response["rate"].get<double>();

                        // Save to file
                        saveToFile(symbol, crypto_prices[symbol]);

                    } catch (const nlohmann::json::exception& e) {
                        std::cerr << "JSON parsing error: " << e.what() << std::endl;
                    }
                }
            }

            // Wait for 10 seconds before next update
            std::this_thread::sleep_for(std::chrono::seconds(10));
        }
    }

public:
    CryptoDashboard() {
        // Start the fetch thread
        fetch_thread = std::thread(&CryptoDashboard::fetchPrices, this);
    }

    ~CryptoDashboard() {
        should_stop = true;
        if (fetch_thread.joinable()) {
            fetch_thread.join();
        }
    }

    void renderUI() {
        ImGui::Begin("Cryptocurrency Dashboard");

        // Lock the data while rendering
        std::lock_guard<std::mutex> lock(prices_mutex);

        for (const auto& [symbol, data] : crypto_prices) {
            ImGui::Text("%s Price: $%.2f", symbol.c_str(), data.price);
            ImGui::Text("Last Updated: %s", data.timestamp.c_str());

            // Add a simple price chart if we have historical data
            if (!data.historical_prices.empty()) {
                ImGui::PlotLines(("##" + symbol).c_str(),
                               data.historical_prices.data(),
                               data.historical_prices.size(),
                               0, nullptr, FLT_MAX, FLT_MAX,
                               ImVec2(200, 80));
            }

            ImGui::Separator();
        }

        ImGui::End();
    }
};

// Main application
int main() {
    // Initialize GLFW and OpenGL
    if (!glfwInit())
        return -1;

    GLFWwindow* window = glfwCreateWindow(800, 600, "Crypto Dashboard", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    // Create dashboard
    CryptoDashboard dashboard;

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        dashboard.renderUI();

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}