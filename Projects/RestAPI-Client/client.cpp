#include <cstdlib>
#include <netdb.h>
#include <arpa/inet.h>
#include <iostream>
#include "utils.h"
#include "requests.h"
#include "connection.h"

int main(int argc, char *argv[]) {
    int port = HTTPALT;
    int sockfd;
    string cookies;
    string jwtToken;
    string contentType = "application/json";
    string host = "ec2-3-8-116-10.eu-west-2.compute.amazonaws.com";
    char *addr = inet_ntoa((in_addr) *((in_addr *) gethostbyname(host.c_str())->h_addr_list[0]));

    while (true) {
        string command;
        getline(cin, command);
        command = TrimTrailingWhitespace(command);

        if (command == "register") {
            json registerMsg;
            string username, password;
            string url = "/api/v1/tema/auth/register";

            cout << "username=";
            getline(cin, username);
            cout << "password=";
            getline(cin, password);
            username = TrimTrailingWhitespace(username);
            password = TrimTrailingWhitespace(password);
            registerMsg["username"] = username;
            registerMsg["password"] = password;

            string request = ComputePostRequest(host, url, contentType,
                                                registerMsg.dump(), "", "");
            sockfd = OpenConnection(addr, port, AF_INET, SOCK_STREAM, 0);
            SendToServer(sockfd, (char *) request.c_str());
            char *reply = ReceiveFromServer(sockfd);

            if (strstr(reply, "Error")) {
                cout << "Username \"" << username << "\" is already taken." << endl;
            } else {
                cout << "Registered account \"" << username << "\"." << endl;
            }

            CloseConnection(sockfd);
            free(reply);
        } else if (command == "login") {
            json loginMsg;
            string username, password;
            string url = "/api/v1/tema/auth/login";

            cout << "username=";
            getline(cin, username);
            cout << "password=";
            getline(cin, password);
            username = TrimTrailingWhitespace(username);
            password = TrimTrailingWhitespace(password);
            loginMsg["username"] = username;
            loginMsg["password"] = password;

            string request = ComputePostRequest(host, url, contentType,
                                                loginMsg.dump(), "", "");
            sockfd = OpenConnection(addr, port, AF_INET, SOCK_STREAM, 0);
            SendToServer(sockfd, (char *) request.c_str());
            char *reply = ReceiveFromServer(sockfd);

            if (char *cookie = strstr(reply, "connect")) {
                char *token = strtok(cookie, ";");
                cookies = token;
                cout << "Logged in." << endl;
            } else if (strstr(reply, "No account")) {
                cout << "No account using \"" << username << "\" as username." << endl;
            } else if (strstr(reply, "Credentials are")) {
                cout << "You have entered an invalid username or password." << endl;
            }
            CloseConnection(sockfd);
            free(reply);
        } else if (command == "enter_library") {
            string url = "/api/v1/tema/library/access";
            string request = ComputeGetRequest(host, url, cookies, "");
            sockfd = OpenConnection(addr, port, AF_INET, SOCK_STREAM, 0);
            SendToServer(sockfd, (char *) request.c_str());
            char *reply = ReceiveFromServer(sockfd);

            if (char *jwtTokenTemp = strstr(reply, "token")) {
                char *token = strtok(jwtTokenTemp + strlen("token\":\""), "\"");
                jwtToken = token;
                cout << "Access to library granted." << endl;
            } else {
                cout << "Cannot grant access to library." << endl;
            }
            CloseConnection(sockfd);
            free(reply);
        } else if (command == "get_books") {
            string url = "/api/v1/tema/library/books";

            string request = ComputeGetRequest(host, url, "", jwtToken);
            sockfd = OpenConnection(addr, port, AF_INET, SOCK_STREAM, 0);
            SendToServer(sockfd, (char *) request.c_str());
            char *reply = ReceiveFromServer(sockfd);

            if (strstr(reply, "Error")) {
                cout << "You do not have access to the library." << endl;
            } else {
                PrintBooks(reply);
            }
            CloseConnection(sockfd);
            free(reply);
        } else if (command == "get_book") {
            string id;
            cout << "id=";
            getline(cin, id);
            id = TrimTrailingWhitespace(id);

            if (IsNumeric(id)) {
                string url = "/api/v1/tema/library/books/";
                url += id;

                string request = ComputeGetRequest(host, url, "", jwtToken);
                sockfd = OpenConnection(addr, port, AF_INET, SOCK_STREAM, 0);
                SendToServer(sockfd, (char *) request.c_str());
                char *reply = ReceiveFromServer(sockfd);

                if (strstr(reply, "No book")) {
                    cout << "No book with id \"" << id << "\" was found." << endl;
                } else if (strstr(reply, "Error")) {
                    cout << "You do not have access to the library." << endl;
                } else {
                    PrintBookInfo(reply, id);
                }

                CloseConnection(sockfd);
                free(reply);
            } else {
                cout << "Book Id should be a numeric input." << endl;
            }
        } else if (command == "add_book") {
            json addBookMsg;
            string title, author, genre, publisher, pageCount;
            string url = "/api/v1/tema/library/books";

            cout << "title=";
            getline(cin, title);
            cout << "author=";
            getline(cin, author);
            cout << "genre=";
            getline(cin, genre);
            cout << "publisher=";
            getline(cin, publisher);
            cout << "pageCount=";
            getline(cin, pageCount);
            title = TrimTrailingWhitespace(title);
            author = TrimTrailingWhitespace(author);
            genre = TrimTrailingWhitespace(genre);
            publisher = TrimTrailingWhitespace(publisher);
            pageCount = TrimTrailingWhitespace(pageCount);

            if (IsValidBookInput(title, author, genre, publisher, pageCount)) {
                addBookMsg["title"] = title;
                addBookMsg["author"] = author;
                addBookMsg["genre"] = genre;
                addBookMsg["pageCount"] = stoi(pageCount);
                addBookMsg["publisher"] = publisher;

                string request = ComputePostRequest(host, url, contentType,
                                                    addBookMsg.dump(), cookies, jwtToken);
                sockfd = OpenConnection(addr, port, AF_INET, SOCK_STREAM, 0);
                SendToServer(sockfd, (char *) request.c_str());
                char *reply = ReceiveFromServer(sockfd);

                if (strstr(reply, "Error")) {
                    cout << "You do not have access to the library." << endl;
                } else if (strstr(reply, "Too many")) {
                    cout << "Could not add book." << endl;
                } else {
                    cout << "Succesfully added book." << endl;
                }

                CloseConnection(sockfd);
                free(reply);
            } else {
                cout << "Wrong input data for a new book." << endl;
            }
        } else if (command == "delete_book") {
            string id;
            cout << "id=";
            getline(cin, id);
            id = TrimTrailingWhitespace(id);

            if (IsNumeric(id)) {
                string url = "/api/v1/tema/library/books/";
                url += id;

                string request = ComputeDeleteRequest(host, url, cookies, jwtToken);
                sockfd = OpenConnection(addr, port, AF_INET, SOCK_STREAM, 0);
                SendToServer(sockfd, (char *) request.c_str());
                char *reply = ReceiveFromServer(sockfd);

                if (strstr(reply, "No book")) {
                    cout << "No book with id \"" << id << "\" was found deletable." << endl;
                } else if (strstr(reply, "Error")) {
                    cout << "You do not have access to the library." << endl;
                } else {
                    cout << "Succesfully deleted book with id:" << id << "." << endl;
                }

                CloseConnection(sockfd);
                free(reply);
            } else {
                cout << "Book Id should be a numeric input." << endl;
            }
        } else if (command == "logout") {
            string url = "/api/v1/tema/auth/logout";
            string request = ComputeGetRequest(host, url, cookies, "");
            sockfd = OpenConnection(addr, port, AF_INET, SOCK_STREAM, 0);
            SendToServer(sockfd, (char *) request.c_str());
            char *reply = ReceiveFromServer(sockfd);

            if (strstr(reply, "Error")) {
                cout << "No account is logged in." << endl;
            } else {
                cout << "Logged out." << endl;
            }

            CloseConnection(sockfd);
            free(reply);
            cookies.erase();
            jwtToken.erase();
        } else if (command == "exit") {
            break;
        } else {
            cout << "Invalid command." << endl;
        }
    }

    return 0;
}
