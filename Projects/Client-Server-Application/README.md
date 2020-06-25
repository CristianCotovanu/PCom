Client-Server Application 
==
Description
--
Client-Server application offering functionality of a message manager.

- Generators - clients who generate the updates on specific topics    
and send them to the server.
- Subscribers - clients who subscribe to a number of topics that they will   
receive updates from, delivered by the server.
- Message broker - the server which delivers and stores messages from generators  
and then redirects them to the corresponding subscribers.    

Usage
--
Compile the application with the Makefile.  
Run recommendation:   
IP_SERVER = 127.0.0.1  
PORT_SERVER = 8080

**UDP Clients** represent the message generator for the topics. 
Generators can be run:
- automatically with **python3 udp_client.py <IP_SERVER> <PORT_SERVER>** and send updates on all topics.  
- manually with **python3 udp_client.py --mode manual <IP_SERVER> <PORT_SERVER>** and then  
selecting on which topic to publish updates with a number from 1 to 15.  

**TCP Clients** represent the subscribers connecting to the server and receiving updates.  
A client can selected the Store & Forward option, once subscribed to a topic with SF  
active, even if the client is not connected to the server, updates on that topic will  
be delivered once the user connects.
SF can be either 1 or 0 case in which the SF option is active or not.  

You can run a client with **./subscriber <ID_CLIENT> <IP_SERVER> <PORT_SERVER>**.  
Subscribers can:
- subscribe to a topic with **subscribe \<TOPIC> \<SF>**
- unsubscribe from a topic with **unsubscribe \<Topic>**
- close the connection with **exit**

**Server** represents the message broker and delivers the topic updates from UDP clients to      
the coresponding TCP clients who have subscribed.  
Topics are hardcoded in the "sample_payloads.json".

Server can:
- close itself and all the connected clients with **exit**

When command formats are not correct suggestive error messages will be shown to the console.

Implementation
--
Implementation based on UDP/TCP connection multiplexing.  

##### Server
I get a UDP topic update:
I check all clients who have subscribed to the topic and send them  
the message if only else if they have the SF option, I place the update in a buffer.

I get a TCP connection:
ID check meaning that the user reconnects/first connection/already connected.  
I get a TCP message:
- subscribe: add the topic to the user's list and set the SF if I should.
- unsubscribe: remove the topic from the user's list.
- exit: I close the socket and set the online flag to false.  
 
I get stdin input:
- exit: send all clients a message so they know they should close connections, then  
close all open sockets.  

Any other input is invalid.  
If the server shuts down unexpectedly the clients have a timeout for closing. 
