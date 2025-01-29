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
#include <fmt/core.h>

// Thread-safe flag to signal application exit
std::atomic<bool> shouldExit{false};

// Background thread to periodically fetch cryptocurrency prices
void fetchPriceForSymbol(CryptoClient &client, CryptoData &data, const std::string &symbol) {
    while (!shouldExit) {
        if (client.fetchPrice(symbol, data)) {
            data.saveToFile(symbol);
        }
        std::this_thread::sleep_for(std::chrono::seconds(60));
    }
}

void dataFetchingThread(CryptoClient &client, CryptoData &data) {
    const std::vector<std::string> symbols = {"BTC", "ETH", "XRP", "SOL", "ADA", "BNB", "SHIB", "DOGE", "PEPE", "USDT"};
    std::vector<std::thread> threads;

    for (const auto &symbol : symbols) {
        std::thread(fetchPriceForSymbol, std::ref(client), std::ref(data), symbol).detach();
    }

    // for (auto &t : threads) {
    //     t.join();  // Ensure all threads complete before function exit
    // }
}

// Render interactive price chart for a selected cryptocurrency
void renderPriceChart(const std::string &symbol, const std::vector<PricePoint> &fullHistory) {
    // Skip rendering if no history available
    if (fullHistory.empty())
        return;

    // Static variables to maintain state between function calls
    static int intervalSelection = 1; // Default to 1-minute intervals
    static std::vector<PricePoint> displayHistory;

    // Interval mapping
    const std::vector<int> intervalMultipliers = {1, 5, 30, 60, 300};
    const std::vector<std::string> intervalNames = {"1 min", "5 min", "30 min", "1 hour", "5 hours"};

    // Interval selection buttons
    ImGui::BeginChild("Interval Selector", ImVec2(0, 50), true);
    for (int i = 0; i < intervalNames.size(); ++i) {
        if (i > 0)
            ImGui::SameLine();
        if (ImGui::Button(intervalNames[i].c_str())) {
            intervalSelection = i;
        }
    }
    ImGui::EndChild();

    // Filter history based on selected interval
    displayHistory.clear();
    int intervalMultiplier = intervalMultipliers[intervalSelection];
    for (size_t i = 0; i < fullHistory.size(); i += intervalMultiplier) {
        displayHistory.push_back(fullHistory[i]);
    }

    // Prepare price data for plotting
    std::vector<float> prices;
    float minPrice = FLT_MAX;
    float maxPrice = -FLT_MAX;

    // Convert prices and find min/max for chart scaling
    for (const auto &point: displayHistory) {
        prices.push_back(static_cast<float>(point.price));
        minPrice = std::min(minPrice, static_cast<float>(point.price));
        maxPrice = std::max(maxPrice, static_cast<float>(point.price));
    }

    // Increase font size for better readability
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
    ImGui::SetWindowFontScale(1.5f);

    // Plot price line chart with large dimensions
    ImGui::PlotLines(("##" + symbol).c_str(),
                     prices.data(),
                     prices.size(),
                     0,
                     nullptr,
                     minPrice,
                     maxPrice,
                     ImVec2(1000, 580)); // Increased chart size

    ImGui::PopFont();
}

int main() {
    // Initialize GLFW for window and OpenGL context
    if (!glfwInit())
        return 1;
    GLFWwindow *window = glfwCreateWindow(1920, 1080, "Crypto Dashboard", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);

    // Initialize ImGui with larger font
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 120");

    // Configure larger base font
    ImFontConfig config;
    config.SizePixels = 20.0f; // Increased base font size
    io.Fonts->AddFontDefault(&config);

    // Initialize cryptocurrency data management and client
    CryptoData data;
    CryptoClient client("606b1972-59c5-45e9-b2be-beb836ee8ae7");

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
                "BTC", "ETH", "XRP", "SOL", "ADA", "BNB", "SHIB", "DOGE", "PEPE", "USDT"};

        // Track selected cryptocurrency for detailed view
        static std::string selectedSymbol;

        // Cryptocurrency price list sidebar
        ImGui::BeginChild("Prices", ImVec2(350, 0), true); // Increased width
        for (const auto &symbol: symbols) {
            // Create selectable list item for each cryptocurrency
            auto priceData = data.getPriceData(symbol);
            if (ImGui::Selectable(symbol.c_str(), selectedSymbol == symbol)) {
                selectedSymbol = symbol;
            }
            ImGui::SameLine(150); // Increased spacing

            // Format large numbers with commas
            std::string priceStr;
            if ((int) priceData.price == 0) {
                priceStr = fmt::format("{:.7f}", priceData.price);
            } else {
                priceStr = fmt::format("{:.2f}", priceData.price);
            }

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

            std::string priceStr;
            std::string startPriceStr;
            if ((int) priceData.price == 0) {
                priceStr = fmt::format("{:.10f}", priceData.price);
                startPriceStr = fmt::format("{:.10f}", data.getStartingPrice(selectedSymbol));
            } else {
                priceStr = fmt::format("{:.2f}", priceData.price);
                startPriceStr = fmt::format("{:.2f}", data.getStartingPrice(selectedSymbol));
            }

            for (auto *str: {&priceStr, &startPriceStr}) {
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
            ImGui::Text("Starting Price: $%s", startPriceStr.c_str());
            if ((int) priceData.price == 0) {
                ImGui::Text("24h Change: %.2f%% ($%.10f)",
                            priceData.percentChange24h,
                            priceData.priceChange24h);
            } else {
                ImGui::Text("24h Change: %.2f%% ($%.2f)",
                            priceData.percentChange24h,
                            priceData.priceChange24h);
            }

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
