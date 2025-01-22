// main.cpp
#include <thread>
#include <chrono>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include "CryptoClient.h"
#include "CryptoData.h"

std::atomic<bool> shouldExit{false};

void dataFetchingThread(CryptoClient& client, CryptoData& data) {
    const std::vector<std::string> symbols = {
        "BTC", "ETH", "XRP", "SOL", "ADA", "BNB", "SHIB", "DOGE", "PEPE", "USDT"
    };

    while (!shouldExit) {
        for (const auto& symbol : symbols) {
            if (client.fetchPrice(symbol, data)) {
                data.saveToFile(symbol);
            }
        }
        std::this_thread::sleep_for(std::chrono::seconds(60));
    }
}

void renderPriceChart(const std::string& symbol, const std::vector<PricePoint>& history) {
    if (history.empty()) return;

    std::vector<float> prices;
    for (const auto& point : history) {
        prices.push_back(static_cast<float>(point.price));
    }

    ImGui::PlotLines(("##" + symbol).c_str(),
                     prices.data(),
                     prices.size(),
                     0,
                     nullptr,
                     FLT_MAX,
                     FLT_MAX,
                     ImVec2(500, 300));
}

int main() {
    // Initialize GLFW and ImGui
    if (!glfwInit()) return 1;
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Crypto Dashboard", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    // Initialize crypto data and client
    CryptoData data;
    CryptoClient client("bdbc3a83-8c73-4f95-8564-4fb4b51a17d1");

    // Start data fetching thread
    std::thread fetchThread(dataFetchingThread, std::ref(client), std::ref(data));

    // Main UI loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Cryptocurrency Dashboard", nullptr,
                    ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

        const std::vector<std::string> symbols = {
            "BTC", "ETH", "XRP", "SOL", "ADA", "BNB", "SHIB", "DOGE", "PEPE", "USDT"
        };

        static std::string selectedSymbol;
        ImGui::BeginChild("Prices", ImVec2(250, 0), true);
        for (const auto& symbol : symbols) {
            auto priceData = data.getPriceData(symbol);
            if (ImGui::Selectable(symbol.c_str(), selectedSymbol == symbol)) {
                selectedSymbol = symbol;
            }
            ImGui::SameLine(100);
            ImGui::Text("$%.2f", priceData.price);
        }
        ImGui::EndChild();

        ImGui::SameLine();

        if (!selectedSymbol.empty()) {
            ImGui::BeginChild("Chart", ImVec2(0, 0), true);
            auto priceData = data.getPriceData(selectedSymbol);

            ImGui::Text("%s Price Chart", selectedSymbol.c_str());
            ImGui::Text("Current Price: $%.2f", priceData.price);
            ImGui::Text("24h Change: %.2f%% ($%.2f)",
                       priceData.percentChange24h,
                       priceData.priceChange24h);

            auto history = data.getHistoricalData(selectedSymbol);
            renderPriceChart(selectedSymbol, history);
            ImGui::EndChild();
        }

        ImGui::End();

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

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}