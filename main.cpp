#include <iostream>
#include "httplib.h"  // Include the httplib header
#include <string>

int main() {
    // Replace YOUR_API_KEY with your actual CoinAPI key
    const std::string api_key = "bdbc3a83-8c73-4f95-8564-4fb4b51a17d1";

    // Base URL for CoinAPI (use HTTP if not using SSL)
    std::string url = "/v1/exchangerate/BTC/USD";

    // Create a client object for HTTP (without SSL)
    httplib::Client cli("http://rest.coinapi.io");

    // Set the Authorization header to include your API key
    httplib::Headers headers = { { "X-CoinAPI-Key", api_key } };

    // Perform the GET request
    auto res = cli.Get(url.c_str(), headers);

    if (res) {
        // If the request was successful, print the response status code and body
        std::cout << "Status Code: " << res->status << std::endl;
        std::cout << "Response Body: " << res->body << std::endl;
    } else {
        // If there was an error, print the error code
        std::cout << "Error code: " << res.error() << std::endl;
    }

    return 0;
}