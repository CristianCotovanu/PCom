#ifndef RESTAPI_UTILS_H
#define RESTAPI_UTILS_H

#define HTTPALT 8080

#include "json.hpp"

using namespace std;
using json = nlohmann::json;

bool IsNumeric(const string &str);

bool IsValidBookInput(const string &title, const string &author,
                      const string &genre, const string &publisher,
                      const string &pageCount);

string TrimTrailingWhitespace(const string &str);

void PrintBooks(char *replyMessage);

void PrintBookHelper(const string &bookTxt);

void PrintBookInfo(char *replyMessage, const string &id);

#endif
