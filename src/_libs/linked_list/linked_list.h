//
// source released under artistic license (see license.txt)
//


struct savemsg
{
	struct savemsg *next;
    int             size;    // sizeof following data
    int             msg_id;  // ident of the item given by LL_PushMsg
    
    void           *data;    // the user data is saved here
    int             type;    // complimentary integer stored
}; // savemsg     


// Create a new linked list
int   LL_Create (int id, int max_msg); 
void  LL_Destroy (int id);
int   LL_PushMsg (int id, const void *lpData, int dwSize);
void *LL_PopMsg (int id);
int   LL_PushTypedMsg (int id, const void *lpData, int dwSize, int type);
void *LL_PopTypedMsg (int id, int *plen, int *pmsg_id, int *ptype);

void  WaitForMsgQueueToFinish (int id);


// we use 3 linked List
#define LL_ID_LOG         0
#define LL_ID_SETTINGS    1
#define LL_ID_MSG_TO_GUI  2


