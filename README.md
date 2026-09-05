# pc-port-scanner
This scanns the open connections on your pc via the windows api, the connections are sepperated by UDP and TCP.


To compile this i have tested it only with gcc but that looks like this: 
gcc main.c -o network -lws2_32 -liphlpapi -lpsapi

The headers you need to include
iphlpapi.h
psapi.h
