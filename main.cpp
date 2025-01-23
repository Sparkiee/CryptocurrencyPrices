// main.cpp

/*
Data Fetching Thread:
    Periodically retrieves prices for multiple cryptocurrencies
    Saves price data to files
    Runs every 60 seconds

UI Rendering:
    Creates an interactive dashboard
    Displays cryptocurrency prices
    Allows selecting and viewing individual cryptocurrency charts
    Provides tooltips and detailed price information
 */

#include <thread>
#include <chrono>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include "CryptoClient.h"
#include "CryptoData.h"

// Thread-safe flag to signal application exit
std::atomic<bool> shouldExit{false};

// Background thread to periodically fetch cryptocurrency prices
void dataFetchingThread(CryptoClient& client, CryptoData& data) {
    // List of cryptocurrencies to track
    const std::vector<std::string> symbols = {
        "BTC", "ETH", "XRP", "SOL", "ADA", "BNB", "SHIB", "DOGE", "PEPE", "USDT"
    };

    // Continuously fetch prices until exit is signaled
    while (!shouldExit) {
        for (const auto& symbol : symbols) {
            // Fetch and save price data for each cryptocurrency
            if (client.fetchPrice(symbol, data)) {
                data.saveToFile(symbol);
            }
        }
        // Wait for 1 minute before next update
        std::this_thread::sleep_for(std::chrono::seconds(60));
    }
}

// Render interactive price chart for a selected cryptocurrency
void renderPriceChart(const std::string& symbol, const std::vector<PricePoint>& history) {
    // Skip rendering if no history available
    if (history.empty()) return;

    // Increase font size for better readability
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
    ImGui::SetWindowFontScale(1.5f);

    // Prepare price data for plotting
    std::vector<float> prices;
    float minPrice = FLT_MAX;
    float maxPrice = -FLT_MAX;

    // Convert prices and find min/max for chart scaling
    for (const auto& point : history) {
        prices.push_back(static_cast<float>(point.price));
        minPrice = std::min(minPrice, static_cast<float>(point.price));
        maxPrice = std::max(maxPrice, static_cast<float>(point.price));
    }

    // Plot price line chart with large dimensions
    ImGui::PlotLines(("##" + symbol).c_str(),
                     prices.data(),
                     prices.size(),
                     0,
                     nullptr,
                     minPrice,
                     maxPrice,
                     ImVec2(1000, 580));  // Increased chart size

    // Add interactive tooltip showing time and price on hover
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::SetWindowFontScale(1.2f);  // Tooltip text size

        // Calculate which price point is being hovered
        int pos = static_cast<int>((ImGui::GetIO().MousePos.x - ImGui::GetItemRectMin().x) /
                                  ImGui::GetItemRectSize().x * (prices.size() - 1));
        if (pos >= 0 && pos < history.size()) {
            const auto& point = history[pos];
            ImGui::Text("Time: %s", point.timestamp.c_str());
            ImGui::Text("Price: $%.2f", point.price);
        }
        ImGui::EndTooltip();
    }

    ImGui::PopFont();
}

int main() {
    // Initialize GLFW for window and OpenGL context
    if (!glfwInit()) return 1;
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Crypto Dashboard", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);

    // Initialize ImGui with larger font
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 120");

    // Configure larger base font
    ImFontConfig config;
    config.SizePixels = 20.0f;  // Increased base font size
    io.Fonts->AddFontDefault(&config);


    // Initialize cryptocurrency data management and client
    CryptoData data;
    CryptoClient client("bdbc3a83-8c73-4f95-8564-4fb4b51a17d1");

    // Start background data fetching thread
    std::thread fetchThread(dataFetchingThread, std::ref(client), std::ref(data));


    // Main UI rendering loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Create main application window
        ImGui::Begin("Cryptocurrency Dashboard", nullptr,
                    ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

        // Set initial window size
        ImGui::SetWindowSize(ImVec2(1600, 900), ImGuiCond_FirstUseEver);

        // List of tracked cryptocurrencies
        const std::vector<std::string> symbols = {
            "BTC", "ETH", "XRP", "SOL", "ADA", "BNB", "SHIB", "DOGE", "PEPE", "USDT"
        };

        // Track selected cryptocurrency for detailed view
        static std::string selectedSymbol;

        // Cryptocurrency price list sidebar
        ImGui::BeginChild("Prices", ImVec2(350, 0), true);  // Increased width
        for (const auto& symbol : symbols) {
            // Create selectable list item for each cryptocurrency
            auto priceData = data.getPriceData(symbol);
            if (ImGui::Selectable(symbol.c_str(), selectedSymbol == symbol)) {
                selectedSymbol = symbol;
            }
            ImGui::SameLine(150);  // Increased spacing

            // Format price with commas for readability
            std::string priceStr = std::format("{:.2f}", priceData.price);
            size_t decimalPos = priceStr.find('.');
            std::string integerPart = priceStr.substr(0, decimalPos);
            std::string decimalPart = priceStr.substr(decimalPos);

            // Add commas for thousands separator
            for (int i = integerPart.length() - 3; i > 0; i -= 3) {
                integerPart.insert(i, ",");
            }

            ImGui::Text("$%s%s", integerPart.c_str(), decimalPart.c_str());
        }
        ImGui::EndChild();

        ImGui::SameLine();

        // Detailed view for selected cryptocurrency
        if (!selectedSymbol.empty()) {
            ImGui::BeginChild("Chart", ImVec2(0, 0), true);
            auto priceData = data.getPriceData(selectedSymbol);

            // Format numbers with commas for display
            std::string priceStr = std::format("{:.2f}", priceData.price);
            std::string startPriceStr = std::format("{:.2f}", data.getStartingPrice(selectedSymbol));

            // Add comma separators to price strings
            for (auto* str : {&priceStr, &startPriceStr}) {
                size_t decimalPos = str->find('.');
                std::string integerPart = str->substr(0, decimalPos);
                std::string decimalPart = str->substr(decimalPos);

                for (int i = integerPart.length() - 3; i > 0; i -= 3) {
                    integerPart.insert(i, ",");
                }
                *str = integerPart + decimalPart;
            }

            // Display cryptocurrency details
            ImGui::Text("%s Price Chart", selectedSymbol.c_str());
            ImGui::Text("Current Price: $%s", priceStr.c_str());
            ImGui::Text("24h Change: %.2f%% ($%.2f)",
                        priceData.percentChange24h,
                        priceData.priceChange24h);
            ImGui::Text("Starting Price: $%s", startPriceStr.c_str());

            // Render price history chart
            auto history = data.getHistoricalData(selectedSymbol);
            renderPriceChart(selectedSymbol, history);
            ImGui::EndChild();
        }

        ImGui::End();

        // Render ImGui content
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    // Cleanup
    shouldExit = true;
    fetchThread.join();

    // Shutdown ImGui and GLFW
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}