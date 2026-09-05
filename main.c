#include "stdio.h"
#include "winsock2.h"
#include "iphlpapi.h"

#include "network.c"


int main(char *argv[], int argc) {
    Port_information info = {0};   

    if (get_udp_table(&info) != 0) {
        fprintf(stderr, "Failed to get UDP table\n");
    }
    if (get_tcp_table(&info) != 0) {
        fprintf(stderr, "Failed to get TCP table\n");
    }
    if (get_process_name(&info, MAX_PROSS_NAME) != 0) {
        fprintf(stderr, "Failed to get the process' name\n");
    }

    struct in_addr addr;

    printf("--- UDP (%lu entries) ---\n", info.UdpCount);
    for (DWORD i = 0; i < info.UdpCount; i++) {
        addr.s_addr = info.UdpEntries[i].LocalIp;
        printf("IP: %-15s Port: %-6u PID: %lu Name:%s\n",
               inet_ntoa(addr), info.UdpEntries[i].LocalPort, info.UdpEntries[i].Pid, info.UdpEntries[i].ProcessName);
    }

    printf("--- TCP (%lu entries) ---\n", info.TcpCount);
    for (DWORD i = 0; i < info.TcpCount; i++) {
        addr.s_addr = info.TcpEntries[i].LocalIp;
        printf("Local IP: %-15s Port: %-6u PID: %lu Name:%s\n",
               inet_ntoa(addr), info.TcpEntries[i].LocalPort, info.TcpEntries[i].Pid, info.TcpEntries[i].ProcessName);
    }

    free_port_information(&info);
    return 0;

}