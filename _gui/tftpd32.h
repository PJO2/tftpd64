/* Weditres generated include file. Do NOT edit */
#ifdef STANDALONE_EDITION
#  ifdef _M_X64
#     define   APP_TITLE              "Tftpd64 by Ph. Jounin"
#  else
#     define   APP_TITLE              "Tftpd32 by Ph. Jounin"
#  endif
#elif defined SERVICE_EDITION
#  ifdef _M_X64
#     define   APP_TITLE              "Tftpd64 Service Edition by Ph. Jounin"
#  else
#     define   APP_TITLE              "Tftpd32 Service Edition by Ph. Jounin"
#  endif
#endif


#define    IDC_BOOTP_NBFIELDS  3
#define    IDC_BOOTP_NBRAWS    4
//#define DLG_100         100

#define  IDM_SERVER_LIST  50
#define  ID_STOP_TRANSFER 51

#define  IDM_DHCP_LIST         55
#define  ID_DELETE_ASSIGNATION 56

#define  IDC_STATIC -1

#define IDD_TFTPD32         101
#define IDI_TFTPD32         102
#define IDD_DIALOG_ABOUT    103
#define IDD_TFTPD_CLIENT    104
#define IDD_DIALOG_SETTINGS 107

#define IDD_DIALOG_GAUGE    108
#define IDD_DIALOG_SHDIR    110
#define IDD_DIALOG_BROWSE   111
#define IDI_TFTPD32KILL     112
#define IDD_DIALOG_FIREWALL 113

#define IDD_DIALOG_SETTINGS_TFTP 120
#define IDD_DIALOG_SETTINGS_DHCP 121

////////////////////////////////////////////////////////////
// BASKET
////////////////////////////////////////////////////////////

#define    IDC_TAB_OPTION      4000
#define    IDC_ABOUT_BUTTON    4002
#define    IDC_SETTINGS_BUTTON 4004
#define    IDC_SHDIR_BUTTON    4005
#define    IDC_TFTPD_HELP      4007
#define    IDC_CB_IP           4008
#define    IDC_TXT_ADDRESS     4009
#define    IDC_TXT_BASEDIR     4010
#define    IDC_BROWSE_BUTTON   4012
#define    IDC_BASE_DIRECTORY  4013
#define    IDC_ABOUT_TFTPD32   4014
#define    IDC_TFTPD_STRING    4015
#define    IDC_CB_DIR          4016

////////////////////////////////////////////////////////////
// MAIN LIST BOX
////////////////////////////////////////////////////////////

#define   IDC_LB_LOG      4100



////////////////////////////////////////////////////////////
// SETTINGS
////////////////////////////////////////////////////////////

#define IDC_TAB_SETTINGS    1199
#define IDC_GRP_TFTP        1210
#define IDC_GRP_GLOBAL      1211
#define IDC_GRP_SYSLOG      1212
#define IDC_GRP_TFTP_SEC    1213
#define IDC_GRP_TFTP_CFG    1214
#define IDC_GRP_TFTP_ADVANCED 1215
#define IDC_GRP_DHCP         1216
#define IDC_GRP_DHCP_POOL    1217
#define IDC_GRP_DHCP_OPT     1218
#define IDC_GRP_GLOBAL_IPv6  1219

#define IDC_TRF_PROGRESS    1280
#define    IDC_FILE_SIZE       1281
#define    IDC_FILE_STATS      1282
#define    IDC_RADIO_SECNONE       1290
#define    IDC_RADIO_SECSTD        1291
#define    IDC_RADIO_SECHIGH       1292
#define    IDC_RADIO_SECRO         1293
#define    IDC_BASEDIR             1303
#define    IDC_TIMEOUT             1304
#define    IDC_MAXRETRANSMIT       1305
#define    IDC_PORT                1306
#define    IDC_CHECK_HIDE          1307
#define    IDC_WINSIZE             1308
#define    IDC_CHECK_NEGOCIATE     1309
#define    IDC_CHECK_PROGRESS      1310
#define    IDC_CHECK_DIRTEXT       1311
#define    IDC_BUTTON_BROWSE       1312
#define    IDC_CHECK_TFTPLOCALIP   1313
#define    IDC_CB_TFTPLOCALIP      1314
#define    IDC_CHECK_UNIX          1315
#define    IDC_CHECK_BEEP          1316
#define    IDC_CHECK_WINSIZE       1317
#define    IDC_CHECK_SAVE_SYSLOG   1318
#define    IDC_CHECK_TFTP_SERVER   1319
#define    IDC_CHECK_TFTP_CLIENT   1320
#define    IDC_CHECK_DHCP_SERVER   1321
#define    IDC_CHECK_SYSLOG_SERVER 1322
#define    IDC_CHECK_SNTP_SERVER   1323
#define    IDC_CHECK_DNS_SERVER    1324
#define    IDC_CHECK_IPv6          1325

#define    IDC_CHECK_PXE           1330
#define    IDC_CHECK_MD5           1331
#define    IDC_CHECK_VROOT         1350
#define    IDC_BUTTON_DEFAULT      1351
#define    IDC_SYSLOG_FILE         1352
#define    IDC_LOCAL_PORTS         1353
#define    IDC_CHECK_PIPE_SYSLOG   1354
#define    IDC_TXT_SYSLOGTOFILE    1355
#define    IDC_TXT_TFTP_TIMEOUT    1356
#define    IDC_TXT_TFTP_RETRANSMIT 1357
#define    IDC_TXT_TFTP_PORT       1358
#define    IDC_TXT_TFTP_PORTS	   1359
#define    IDC_TXT_WINSIZE		   1360

#define    IDC_DIR_TEXT            1402
#define    IDC_CHECK_PERS_LEASES   1403
#define    IDC_CHECK_LOCALIP_DHCP  1404
#define    IDC_CB_LOCALIP_DHCP     1405
#define    IDC_CHECK_PING          1406
#define    IDC_CHECK_DOUBLE_ANSWER 1407

//#define IDC_TREEDIR         1500

////////////////////////////////////////////////////////////
// TFTP SERVER
////////////////////////////////////////////////////////////

#define IDC_TXT_CURACTION   1501
#define    IDC_TFTP_CLEAR      1502
// #define    IDC_CURRENT_ACTION  1503
#define    IDC_TFTP_COPYTOCLIP 1504
#define   IDC_LV_TFTP          1505

////////////////////////////////////////////////////////////
// SYSLOG
////////////////////////////////////////////////////////////

#define  IDC_LB_SYSLOG       1600
#define    IDC_SYSLOG_CLEAR    1601
#define    IDC_SYSLOG_COPY     1602


////////////////////////////////////////////////////////////
// SNTP
////////////////////////////////////////////////////////////


#define  IDC_TXT_SNTP        1700

////////////////////////////////////////////////////////////
// DNS Server
////////////////////////////////////////////////////////////

#define  IDC_TXT_DNS        1800
#define  IDC_LB_DNS         1801


////////////////////////////////////////////////////////////
// TFTP CLIENT
////////////////////////////////////////////////////////////


#define    IDC_TXT_CLIENTHOST  2001
#define    IDC_TXT_CLIENT_LOCALFILE  2002
#define    IDC_CLIENT_GET_BUTTON   2003
#define    IDC_CLIENT_SEND_BUTTON  2004
#define    IDC_CLIENT_HOST     2005
#define    IDC_CLIENT_LOCALFILE    2006
#define    IDC_CLIENT_PROGRESS     2007
#define    IDC_CLIENT_BREAK_BUTTON 2008
#define    IDC_CLIENT_BLOCK        2009
#define    IDC_TXT_CLIENTPORT      2010
#define    IDC_CLIENT_PORT         2011
#define    IDC_TXT_CLIENTBLOCKSIZE 2012
#define    IDC_CB_DATASEG          2013
#define    IDC_CLIENT_BROWSE       2014
#define    IDC_CLIENT_FULL_PATH        2015
#define    IDC_TXT_CLIENT_REMOTEFILE  2016
#define    IDC_CLIENT_REMOTEFILE     2017
#define    IDC_TEXT_TFTPCLIENT_HELP  2018


////////////////////////////////////////////////////////////
// DHCP
////////////////////////////////////////////////////////////
#define    IDC_DHCP_ADDRESS_POOL    3002
#define    IDC_DHCP_MASK            3003
#define    IDC_DHCP_DNS_SERVER      3004
#define    IDC_DHCP_POOL_SIZE       3005
#define    IDC_DHCP_BOOT_FILE       3006
#define    IDC_DHCP_POOL            3007
#define    IDC_DHCP_DEFAULT_ROUTER  3008
#define    IDC_DHCP_DOMAINNAME      3009
#define    IDC_DHCP_ADDOPTION_NB    3010
#define    IDC_DHCP_ADDOPTION_VALUE 3011
#define	   IDC_DHCP_LEASE           3012
#define	   IDC_DHCP_OPTION42        3013
#define	   IDC_DHCP_OPTION120       3014
#define    IDC_DHCP_WINS_SERVER     3015

#define    IDC_TXT_DEFAULT_ROUTER  3101
#define    IDC_TXT_ADDRESS_POOL    3102
#define    IDC_TXT_POOL_SIZE       3103
#define    IDC_TXT_BOOT_FILE       3104
#define    IDC_TXT_DNS_SERVER      3105
#define    IDC_TXT_MASK            3106
#define    IDC_TXT_DOMAINNAME      3107
#define    IDC_TXT_ADDOPTION       3108
#define    IDC_TXT_LEASE           3109
#define    IDC_TXT_OPTION42        3110
#define    IDC_TXT_OPTION120	   3111
#define    IDC_TXT_WINS_SERVER	   3112

#define   IDC_DHCP_OK     3200
#define   IDC_LV_DHCP	  3300

////////////////////////////////////////////////////////////
// SHOW DIR
////////////////////////////////////////////////////////////

#define    IDC_LB_SHDIR        3401
#define    IDC_SD_EXPLORER     3402
#define    IDC_SD_COPY         3403


////////////////////////////////////////////////////////////
// Firewall
////////////////////////////////////////////////////////////

#define IDC_CHECK_FIREWALL 3501

////////////////////////////////////////////////////////////
// CMSGBOX
////////////////////////////////////////////////////////////
#define IDD_MESSAGEBOXDLG  600001
#define IDC_MSGBOXTEXT     600002
#define IDI_MSGBOXICON  600010
#define IDI_MSGBOXICONQUESTION  600011
#define IDI_MSGBOXICONEXCLA  600012
#define IDI_MSGBOXICONINFO  600013

////////////////////////////////////////////////////////////
// END
////////////////////////////////////////////////////////////

#define   IDC_TXT_BAD_SERVICES    10000
#define   _APS_NEXT_COMMAND_VALUE 40001


