// main.cpp
#include <thread>
#include <chrono>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include "CryptoClient.hpp"
#include "CryptoData.hpp"

std::atomic<bool> shouldExit{false};

void dataFetchingThread(CryptoClient& client, CryptoData& data) {
    const std::vector<std::string> symbols = {"BTC", "ETH", "XRP"};

    while (!shouldExit) {
        for (const auto& symbol : symbols) {
            if (client.fetchPrice(symbol, data)) {
                data.saveToFile(symbol);
            }
        }
        std::this_thread::sleep_for(std::chrono::seconds(30));
    }
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

        ImGui::Begin("Cryptocurrency Dashboard");

        // Display current prices
        const std::vector<std::string> symbols = {"BTC", "ETH", "XRP"};
        for (const auto& symbol : symbols) {
            double price = data.getPrice(symbol);
            ImGui::Text("%s: $%.2f", symbol.c_str(), price);

            if (ImGui::Button(("Show History##" + symbol).c_str())) {
                // Display historical data in a popup or new window
                auto history = data.getHistoricalData(symbol);
                // Plot historical data using ImGui::PlotLines
            }
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