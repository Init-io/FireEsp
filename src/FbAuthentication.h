#ifndef FBAUTHENTICATION_H
#define FBAUTHENTICATION_H

#include "FbServer.h"
#include <Arduino.h>

class FbAuthentication {
private:
    FbServer& server;  // Reference to FbServer
    String idToken;
    String localId;
    String refreshToken;

    // Helper method for HTTP requests
    String httpRequest(String method, String url, String payload);

    // Helper method to extract token values from response
    String extractTokenFromResponse(String response, String tokenName);

    // Helper method to extract error message from response
    String getErrorMessage(String response);

public:
    // Constructor
    FbAuthentication(FbServer& serverRef);

    // Authentication methods
    bool signUp(String email, String password);
    bool signIn(String email, String password);
    bool resetPassword(String email);
    bool verifyEmail(String idToken);
    bool checkEmailVerified(String idToken);
    bool deleteUser(String idToken);
    bool refreshIdToken(String refreshToken);

    // Getters
    String getIdToken();
    String getUserId();
    String getRefreshToken();
};

#endif
