//////////////////////////////////////////////////////
//
// Projet TFTPD32.  November 2006 Ph.jounin
//                  A free DHCP server for Windows
// File linked list.c: Linked list management
//                                        from Nick Wagner
//
//
// Released under European Union Public License
//
//////////////////////////////////////////////////////

#include <windows.h>
#include <assert.h>
#include "linked_list.h"

// ---------------------------------------------------------------
// A multithread linked list 
// This module exports the function LL_PushMsg and LL_PopMsg
// which add (retrieve) a message to (from) a list 
// A Semaphore is defined in order to prevent corrupted data
// ---------------------------------------------------------------


#define SizeOfTab(x)  (sizeof x / sizeof *x)
    
static struct S_LL
{
   struct savemsg  *phead;
   struct savemsg  *ptail;
   int              nb_items;
   int              cur_msg_id;     // current identifier
   int              max_items;
   HANDLE           msglock;   
}
tLL [10];
static int nbLL=0;



// ---------------------------------------------------------------
// linked list management
// ---------------------------------------------------------------

// CReate a new LL structure  (should be also protected by a semaphore...)
int LL_Create (int id, int max_msg)
{
   if (id >= SizeOfTab (tLL)-1)  return -1;
   tLL[id].phead = tLL[nbLL].ptail = NULL;
   tLL[id].nb_items = 0;
   tLL[id].cur_msg_id = 1;
   tLL[id].max_items = max_msg;
   tLL[id].msglock =  CreateMutex(NULL, 0, NULL);
return id;
} // LL_Create

void LL_Destroy (int id)
{
  CloseHandle (tLL[id].msglock);
} // LL_Destroy

// AddItem    
int LL_PushTypedMsg (int id, const void *lpData, int dwSize, int type)
{
struct savemsg* pmsg ;
int    msg_id;
int    Rc;
int Try = 0, bSuccess=FALSE;
#define MAX_TRIES  5


	while (! bSuccess  &&  ++Try < MAX_TRIES)
	{
		Rc = WaitForSingleObject(tLL[id].msglock, INFINITE);
		assert (Rc==WAIT_OBJECT_0);
    
		// queue full ?
		if (tLL[id].nb_items < tLL[id].max_items - 1)
		{
			// one more item
			++tLL[id].nb_items;

			pmsg = malloc(sizeof(struct savemsg));
			assert(pmsg != NULL);

			pmsg->data = malloc(dwSize);
			pmsg->size = dwSize;
			pmsg->next = NULL;
			memcpy(pmsg->data, lpData, dwSize);
			pmsg->type = type;
			// attribute unique id to the msg
			pmsg->msg_id = msg_id = tLL[id].cur_msg_id++;


			// Assumes you have the lock
			if (!tLL[id].phead)
				tLL[id].phead = tLL[id].ptail = pmsg;
			else
			{
				if (!tLL[id].ptail) //Shouldn't be hit, but just in case
				{
					struct savemsg* ptmp = tLL[id].phead;
					while (ptmp)
					{
						tLL[id].ptail = ptmp;
						ptmp = ptmp->next;
					}
				}

				tLL[id].ptail->next = pmsg;
				tLL[id].ptail = pmsg;
			}
			bSuccess = TRUE;
		} // queue is not full
		else
		{
			ReleaseMutex(tLL[id].msglock);
			OutputDebugString("--------- Log queue full\n");
			Sleep(50);
		}
	} // 5 tries

	ReleaseMutex(tLL[id].msglock);

    // can not return directly pmsg->msg_id
    // since it may have already been freed
return msg_id;
} // LL_PushTypedMsg


// Returns when the message queue is empty.  Useful for loading the ini file after a save
void WaitForMsgQueueToFinish (int id)
{
struct savemsg* pmsg = tLL[id].phead;
int Rc;
    while (pmsg != NULL)
    {
		Sleep(0);  // Pass the hand to other threads for the queue to empty        
        Rc = WaitForSingleObject(tLL[id].msglock, INFINITE);
		assert (Rc==WAIT_OBJECT_0);
        pmsg = tLL[id].phead;
        ReleaseMutex (tLL[id].msglock);
    } ;
}  // WaitForMsgQueueToFinish()


//Assumes you have the lock 
//Returns NULL if no message available
void *LL_PopTypedMsg (int id, int *plen, int *pmsg_id, int *ptype)
{
struct savemsg *pcur;
void           *lpData;
int             Rc;

    if (tLL[id].phead==NULL) return NULL;

	Rc = WaitForSingleObject(tLL[id].msglock, INFINITE);
    assert (Rc==WAIT_OBJECT_0);

    pcur = tLL[id].phead;
	if(tLL[id].phead)
	{
		if(tLL[id].ptail == tLL[id].phead)
			tLL[id].ptail = NULL;  //We'll be removing the head
		tLL[id].phead = tLL[id].phead->next;
	}
    tLL[id].nb_items--;
    
    ReleaseMutex (tLL[id].msglock);
    
    // return only user data, forget container 
    lpData = pcur->data;
    if (pmsg_id != NULL) *pmsg_id = pcur->msg_id ; 
    if (ptype != NULL)   *ptype = pcur->type;
    if (plen != NULL)    *plen = pcur->size;
    free (pcur);

return lpData;
} // LL_PopTypedMsg

//Assumes you have the lock 
//Returns NULL if no message available
void *LL_PopMsg (int id)
{
return LL_PopTypedMsg (id, NULL, NULL, NULL);
} // LL_PopMsg


int LL_PushMsg (int id, const void *data, int dwSize)
{
return LL_PushTypedMsg (id, data, dwSize, 0);
} // LL_PushMsg
