#include "FbAuthentication.h"
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>

// Constructor
FbAuthentication::FbAuthentication(FbServer& serverRef) : server(serverRef) {}

// Helper method to perform HTTP requests
String FbAuthentication::httpRequest(String method, String url, String payload) {
    WiFiClientSecure client;
    client.setInsecure();

    String host = "identitytoolkit.googleapis.com";

    if (!client.connect(host.c_str(), 443)) {
        return "{\"error\": \"Connection failed\"}";
    }

    String request = method + " " + url.substring(url.indexOf("/", 8)) + " HTTP/1.1\r\n";
    request += "Host: " + host + "\r\n";
    request += "Content-Type: application/json\r\n";
    request += "Content-Length: " + String(payload.length()) + "\r\n";
    request += "Connection: close\r\n\r\n";
    request += payload;

    client.print(request);

    while (client.connected() && !client.available()) {
        delay(10);
    }

    while (client.available()) {
        String line = client.readStringUntil('\n');
        if (line == "\r") {
            break;
        }
    }

    String responseBody = "";
    while (client.available()) {
        String chunkSizeHex = client.readStringUntil('\n');
        int chunkSize = strtol(chunkSizeHex.c_str(), NULL, 16);
        if (chunkSize == 0) {
            break;
        }

        char* chunk = new char[chunkSize + 1];
        client.readBytes(chunk, chunkSize);
        chunk[chunkSize] = '\0';
        responseBody += chunk;
        delete[] chunk;

        client.readStringUntil('\n');
    }

    return responseBody;
}

// Helper to extract token from response
String FbAuthentication::extractTokenFromResponse(String response, String tokenName) {
    StaticJsonDocument<1024> doc;
    DeserializationError error = deserializeJson(doc, response);
    if (error) {
        return "";
    }
    if (doc.containsKey(tokenName)) {
        return doc[tokenName].as<String>();
    }
    return "";
}

// Helper to extract detailed error message
String FbAuthentication::getErrorMessage(String response) {
    int errorPos = response.indexOf("\"error\"");
    if (errorPos != -1) {
        int messagePos = response.indexOf("\"message\":", errorPos);
        if (messagePos != -1) {
            String errorMessage = response.substring(messagePos + 11);
            errorMessage = errorMessage.substring(0, errorMessage.indexOf("\""));
            return errorMessage;
        }
    }
    return "Unknown error";
}

// Sign up method
bool FbAuthentication::signUp(String email, String password) {
    String url = "https://identitytoolkit.googleapis.com/v1/accounts:signUp?key=" + server.getApiKey();
    String payload = "{\"email\":\"" + email + "\",\"password\":\"" + password + "\",\"returnSecureToken\":true}";
    String response = httpRequest("POST", url, payload);

    StaticJsonDocument<1024> jsonDoc;
    DeserializationError error = deserializeJson(jsonDoc, response);

    if (error) {
        Serial.println("Error parsing signUp response: " + String(error.c_str()));
        return false;
    }

    if (jsonDoc.containsKey("idToken") && jsonDoc.containsKey("localId") && jsonDoc.containsKey("refreshToken")) {
        this->idToken = jsonDoc["idToken"].as<String>();
        this->localId = jsonDoc["localId"].as<String>();
        this->refreshToken = jsonDoc["refreshToken"].as<String>();
        return true;
    } else {
        Serial.println("Error: Missing idToken, localId or refreshToken");
        return false;
    }
}

// Sign in method
bool FbAuthentication::signIn(String email, String password) {
    String url = "https://identitytoolkit.googleapis.com/v1/accounts:signInWithPassword?key=" + server.getApiKey();
    String payload = "{\"email\":\"" + email + "\",\"password\":\"" + password + "\",\"returnSecureToken\":true}";
    String response = httpRequest("POST", url, payload);

    Serial.println("Raw Response: " + response);

    StaticJsonDocument<1024> jsonDoc;
    DeserializationError error = deserializeJson(jsonDoc, response);

    if (error) {
        Serial.println("Error parsing signIn response: " + String(error.c_str()));
        return false;
    }

    if (jsonDoc.containsKey("idToken") && jsonDoc.containsKey("localId") && jsonDoc.containsKey("refreshToken")) {
        this->idToken = jsonDoc["idToken"].as<String>();
        this->localId = jsonDoc["localId"].as<String>();
        this->refreshToken = jsonDoc["refreshToken"].as<String>();

        Serial.println("idToken: " + idToken);
        Serial.println("localId: " + localId);
        Serial.println("refreshToken: " + refreshToken);

        return true;
    } else {
        Serial.println("Error: Missing idToken, localId or refreshToken in response");
        if (jsonDoc.containsKey("error")) {
            Serial.println("Error message: " + String(jsonDoc["error"]["message"].as<String>()));
        }
        return false;
    }
}

// Refresh idToken
bool FbAuthentication::refreshIdToken(String refreshToken) {
    String url = "https://securetoken.googleapis.com/v1/token?key=" + server.getApiKey();
    String payload = "grant_type=refresh_token&refresh_token=" + refreshToken;

    WiFiClientSecure client;
    client.setInsecure();

    String host = "securetoken.googleapis.com";
    if (!client.connect(host.c_str(), 443)) {
        Serial.println("Connection to refresh token server failed");
        return false;
    }

    String request = "POST /v1/token?key=" + server.getApiKey() + " HTTP/1.1\r\n";
    request += "Host: " + host + "\r\n";
    request += "Content-Type: application/x-www-form-urlencoded\r\n";
    request += "Content-Length: " + String(payload.length()) + "\r\n";
    request += "Connection: close\r\n\r\n";
    request += payload;

    client.print(request);

    while (client.connected() && !client.available()) {
        delay(10);
    }

    while (client.available()) {
        String line = client.readStringUntil('\n');
        if (line == "\r") {
            break;
        }
    }

    String responseBody = "";
    while (client.available()) {
        responseBody += client.readStringUntil('\n');
    }

    client.stop();

    Serial.println("Refresh Token Response: " + responseBody);

    StaticJsonDocument<1024> doc;
    DeserializationError error = deserializeJson(doc, responseBody);

    if (error) {
        Serial.println("Error parsing refresh token response: " + String(error.c_str()));
        return false;
    }

    if (doc.containsKey("id_token") && doc.containsKey("user_id")) {
        this->idToken = doc["id_token"].as<String>();
        this->localId = doc["user_id"].as<String>();

        Serial.println("Token refreshed successfully!");
        return true;
    } else {
        Serial.println("Error: Failed to refresh idToken");
        return false;
    }
}

// Getters
String FbAuthentication::getIdToken() {
    return idToken;
}

String FbAuthentication::getUserId() {
    return localId;
}

String FbAuthentication::getRefreshToken() {
    return refreshToken;
}

// Reset password
bool FbAuthentication::resetPassword(String email) {
    String url = "https://identitytoolkit.googleapis.com/v1/accounts:sendOobCode?key=" + server.getApiKey();
    String payload = "{\"requestType\":\"PASSWORD_RESET\",\"email\":\"" + email + "\"}";
    String response = httpRequest("POST", url, payload);

    if (response.indexOf("email") != -1) {
        return true;
    }

    String errorMessage = getErrorMessage(response);
    if (errorMessage != "Unknown error") {
        Serial.println("Password reset error: " + errorMessage);
    }

    return false;
}

// Verify email
bool FbAuthentication::verifyEmail(String idToken) {
    String url = "https://identitytoolkit.googleapis.com/v1/accounts:sendOobCode?key=" + server.getApiKey();
    String payload = "{\"requestType\":\"VERIFY_EMAIL\",\"idToken\":\"" + idToken + "\"}";
    String response = httpRequest("POST", url, payload);

    if (response.indexOf("email") != -1) {
        return true;
    }

    String errorMessage = getErrorMessage(response);
    if (errorMessage != "Unknown error") {
        Serial.println("Email verification error: " + errorMessage);
    }

    return false;
}

// Check email verification
bool FbAuthentication::checkEmailVerified(String idToken) {
    String url = "https://identitytoolkit.googleapis.com/v1/accounts:lookup?key=" + server.getApiKey();
    String payload = "{\"idToken\":\"" + idToken + "\"}";
    String response = httpRequest("POST", url, payload);

    if (response.indexOf("\"emailVerified\":true") != -1) {
        return true;
    }

    return false;
}

// Delete user
bool FbAuthentication::deleteUser(String idToken) {
    String url = "https://identitytoolkit.googleapis.com/v1/accounts:delete?key=" + server.getApiKey();
    String payload = "{\"idToken\":\"" + idToken + "\"}";
    String response = httpRequest("POST", url, payload);

    if (response.indexOf("idToken") != -1) {
        return true;
    }

    return false;
}
