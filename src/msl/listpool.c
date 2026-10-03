#include <msl/listpool.h>
#include <stdio.h>

void ListNext(ListNode** node) {
  *node = (*node)->next;
}

ListNode* ListRemove(ListNode** node) {
  ListNode* next;
  ListNode** previous;
  ListNode* removed = *node;

  if (*node == NULL) {
    printf("ListRemove: NULL node, ignored\n");
    return NULL;
  }
  next = (*node)->next;
  if (next != NULL) {
    next->pprev = (*node)->pprev;
  }
  previous = (*node)->pprev;
  if (previous != NULL) {
    *previous = (*node)->next;
  }
  (*node)->next = NULL;
  (*node)->pprev = NULL;
  *node = next;
  return removed;
}

void ListInsert(ListNode** head, ListNode* node) {
  if (node->pprev != NULL) {
    printf("ListInsert: was in another list, performing REMOVE first\n");
    node = ListRemove(&node);
  }
  node->next = *head;
  node->pprev = head;
  if (*head != NULL) {
    (*head)->pprev = &node->next;
  }
  *head = node;
}

u32 ListNodeID(ListPool* pool, ListNode* node) {
  ListNodeId id;

  id.parts.index = node->index;
  id.parts.serial = node->serial;
  return id.value;
}

ListNode* ListNodeFind(ListPool* pool, u32 value) {
  ListNodeId id;
  ListNode* node = NULL;

  id.value = value;
  if (pool == NULL) {
    printf("ListNodeFind NULL pool\n");
    return NULL;
  }
  if (pool->count == 0) {
    printf("ListNodeFind pool was never Attached\n");
    return NULL;
  }
  if (id.parts.index >= pool->count) {
    printf("ListNodeFind: id %08x exceeds pool size (%x)\n", id.value, pool->count);
  } else {
    node = pool->nodes + (u16)id.parts.index;
    if (node->state == 0 || node->serial != id.parts.serial) {
      printf("ListNodeFind: Stale ID 0x%08x for Pool 0x%08x\n", id.value, pool);
      node = NULL;
    }
  }
  return node;
}

void* ListNodeData(ListPool* pool, ListNode* node) {
  if (node == NULL) {
    printf("ListNodeData NULL node\n");
    return NULL;
  }
  return node->data;
}

static void ListPoolReset(ListPool* pool) {
  ListNode* node;
  ListNode* previous = NULL;
  u8* data;
  u32 index;
  if (pool == NULL) {
    printf("ListPoolReset NULL pool\n");
  } else if (pool->count == 0) {
    printf("ListPoolReset pool was never Attached\n");
  } else {
    data = pool->data;
    index = 0;
    node = pool->nodes;
    for (; index < pool->count; index++, node++, data += pool->dataSize) {
      node->index = index;
      node->state = 0;
      if (pool->dataSize == 0) {
        node->data = NULL;
      } else {
        node->data = data;
      }
      node->next = previous;
      if (previous != NULL) {
        previous->pprev = &node->next;
      }
      previous = node;
    }
    pool->freeList = previous;
    previous->pprev = &pool->freeList;
    pool->used = 0;
  }
}

void ListNodeFree(ListPool* pool, ListNode* node) {
  if (pool == NULL) {
    printf("ListPoolReset NULL pool\n");
    return;
  }
  if (pool->count == 0) {
    printf("ListPoolReset pool was never Attached\n");
    return;
  }
  if (node == NULL) {
    printf("ListNodeFree NULL node\n");
    return;
  }
  if (node < pool->nodes || node >= pool->nodes + pool->count) {
    printf("ListNodeFree invalid Node 0x%08x\n", node);
    return;
  }
  if (node->state) {
    node->state = 0;
    node->serial++;
    if (pool->dataSize == 0) {
      node->data = NULL;
    }
    ListInsert(&pool->freeList, node);
    pool->used--;
  }
}

ListNode* ListNodeAllocIDrange(ListPool* pool, u32 value, u32 min, u32 max) {
  union {
    u32 value;
    struct {
      u16 index;
      u16 serial;
    } parts;
  } id;
  ListNode* node;
  u32 index;
  id.value = value;
  if (pool == NULL) {
    printf("ListNodeAllocIDrange NULL pool\n");
    return NULL;
  }
  if (pool->count == 0) {
    printf("ListNodeAllocIDrange pool was never Attached\n");
    return NULL;
  }
  if (value == 0) {
    printf("ListNodeAllocIDrange: id 0 Invalid\n");
    return NULL;
  }
  if (max < min) {
    printf("ListNodeAllocIDrange: max (%d) is < min (%d)\n", max, min);
    return NULL;
  }
  if (max >= pool->count) {
    printf("ListNodeAllocIDrange: max 0x%x exceeds pool size (%x)\n", max, pool->count);
    return NULL;
  }
  index = id.parts.index;
  if (index >= pool->count) {
    printf("ListNodeAllocIDrange: id %08x exceeds pool size (%x)\n", value, pool->count);
    return NULL;
  }
  if (index > max) {
    printf("ListNodeAllocIDrange: id %08x exceeds max (0x%x)\n", value, max);
    return NULL;
  }
  if (index < min) {
    index = min;
  }
  for (; index <= max; index++) {
    node = pool->nodes + index;
    if (node->state == 0) {
      if (node->pprev == NULL) {
        printf("ListNodeAllocIDrange (%08x): node not in free list???\n", value);
        return NULL;
      }
      node = ListRemove(&node);
      node->state = 1;
      node->serial = id.parts.serial;
      pool->used++;
      return node;
    }
  }
  return NULL;
}

ListNode* ListNodeAllocID(ListPool* pool, u32 value) {
  union {
    u32 value;
    struct {
      u16 index;
      u16 serial;
    } parts;
  } id;
  ListNode* node;
  id.value = value;
  if (pool == NULL) {
    printf("ListNodeAllocID NULL pool\n");
    return NULL;
  }
  if (pool->count == 0) {
    printf("ListNodeAllocID pool was never Attached\n");
    return NULL;
  }
  if (value == 0) {
    printf("ListNodeAllocID: id 0 Invalid\n");
    return NULL;
  }
  if (id.parts.index >= pool->count) {
    printf("ListNodeAllocID: id %08x exceeds pool size (%x)\n", value, pool->count);
    return NULL;
  }
  node = pool->nodes + id.parts.index;
  if (node->state != 0) {
    printf("ListNodeAllocID: ID %08x is already in use\n", value);
    return NULL;
  }
  if (node->pprev == NULL) {
    printf("ListNodeAllocID (%08x): node not in free list???\n", value);
    return NULL;
  }
  node = ListRemove(&node);
  node->state = 1;
  node->serial = id.parts.serial;
  pool->used++;
  return node;
}

ListNode* ListNodeAlloc(ListPool* pool) {
  ListNode* node;
  if (pool == NULL) {
    printf("ListNodeAlloc NULL pool\n");
    return NULL;
  }
  if (pool->count == 0) {
    printf("ListNodeAlloc pool was never Attached\n");
    return NULL;
  }
  if (pool->freeList == NULL) {
    printf("ListNodeAlloc no more nodes\n");
    return NULL;
  }
  node = ListRemove(&pool->freeList);
  node->state = 1;
  pool->used++;
  return node;
}

void ListPoolDetach(ListPool* pool) {
  if (pool == NULL) {
    return;
  }
  if (pool->used != 0) {
    printf("ListPoolDetach discarding %d un-deleted elements\n", pool->used);
  }
  pool->dataSize = 0;
  pool->count = 0;
  pool->data = NULL;
  pool->nodes = NULL;
  pool->freeList = NULL;
  pool->used = 0;
}

int ListPoolAttach(ListPool* pool, void* mem, u32 count, u32 dataSize) {
  if (pool == NULL) {
    printf("ListPoolAttach NULL == pool\n");
    return 0;
  }
  if (mem == NULL) {
    printf("ListPoolAttach NULL == mem\n");
    return 0;
  }
  if (count == 0) {
    printf("ListPoolAttach to what?  elts == 0\n");
    return 0;
  }
  printf("ListPoolAttach: %d elts of size %d at 0x%08x\n", count, dataSize, mem);
  pool->count = count;
  pool->dataSize = dataSize;
  pool->data = mem;
  pool->nodes = (ListNode*)((u8*)mem + count * dataSize);
  ListPoolReset(pool);
  pool->unk18 = 0;
  return 1;
}
