
#ifndef QTC_LN_LN_CLIENT_H
#define QTC_LN_LN_CLIENT_H

#include <string>
#include <vector>
#include <json/json.h> // Assuming a JSON library like nlohmann/json is available

// Mock CLNClient class for simulating Lightning Network interactions
class CLNClient {
public:
    CLNClient();

    // Simulate opening a channel
    // Returns a JSON string representing the response
    std::string OpenChannel(const std::string& node_id, uint64_t funding_amount_satoshi);

    // Simulate closing a channel
    // Returns a JSON string representing the response
    std::string CloseChannel(const std::string& channel_id);

    // Simulate sending a payment
    // Returns a JSON string representing the response
    std::string SendPayment(const std::string& bolt11_invoice);

    // Simulate getting node info
    // Returns a JSON string representing the response
    std::string GetInfo();

private:
    // Helper to create mock JSON responses
    std::string createMockResponse(bool success, const std::string& message, const Json::Value& result = Json::Value());
};

#endif // QTC_LN_LN_CLIENT_H
