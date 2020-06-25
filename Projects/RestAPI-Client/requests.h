#ifndef _REQUESTS_
#define _REQUESTS_

// computes and returns a GET request string
std::string ComputeGetRequest(const std::string &host, const std::string &url,
                              const std::string &cookies, const std::string &jwtToken);

// computes and returns a POST request string
std::string ComputePostRequest(const std::string &host, const std::string &url,
                               const std::string &contentType, const std::string &bodyData,
                               const std::string &cookies, const std::string &jwtToken);

// computes and returns a DELETE request string
std::string ComputeDeleteRequest(const std::string &host, const std::string &url,
                                 const std::string &cookies, const std::string &jwtToken);

#endif
