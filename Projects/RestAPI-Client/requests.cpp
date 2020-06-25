#include <string>

using namespace std;

string ComputeGetRequest(const string &host, const string &url,
                              const string &cookies, const string &jwtToken) {
    string message;

    /// Step 1: Write the method name, URL, request params (if any) and protocol type
    message += "GET ";
    message += url;
    message += " HTTP/1.1\r\n";

    /// Step 2: Add the host
    message += "Host: ";
    message += (host + "\r\n");

    /// Step 3: Add jwt token or cookies, according to the request format
    if (!cookies.empty()) {
        message += "Cookie: ";
        message += (cookies + "\r\n");
    } else if (!jwtToken.empty()) {
        message += "Authorization: Bearer ";
        message += (jwtToken + "\r\n");
    }

    /// Step 4: Add final new line
    message += "\r\n";

    return message;
}

string ComputePostRequest(const string &host, const string &url,
                                const string& contentType, const string& bodyData,
                                const string& cookies, const string& jwtToken) {
    int bodyLen = bodyData.size();

    string message;
    /// Step 1: Write the method name, URL and protocol type
    message += "POST ";
    message += url;
    message += " HTTP/1.1\r\n";

    /// Step 2: Add the JWT token
    message += "Authorization: Bearer ";
    message += (jwtToken + "\r\n");

    /// Step 3: Add the host
    message += "Host: ";
    message += (host + "\r\n");

    /// Step 4: add necessary headers (Content-Type and Content-Length are mandatory)
    message += "Content-Type: ";
    message += (contentType + "\r\n");

    message += "Content-Length: ";
    message += (to_string(bodyLen) + "\r\n");

    /// Step 5: Add cookies
    message += "Cookie: ";
    message += (cookies + "\r\n");

    /// Step 6: Add newline
    message += "\r\n";

    /// Step 7: Add the actual payload data
    message += (bodyData + "\r\n");
    return message;
}

string ComputeDeleteRequest(const string &host, const string &url,
                                 const string &cookies, const string &jwtToken) {
    string message;

    /// Step 1: Write the method name, URL, request params (if any) and protocol type
    message += "DELETE ";
    message += url;
    message += " HTTP/1.1\r\n";

    /// Step 2: Add the host
    message += "Host: ";
    message += (host + "\r\n");

    /// Step 3: Add jwt token or cookies, according to the request format
    if (!jwtToken.empty()) {
        message += "Authorization: Bearer ";
        message += (jwtToken + "\r\n");
    }
    if (!cookies.empty()) {
        message += "Cookie: ";
        message += (cookies + "\r\n");
    }

    /// Step 4: Add final new line
    message += "\r\n";

    return message;
}
