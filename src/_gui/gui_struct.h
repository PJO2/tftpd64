

///////////////////////////////////
// structure of GUI database

struct S_TftpGui
{
    // identifier
    DWORD   dwTransferId;       
    // items to be displayed
    char   *filename;
    SOCKADDR_STORAGE stg_addr;
    int    opcode;
    // stats
    struct S_Trf_Statistics stat;
    // GUI resources
    HWND    hGaugeWnd;
    // next
    struct S_TftpGui *next;
};
