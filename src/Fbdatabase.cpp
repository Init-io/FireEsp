#include "FbDatabase.h"
#include <WiFiClientSecure.h>

// Constructor
FbDatabase::FbDatabase(FbServer& serverRef) : server(serverRef) {}

// Put (String value)
bool FbDatabase::put(String path, String key, String value, String idToken) {
    String url = server.getDatabaseURL() + (path.startsWith("/") ? path : "/" + path) + ".json" +
                 (idToken.length() > 0 ? "?auth=" + idToken : "");
    String payload = "{\"" + key + "\":\"" + value + "\"}";
    String response = httpRequest("PATCH", url, payload);

    Serial.println("PUT Response (String): " + response);
    return response.indexOf("error") == -1;
}

// Put (int value)
bool FbDatabase::put(String path, String key, int value, String idToken) {
    String url = server.getDatabaseURL() + (path.startsWith("/") ? path : "/" + path) + ".json" +
                 (idToken.length() > 0 ? "?auth=" + idToken : "");
    String payload = "{\"" + key + "\":" + String(value) + "}";
    String response = httpRequest("PATCH", url, payload);

    Serial.println("PUT Response (int): " + response);
    return response.indexOf("error") == -1;
}

// Put JSON directly to the database
bool FbDatabase::putJson(String path, String jsonPayload, String idToken) {
    String url = server.getDatabaseURL() + (path.startsWith("/") ? path : "/" + path) + ".json" +
                 (idToken.length() > 0 ? "?auth=" + idToken : "");
    String response = httpRequest("PATCH", url, jsonPayload);

    Serial.println("PUT JSON Response: " + response);
    return response.indexOf("error") == -1;
}


// Update (String value) - Alias for put
bool FbDatabase::update(String path, String key, String value, String idToken) {
    return put(path, key, value, idToken);
}

// Update (int value) - Alias for put
bool FbDatabase::update(String path, String key, int value, String idToken) {
    return put(path, key, value, idToken);
}

String FbDatabase::get(String path, String idToken) {
    String url = server.getDatabaseURL() + (path.startsWith("/") ? path : "/" + path) + ".json" +
                 (idToken.length() > 0 ? "?auth=" + idToken : "");
    String payload = httpRequest("GET", url, "");

    Serial.println("GET Response: " + payload);

    // Check if the response contains an error
    if (payload.indexOf("error") != -1) {
        return payload; // Return the error message as is
    }

    payload.trim(); // Ensure no leading/trailing whitespace
    return payload; // Return the cleaned payload
}

// Get raw JSON from the database
String FbDatabase::getJson(String path, String idToken) {
    String url = server.getDatabaseURL() + (path.startsWith("/") ? path : "/" + path) + ".json" +
                 (idToken.length() > 0 ? "?auth=" + idToken : "");
    String response = httpRequest("GET", url, "");

    Serial.println("GET JSON Response: " + response);

    if (response.indexOf("error") != -1) {
        return response; // Return error message as is
    }

    response.trim(); // Clean extra spaces
    return response; // Return raw JSON
}

// Remove data from the database
bool FbDatabase::remove(String path, String idToken) {
    String url = server.getDatabaseURL() + (path.startsWith("/") ? path : "/" + path) + ".json" +
                 (idToken.length() > 0 ? "?auth=" + idToken : "");
    String response = httpRequest("DELETE", url, "");

    Serial.println("DELETE Response: " + response);
    return response.indexOf("error") == -1;
}

// Helper method to make HTTP requests
String FbDatabase::httpRequest(String method, String url, String payload) {
    WiFiClientSecure client;
    client.setInsecure(); // Disable certificate validation (not recommended for production)

    // Extract the host and path from the URL
    String host = url.substring(8, url.indexOf('/', 8)); // Extract host after "https://"
    String path = url.substring(url.indexOf('/', 8));    // Extract path

    if (!client.connect(host.c_str(), 443)) {
        return "{\"error\": \"Connection failed to " + host + "\"}";
    }

    // Build the HTTP request
    String request = method + " " + path + " HTTP/1.1\r\n";
    request += "Host: " + host + "\r\n";
    request += "Content-Type: application/json\r\n";
    if (payload.length() > 0) {
        request += "Content-Length: " + String(payload.length()) + "\r\n";
    }
    request += "Connection: close\r\n\r\n";
    if (payload.length() > 0) {
        request += payload;
    }

    // Send the request
    client.print(request);

    // Read and parse the response
    String payloadStartDelimiter = "\r\n\r\n"; // Delimiter to find payload start
    String response = "";
    bool isHeader = true; // Track whether we're reading the headers
    int payloadStartIndex = -1;

    while (client.connected()) {
        while (client.available()) {
            char c = client.read();
            response += c;

            // Find the end of the headers
            if (isHeader && response.endsWith(payloadStartDelimiter)) {
                isHeader = false; // We've reached the payload
                payloadStartIndex = response.length();
            }
        }
    }

    client.stop();

    // Return only the payload if headers were found
    if (payloadStartIndex != -1) {
        return response.substring(payloadStartIndex); // Extract payload
    }

    return "{\"error\": \"Malformed response or no payload found\"}";
}
