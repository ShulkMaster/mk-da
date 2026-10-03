#ifndef MSL_LISTPOOL_H
#define MSL_LISTPOOL_H

#include <dolphin/types.h>

typedef struct ListNode {
  void* data;
  struct ListNode* next;
  struct ListNode** pprev;
  u16 state : 2;
  u16 index : 14;
  u16 serial;
} ListNode;

/* Node handle: index and serial packed in one word (passed and returned in a
   single register). */
struct ListNodeIdParts {
  u16 index;
  u16 serial;
};

typedef union ListNodeId {
  u32 value;
  struct ListNodeIdParts parts;
} ListNodeId;

typedef struct ListPool {
  u32 dataSize;
  u32 count;
  void* data;
  ListNode* nodes;
  ListNode* freeList;
  u32 used;
  u32 unk18;
} ListPool;

void ListNext(ListNode** node);
ListNode* ListRemove(ListNode** node);
void ListInsert(ListNode** head, ListNode* node);
u32 ListNodeID(ListPool* pool, ListNode* node);
ListNode* ListNodeFind(ListPool* pool, u32 id);
void* ListNodeData(ListPool* pool, ListNode* node);
void ListNodeFree(ListPool* pool, ListNode* node);
int ListPoolAttach(ListPool* pool, void* mem, u32 count, u32 dataSize);

extern ListPool g_listPoolSound;

#endif
