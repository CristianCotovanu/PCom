#include <iostream>
#include "utils.h"

bool IsNumeric(const string &str) {
    return (str.find_first_not_of("0123456789") == string::npos);
}

string TrimTrailingWhitespace(const string &str) {
    size_t end = str.find_last_not_of(" \n\r\t\f\v");
    return (end == string::npos) ? "" : str.substr(0, end + 1);
}

bool IsValidBookInput(const string &title, const string &author,
                      const string &genre, const string &publisher,
                      const string &pageCount) {
    return IsNumeric(pageCount) && !title.empty() && !author.empty()
           && !publisher.empty() && !genre.empty() && !IsNumeric(title)
           && !IsNumeric(author) && !IsNumeric(genre) && !IsNumeric(publisher);
}

void PrintBookHelper(const string &bookTxt) {
    json tmpBook = json::parse(bookTxt);

    cout << "[BookId:" << tmpBook["id"] << "] Title: " << tmpBook["title"] << endl;
}

void PrintBooks(char *replyMessage) {
    string reply(replyMessage);

    size_t first = reply.find('{');
    size_t last = reply.find('}');

    if (first == string::npos || last == string::npos) {
        cout << "No books in the library." << endl;
        return;
    } else {
        cout << "[Library]" << endl;
    }

    while (first != string::npos && last != string::npos) {
        string bookTxt = reply.substr(first, last - first + 1);

        PrintBookHelper(bookTxt);

        reply = reply.substr(last + 1);
        first = reply.find('{');
        last = reply.find('}');
    }
}

void PrintBookInfo(char *replyMessage, const string &id) {
    string reply(replyMessage);
    int foundIdx = reply.find('[');
    string found = string(reply.begin() + foundIdx + 1, reply.begin() + reply.size() - 1);

    json bookJson = json::parse(found);
    cout << "[BookId:" << id << "]" << endl;
    cout << "Title: " << bookJson["title"] << endl;
    cout << "Author: " << bookJson["author"] << endl;
    cout << "Publisher: " << bookJson["publisher"] << endl;
    cout << "Genre: " << bookJson["genre"] << endl;
    cout << "Page count: " << bookJson["page_count"] << endl;
}