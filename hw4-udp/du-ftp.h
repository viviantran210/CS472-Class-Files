#pragma once

#define PROG_MD_CLI     0
#define PROG_MD_SVR     1
#define DEF_PORT_NO     2080
#define FNAME_SZ        150
#define PROG_DEF_FNAME  "test.c"
#define PROG_DEF_SVR_ADDR   "127.0.0.1"

typedef struct prog_config{
    int     prog_mode;
    int     port_number;
    char    svr_ip_addr[16];
    char    file_name[128];
} prog_config;

#define     DP_S_IN_PROGRESS          0
#define     DP_S_COMPLETE             1
#define     DP_S_ERROR               -1
#define     DP_ERROR_FILE_OPEN       -33 // cannot open file

// Define the PDU for the FTP protocol
typedef struct dp_ftp_pdu {
    int     proto_ver;
    int     mtype;    
    char    file_name[128]; // File name being transferred
    int     status;         // Status of the transfer (0 = in progress, 1 = complete)
    int     seqnum;
    int     err_num;   
    int     dgram_sz;    
} dp_ftp_pdu;