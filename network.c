#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <iphlpapi.h>
#include <processthreadsapi.h>

// this is the max path length for a process
#define MAX_PROSS_NAME 260

// One UDP socket entry
typedef struct {
    DWORD   LocalIp;    
    u_short LocalPort;  
    DWORD   Pid;
    char ProcessName[MAX_PROSS_NAME];
} UdpEntry;

// One TCP socket entry   
typedef struct {
    DWORD   LocalIp;
    u_short LocalPort;
    DWORD   RemoteIp;
    u_short RemotePort;
    DWORD   State;      
    DWORD   Pid;
    char ProcessName[MAX_PROSS_NAME];
} TcpEntry;

// keeps both tables together for my personal convience
typedef struct Port_information {
    UdpEntry *UdpEntries;
    DWORD     UdpCount;

    TcpEntry *TcpEntries;
    DWORD     TcpCount;
} Port_information;

//resolves the proccess' name by itering over each table by them selves
// probaly could do it more legant but this works
// the unkwnowns from what i have seen are so far all on windows internals
// probaly need to replace the permissions with one higgher could also move the creating of the hProcess
//out of each loop and only make it once, and could reduce a couple varibles here and there
// this is work ing tho so ima leave it for future me  will look at this later
int get_process_name(Port_information *port_info, DWORD name_out_size) {
    // resolve for every UDP entry
    for (DWORD i = 0; i < port_info->UdpCount; i++) {
        DWORD pid = port_info->UdpEntries[i].Pid;
        char *name_out = port_info->UdpEntries[i].ProcessName;

        HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
        if (hProcess == NULL) {
            strncpy(name_out, "{unknown}", name_out_size);
            continue;   
        }

        DWORD size = name_out_size;   
        BOOL ok = QueryFullProcessImageNameA(hProcess, 0, name_out, &size);
        CloseHandle(hProcess);        
        if (!ok) {
            strncpy(name_out, "{unknown}", name_out_size);
            continue;
        }

        char *short_name = strrchr(name_out, '\\');
        if (short_name != NULL) {
            memmove(name_out, short_name + 1, strlen(short_name + 1) + 1);
        }
    }

    // --- resolve for every TCP entry ---
    for (DWORD i = 0; i < port_info->TcpCount; i++) {
        DWORD pid = port_info->TcpEntries[i].Pid;
        char *name_out = port_info->TcpEntries[i].ProcessName;

        HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
        if (hProcess == NULL) {
            strncpy(name_out, "{unknown}", name_out_size);
            continue;
        }

        DWORD size = name_out_size;
        BOOL ok = QueryFullProcessImageNameA(hProcess, 0, name_out, &size);
        CloseHandle(hProcess);

        if (!ok) {
            strncpy(name_out, "{unknown}", name_out_size);
            continue;
        }

        char *short_name = strrchr(name_out, '\\');
        if (short_name != NULL) {
            memmove(name_out, short_name + 1, strlen(short_name + 1) + 1);
        }
    }

    return 0;

}


// gets the udp table
int get_udp_table(Port_information *port_info) {
    PMIB_UDPTABLE_OWNER_PID pUdpTable = NULL;
    // this is the buffer being set to zero 
    //The estimated size of the structure returned in pUdpTable, in bytes. If this value is set too small, ERROR_INSUFFICIENT_BUFFER 
    //from windows docs ^^  the function puts the info there and if its too small you get a  to small buffer error
    DWORD dwSize = 0;
    //this checks if the buffer was too small which it probaly always will be
    if (GetExtendedUdpTable(NULL, &dwSize, TRUE, AF_INET, UDP_TABLE_OWNER_PID, 0)!= ERROR_INSUFFICIENT_BUFFER) {
        return -1;
    }
    //creates memmory space for the table with tthe type PMIB_UDPTABLE_OWNER_PID and of dwSzie
    pUdpTable = (PMIB_UDPTABLE_OWNER_PID)malloc(dwSize);
    // could probaly inprove error handling here
    if (pUdpTable == NULL) return -1;

    //if there is a error you free the memory holding what the table is
    if (GetExtendedUdpTable(pUdpTable, &dwSize, TRUE, AF_INET, UDP_TABLE_OWNER_PID, 0)
        != NO_ERROR) {
        free(pUdpTable);
        return -1;
    }
    // takes the size of oour UdpEntry type and multipleys it by how many entries to create extra space
    UdpEntry *entries = malloc(sizeof(UdpEntry) * pUdpTable->dwNumEntries);
    if (entries == NULL) {
        free(pUdpTable);
        return -1;
    }


    //these over here move the information to there supporting entries location on the object and then frees the table we created in this function 
    for (DWORD i = 0; i < pUdpTable->dwNumEntries; i++) {
        entries[i].LocalIp   = pUdpTable->table[i].dwLocalAddr;
        entries[i].LocalPort = ntohs((u_short)pUdpTable->table[i].dwLocalPort);
        entries[i].Pid       = pUdpTable->table[i].dwOwningPid;
    }

    port_info->UdpEntries = entries;
    port_info->UdpCount   = pUdpTable->dwNumEntries;

    free(pUdpTable);
    return 0;
}

//  gets the tcp table this and the udp_table have the same logic, i probaly can get both functions down to one, but i didn't want to get confused when first writing this
int get_tcp_table(Port_information *port_info) {
    PMIB_TCPTABLE_OWNER_PID pTcpTable = NULL;
    DWORD dwSize = 0;

    if (GetExtendedTcpTable(NULL, &dwSize, TRUE, AF_INET, TCP_TABLE_OWNER_PID_ALL, 0)
        != ERROR_INSUFFICIENT_BUFFER) {
        return -1;
    }

    pTcpTable = (PMIB_TCPTABLE_OWNER_PID)malloc(dwSize);
    if (pTcpTable == NULL) return -1;

    if (GetExtendedTcpTable(pTcpTable, &dwSize, TRUE, AF_INET, TCP_TABLE_OWNER_PID_ALL, 0)
        != NO_ERROR) {
        free(pTcpTable);
        return -1;
    }

    TcpEntry *entries = malloc(sizeof(TcpEntry) * pTcpTable->dwNumEntries);
    if (entries == NULL) {
        free(pTcpTable);
        return -1;
    }

    for (DWORD i = 0; i < pTcpTable->dwNumEntries; i++) {
        entries[i].LocalIp    = pTcpTable->table[i].dwLocalAddr;
        entries[i].LocalPort  = ntohs((u_short)pTcpTable->table[i].dwLocalPort);
        entries[i].RemoteIp   = pTcpTable->table[i].dwRemoteAddr;
        entries[i].RemotePort = ntohs((u_short)pTcpTable->table[i].dwRemotePort);
        entries[i].State      = pTcpTable->table[i].dwState;
        entries[i].Pid        = pTcpTable->table[i].dwOwningPid;
    }

    port_info->TcpEntries = entries;
    port_info->TcpCount   = pTcpTable->dwNumEntries;

    free(pTcpTable);
    return 0;
}

// Frees both arrays to be clear this is for the objects that go into get_tcp_table and get_udp_table so this shoudln't be ran in either of those functionss
void free_port_information(Port_information *port_info) {
    free(port_info->UdpEntries);
    free(port_info->TcpEntries);
    port_info->UdpEntries = NULL;
    port_info->TcpEntries = NULL;
    port_info->UdpCount = 0;
    port_info->TcpCount = 0;
}