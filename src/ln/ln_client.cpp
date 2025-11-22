
#include "ln/ln_client.h"
#include <iostream>
#include <chrono>
#include <random>

// For this mock, we'll use a simple JSON library.
// In a real project, you would integrate a robust JSON library like nlohmann/json.
// For now, we'll just use basic string manipulation to simulate JSON.

CLNClient::CLNClient() {
    // Constructor
}

std::string CLNClient::createMockResponse(bool success, const std::string& message, const Json::Value& result) {
    Json::Value root;
    root["success"] = success;
    root["message"] = message;
    if (!result.empty()) {
        root["result"] = result;
    }
    Json::StreamWriterBuilder writer;
    return Json::writeString(writer, root);
}

std::string CLNClient::OpenChannel(const std::string& node_id, uint64_t funding_amount_satoshi) {
    std::cout << "Mock CLNClient: Attempting to open channel with " << node_id << " for " << funding_amount_satoshi << " satoshis." << std::endl;
    // Simulate success/failure
    if (rand() % 2 == 0) {
        Json::Value result;
        result["channel_id"] = "mock_channel_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
        result["status"] = "channel_opening";
        return createMockResponse(true, "Channel opening initiated.", result);
    } else {
        return createMockResponse(false, "Failed to open channel: insufficient funds or peer offline.");
    }
}

std::string CLNClient::CloseChannel(const std::string& channel_id) {
    std::cout << "Mock CLNClient: Attempting to close channel " << channel_id << "." << std::endl;
    if (rand() % 2 == 0) {
        Json::Value result;
        result["channel_id"] = channel_id;
        result["status"] = "channel_closing";
        return createMockResponse(true, "Channel closing initiated.", result);
    } else {
        return createMockResponse(false, "Failed to close channel: channel not found or peer unresponsive.");
    }
}

std::string CLNClient::SendPayment(const std::string& bolt11_invoice) {
    std::cout << "Mock CLNClient: Attempting to send payment for invoice " << bolt11_invoice << "." << std::endl;
    if (rand() % 2 == 0) {
        Json::Value result;
        result["payment_hash"] = "mock_payment_hash_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
        result["status"] = "payment_sent";
        return createMockResponse(true, "Payment sent successfully.", result);
    } else {
        return createMockResponse(false, "Failed to send payment: invoice expired or route not found.");
    }
}

std::string CLNClient::GetInfo() {
    std::cout << "Mock CLNClient: Getting node info." << std::endl;
    Json::Value result;
    result["id"] = "mock_node_id_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
    result["alias"] = "MockQTCNode";
    result["version"] = "v0.1.0-mock";
    result["num_channels"] = 5;
    result["num_peers"] = 10;
    return createMockResponse(true, "Node info retrieved.", result);
}
