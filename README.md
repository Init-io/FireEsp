---

# FireEsp - A C++ Firebase Library for Arduino

[![Arduino Library](https://img.shields.io/badge/Arduino-Library-blue.svg)](https://docs.arduino.cc/libraries/fireesp/?_gl=1*tsbv28*_up*MQ..*_ga*OTc3MDE5NDYxLjE3Mzg2OTIzNDg.*_ga_NEXN8H46L5*MTczODY5MjM0Ni4xLjEuMTczODY5MjQxMi4wLjAuNDU4ODMyODky)
![GitHub Release](https://img.shields.io/github/v/release/init-io/FireEsp?label=release)
[![License](https://img.shields.io/github/license/init-io/FireEsp)](LICENSE)

FireEsp is a lightweight C++ library that simplifies the integration of Firebase services into your Arduino projects. It provides classes to interact with Firebase Authentication, Realtime Database, and Server Configuration. This library is designed for ease of use and aims to make Firebase integration seamless for IoT and embedded projects using Arduino-compatible boards.

---

## Features

* **Firebase Authentication**: Sign up, sign in, reset passwords, verify email addresses, delete users, and refresh ID tokens.
* **Firebase Realtime Database**: Perform basic CRUD operations (create, read, update, delete) on Firebase's Realtime Database.
* **Server Configuration**: Set and manage Firebase project details such as API key, auth domain, and database URL.
* **HTTP Requests**: Send HTTP requests to Firebase's REST API to perform authentication and database operations.
* **Debugging Support**: Easily switch between production and debugging modes with a configurable debug level.
* **Token Management**: Full support for refresh tokens and ID token renewal.

---

## Requirements

* **Arduino IDE**: To compile and upload code to your Arduino or compatible board.
* **Firebase Project**: You need a Firebase project with Authentication and Realtime Database enabled.
* **Arduino-Compatible Board**: Supports boards like ESP8266, ESP32, etc.

---

## Installation

```bash
git clone https://github.com/Init-io/FireEsp.git
```

1. Open the Arduino IDE.
2. Go to **Sketch > Include Library > Add .ZIP Library...**.
3. Select the `FireESP` folder.

---

## Usage

### Firebase Configuration

```cpp
FbServer server("YOUR_API_KEY", "YOUR_AUTH_DOMAIN", "YOUR_DATABASE_URL");
server.initialize();
```

---

## Firebase Authentication

### Sign Up

```cpp
FbAuthentication auth(server);
bool success = auth.signUp("user@example.com", "password123");
```

### Sign In

```cpp
bool success = auth.signIn("user@example.com", "password123");
```

### Refresh ID Token

```cpp
bool success = auth.refreshIdToken(auth.getRefreshToken());
```

### Get Current Tokens

```cpp
String idToken = auth.getIdToken();
String refreshToken = auth.getRefreshToken();
```

### Reset Password

```cpp
bool success = auth.resetPassword("user@example.com");
```

### Verify Email

```cpp
bool success = auth.verifyEmail(auth.getIdToken());
```

### Check Email Verification

```cpp
bool verified = auth.checkEmailVerified(auth.getIdToken());
```

### Delete User

```cpp
bool success = auth.deleteUser(auth.getIdToken());
```

---

## Firebase Database Operations

### Put Data (String)

```cpp
FbDatabase database(server);
bool success = database.put("/path/to/data", "key", "value", auth.getIdToken());
```

### Put Data (Integer)

```cpp
bool success = database.put("/path/to/data", "key", 123, auth.getIdToken());
```

### Put Raw JSON

```cpp
bool success = database.putJson("/path/to/data", "{\"name\":\"John\",\"age\":25}", auth.getIdToken());
```

### Update Data (String)

```cpp
bool success = database.update("/path/to/data", "key", "new_value", auth.getIdToken());
```

### Get Data (String)

```cpp
String value = database.get("/path/to/data", auth.getIdToken());
```

### Get Raw JSON

```cpp
String json = database.getJson("/path/to/data", auth.getIdToken());
```

### Remove Data

```cpp
bool success = database.remove("/path/to/data", auth.getIdToken());
```

---

## Debugging Support

Enable or disable debugging globally.

```cpp
#define DEBUG 1  // 1 = Debug Mode, 0 = Production Mode
#include <FireEsp.h>
```

* **Debug Mode:** Prints HTTP requests, responses, and errors.
* **Production Mode:** No sensitive data is printed.

---

## Example Project

```cpp
#define DEBUG 1
#include <FireEsp.h>

FbServer server("YOUR_API_KEY", "YOUR_AUTH_DOMAIN", "YOUR_DATABASE_URL");
FbAuthentication auth(server);
FbDatabase database(server);

void setup() {
  Serial.begin(115200);
  server.initialize();

  if (auth.signIn("user@example.com", "password123")) {
    Serial.println("User signed in!");

    // Save Data
    database.put("/users/user1", "name", "John Doe", auth.getIdToken());

    // Refresh Token Example
    if (auth.refreshIdToken(auth.getRefreshToken())) {
      Serial.println("Token refreshed successfully!");
    }
  }
}

void loop() {
  // Your code here
}
```

---

## Known Issues

* **WiFi Instability**: Unstable networks may cause requests to fail intermittently.
* **Firebase Delays**: Occasionally, Firebase may delay response, especially during refresh or sign-in.

---

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

---

## Contributing

We welcome contributions! Please:

* Fork the repo
* Create a feature branch
* Submit a pull request

**Follow coding style and document your changes properly.**

---

## Contact

* GitHub: [FireEsp Repository](https://github.com/Init-io/FireEsp)
* Email: [developers.init.io@gmail.com](mailto:developers.init.io@gmail.com)

---
