REST API
===========
Description
----------
HTTP Client for a REST API which can communicate with the server of a library.

Usage
-----
Compile the program using the "Makefile".  
Run in the command line with "./client".  
The application offers functionality through various commands.

**register**  
Register account. It offers prompt.  
username=  
password=  

**login**  
Login on the server. It offers prompt.  
username=  
password=  

**enter_library**  
Requests access to the library through a jwt token.

**get_books**  
Get information for all the books in the library.  

**get_book**  
Get information for a book by it's ID. It offers prompt.  
id=

**add_book**  
Delete a book by it's ID. It offers prompt.  
title=  
author=  
genre=  
publisher=  
page_count=  

**delete_book**  
Delete a book by it's ID. It offers prompt.  
id=  

**logout**  
Logout from the current session.

**exit**  
Close application.

Implementation
--------------
Client reads input command from stdin, checks validity, offers prompt   
for specific command input data depending on command type, and then  
computes and sends specific request.  
Connection to the server is being opened and closed for each request.  
Cookies and the JWT Token are contained in variables which update when  
there is a succesful request for either of them. On logout both cookies and  
the token reset.  
Whenever a request is succesful or not I show a suggestive message according   
to the specific request to the console.  
Input data like username, password, id, etc from the console is being  
trimmed of its trailling whitespaces through a helper function.  
Input data validity is checked for various criterias: ids can only be numeric,  
title, author, publisher, genre can not contain numbers.  
JSON parsing was done using nlohmann for cpp.  
For books information output, I parsed the reply message and with json parser  
functionality extracted specific information.  
