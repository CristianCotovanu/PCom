Router Forwarding 
==
Description
--
Simulating the process of packet forwarding inside a router.

Usage
--
* Checker: "sudo python3 topo.py tests" or "./check.sh"

* Interactive:  
Compile with **Makefile**.  
Simulate the netwrk with the python script **sudo python3 topo.py**.  
It will open 4 hosts (h0, h1, h2, h3).  
From any of them you can ping any other.  

Dependencies
--
To test it, we have to simulate a virtual network, here we are going to use Mininet and  
openvswitch-testcontroller.

sudo apt install mininet openvswitch-testcontroller xterm python3-pip  
sudo cp /usr/bin/ovs-testcontroller /usr/bin/ovs-controller  
sudo pip3 install scapy  
sudo pip3 install pathlib   
sudo pip3 install git+https://github.com/mininet/mininet.git 

For a better way of testing and visualising the whole process I recommend   
using **wireshark** to see what packets are forwarded and where.


Implementation
--
##### Application flow:  
1. Parse the routing table and place the information in a hash map<destIp, prefix>.  
2. Initialize the queue for packets and the ARP table.
3. Check if packet received is an **ARP**.
    * **ARP REQUEST** - if destination is the router, send an ARP Reply with router's MAC Address. 
    * **ARP REPLY** - Add the new entry in the ARP table and forward the corresponding   
    packet with the new MAC from the waiting list. 
4. Check if packet received is an **IP**.
5. Check if IP contains an **ICMP Echo Request**, case being sent an **ICMP Echo Reply** as response.
6. Query routing table for **the destination address**.
    *  If not found send **ICMP Host Unreachable**.
7. If found, query the ARP table for the corresponding **MAC Address**.
8. Did not find MAC: Send ARP Request on broadcast address and put the packet  
in the waiting list until we find the corresponding MAC Address.  
9. Found MAC: Forwarding process begins
    * Verify checksum.
    * Decrease TTL. If 0 send ICMP Time Exceeded.
    * Forward packet to the corresponding route.

Bonus: The checksum calculus is done using the RFC 1624 formula.
