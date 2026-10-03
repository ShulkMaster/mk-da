/*
 * This project builds on work from the Metroid Prime decompilation project.
 * Credit and thanks to the PrimeDecomp contributors.
 * https://github.com/PrimeDecomp/prime
 */

#include <dolphin/asm_sequences.inc>

#include <Runtime.PPCEABI.H/exception_fragment.h>
#include <dolphin/types.h>

typedef struct ProcessInfo {
  __eti_init_info* exception_info;
  char* TOC;
  int active;
} ProcessInfo;

static ProcessInfo fragmentinfo[1];

#include <exception>

namespace std {
class bad_exception : public exception {
public:
  bad_exception() {}
  virtual ~bad_exception() throw();
  virtual const char *what() const throw() { return "bad_exception"; }
};
}

struct CatchInfo {
  void* location;
  void* typeinfo;
  void* dtor;
  void* sublocation;
  s32 pointercopy;
  void* stacktop;
};

extern "C" char __throw_catch_compare(const char*, const char*, long*);

#define DTORCALL_COMPLETE(dtor, objptr) (((void (*)(void*, int))dtor)(objptr, -1))
using std::bad_exception;
using std::unexpected;

typedef u8 exaction_type;

typedef struct ex_branch {
  exaction_type action;
  u8 unused;
  u16 target;
} ex_branch;

typedef struct ex_destroylocal {
  exaction_type action;
  u8 unused;
  s16 local;
  void* dtor;
} ex_destroylocal;

typedef struct ex_destroylocalcond {
  exaction_type action;
  u8 dlc_field;
  s16 cond;
  s16 local;
  void* dtor;
} ex_destroylocalcond;

typedef struct ex_destroylocalpointer {
  exaction_type action;
  u8 dlp_field;
  s16 pointer;
  void* dtor;
} ex_destroylocalpointer;

typedef struct ex_destroylocalarray {
  exaction_type action;
  u8 unused;
  s16 localarray;
  u16 elements;
  u16 element_size;
  void* dtor;
} ex_destroylocalarray;

typedef struct ex_destroymember {
  exaction_type action;
  u8 dm_field;
  s16 objectptr;
  s32 offset;
  void* dtor;
} ex_destroymember;

typedef struct ex_destroymembercond {
  exaction_type action;
  u8 dmc_field;
  s16 cond;
  s16 objectptr;
  s32 offset;
  void* dtor;
} ex_destroymembercond;

typedef struct ex_destroymemberarray {
  exaction_type action;
  u8 dma_field;
  s16 objectptr;
  s32 offset;
  s32 elements;
  s32 element_size;
  void* dtor;
} ex_destroymemberarray;

typedef struct ex_deletepointer {
  exaction_type action;
  u8 dp_field;
  s16 objectptr;
  void* deletefunc;
} ex_deletepointer;

typedef struct ex_deletepointercond {
  exaction_type action;
  u8 dpc_field;
  s16 cond;
  s16 objectptr;
  void* deletefunc;
} ex_deletepointercond;

typedef struct ex_catchblock {
  exaction_type action;
  u8 unused;
  char* catch_type;
  u16 catch_pcoffset;
  s16 cinfo_ref;
} ex_catchblock;

typedef struct ex_activecatchblock {
  exaction_type action;
  u8 unused;
  s16 cinfo_ref;
} ex_activecatchblock;

typedef struct ex_specification {
  exaction_type action;
  u8 unused;
  u16 specs;
  s32 pcoffset;
  s32 cinfo_ref;
  char* spec[];
} ex_specification;

typedef struct ex_catchblock_32 {
  exaction_type action;
  u8 unused;
  char* catch_type;
  s32 catch_pcoffset;
  s32 cinfo_ref;
} ex_catchblock_32;

#define EXACTION_ACTIVECATCHBLOCK 13

#define EXACTION_BRANCH    1

#define EXACTION_CATCHBLOCK 12

#define EXACTION_CATCHBLOCK_32 16

#define EXACTION_DELETEPOINTER 10

#define EXACTION_DELETEPOINTERCOND 11

#define EXACTION_DESTROYBASE   6

#define EXACTION_DESTROYLOCAL 2

#define EXACTION_DESTROYLOCALARRAY 5

#define EXACTION_DESTROYLOCALCOND 3

#define EXACTION_DESTROYLOCALPOINTER 4

#define EXACTION_DESTROYMEMBER 7

#define EXACTION_DESTROYMEMBERARRAY 9

#define EXACTION_DESTROYMEMBERCOND 8

#define EXACTION_ENDBIT 0x80

#define EXACTION_MASK   0x7F

#define EXACTION_SPECIFICATION 15

#define ET_GetSavedGPRs(field) ((field) >> 11)

#define ET_GetSavedFPRs(field) (((field) >> 6) & 0x1f)

#define ET_GetHasFramePtr(field) (((field) >> 4) & 1)

#define RETURN_ADDRESS 4

using std::terminate;
struct ExceptionRangeSmall {
  u16 start;
  u16 end;
  u16 action;
};

struct ExceptionTableSmall {
  u16 et_field;
  ExceptionRangeSmall ranges[0];
};

struct ExceptionRangeLarge {
  u32 start;
  u16 size;
  u16 action;
};

struct ExceptionTableLarge {
  u16 et_field;
  u16 et_field2;
  ExceptionRangeLarge ranges[0];
};

struct ExceptionTableIndex {
  u32 functionoffset;
  u32 eti_field;
  u32 exceptionoffset;
};

struct MWExceptionInfo {
  ExceptionTableSmall* exception_record;
  char* current_function;
  char* action_pointer;
  char* code_section;
  char* data_section;
  char* TOC;
};

struct FragmentInfo {
  ExceptionTableIndex* exception_start;
  ExceptionTableIndex* exception_end;
  char* code_start;
  char* code_end;
  char* data_start;
  char* data_end;
  char* TOC;
  int active;
};

#define ETI_GetFunctionSize(field) ((field) & 0x7fffffff)
#define ETI_GetDirectStore(field) ((field) >> 31)
#define ET_IsLargeTable(field) (((field) >> 3) & 1)
struct ActionIterator {
  MWExceptionInfo info;
  char* current_SP;
  char* current_FP;
  s32 current_R31;
};

union MWE_GeckoVector64 {
  f64 d;
  f32 f[2];
};

struct GeckoFPRContext {
  f64 d;
  MWE_GeckoVector64 v;
};

struct ThrowContext {
  GeckoFPRContext FPR[32];
  s32 GPR[32];
  s32 CR;
  char* SP;
  char* FP;
  char* throwSP;
  char* returnaddr;
  char* throwtype;
  void* location;
  void* dtor;
  CatchInfo* catchinfo;
};

static char* ExPPC_PopStackFrame(ThrowContext*, MWExceptionInfo*);

#define ex_deletepointer_GetRegPointer(field)  ((field) >> 7)
#define ex_deletepointercond_GetRegCond(field)              ((field) >> 7)
#define ex_deletepointercond_GetRegPointer(field)           (((field) >> 6) & 0x1)
#define ex_destroylocalcond_GetRegCond(field)  ((field) >> 7)
#define ex_destroylocalpointer_GetRegPointer(field)  ((field) >> 7)
#define ex_destroymember_GetRegPointer(field)  ((field) >> 7)
#define ex_destroymemberarray_GetRegPointer(field)  ((field) >> 7)
#define ex_destroymembercond_GetRegCond(field)              ((field) >> 7)
#define ex_destroymembercond_GetRegPointer(field)           (((field) >> 6) & 0x1)

typedef s16 vbase_ctor_arg_type;
typedef char local_cond_type;
typedef void (*DeleteFunc)(void*);
#define DTORCALL_PARTIAL(dtor, objptr) (((void (*)(void*, int))dtor)(objptr, 0))

#define ET_HasElfVector(field) (((field) >> 1) & 1)

static void ExPPC_FindExceptionRecord(char*, MWExceptionInfo*);

static asm void ExPPC_LongJump(ThrowContext *, void *, void *) { SEQ_ExPPC_LongJump__FP12ThrowContextPvPv(); }

static void ExPPC_ThrowHandler(ThrowContext* context);
static void ExPPC_UnwindStack(ThrowContext*, MWExceptionInfo*, void*);
static exaction_type ExPPC_NextAction(ActionIterator*);
#define EXACTION_ENDOFLIST 0
#define EXACTION_TERMINATE 14

extern "C" asm void __throw(char* throwtype, void* location, void* dtor) { SEQ___throw(); }

extern "C" void __end__catch(CatchInfo* catchinfo) {
  if (catchinfo->location && catchinfo->dtor) {
    DTORCALL_COMPLETE(catchinfo->dtor, catchinfo->location);
  }
}

static inline exaction_type ExPPC_CurrentAction(const ActionIterator* iter)
{
  if (iter->info.action_pointer == 0) {
    return EXACTION_ENDOFLIST;
  }

  return ((ex_destroylocal*)iter->info.action_pointer)->action & EXACTION_MASK;
}

static inline int ExPPC_IsInSpecification(char* extype, ex_specification* spec)
{
  s32 i, offset;

  for (i = 0; i < spec->specs; i++) {
    if (__throw_catch_compare(extype, spec->spec[i], &offset))
      return 1;
  }

  return 0;
}

static inline void ExPPC_HandleUnexpected(ThrowContext* context, MWExceptionInfo* info, ex_specification* unexp)
{
  CatchInfo* catchinfo;

#pragma exception_terminate

  ExPPC_UnwindStack(context, info, unexp);

  catchinfo           = (CatchInfo*)(context->FP + unexp->cinfo_ref);
  catchinfo->location = context->location;
  catchinfo->typeinfo = context->throwtype;
  catchinfo->dtor     = context->dtor;
  catchinfo->stacktop = unexp;

  ExPPC_LongJump(context, info->TOC, info->current_function + unexp->pcoffset);
}

static void ExPPC_ThrowHandler(ThrowContext* context)
{
  ActionIterator iter;
  MWExceptionInfo info;
  exaction_type action;
  CatchInfo* catchinfo;
  s32 offset;

  ExPPC_FindExceptionRecord(context->returnaddr, &info);

  if (info.exception_record == 0) {
    terminate();
  }

  context->FP = (ET_GetHasFramePtr(info.exception_record->et_field)) ? (char*)context->GPR[31] : context->SP;

  if (context->throwtype == 0) {
    iter.info        = info;
    iter.current_SP  = context->SP;
    iter.current_FP  = context->FP;
    iter.current_R31 = context->GPR[31];

    for (action = ExPPC_CurrentAction(&iter);; action = ExPPC_NextAction(&iter)) {
      switch (action) {
      case EXACTION_ACTIVECATCHBLOCK:
        break;
      case EXACTION_ENDOFLIST:
      case EXACTION_DESTROYLOCAL:
      case EXACTION_DESTROYLOCALCOND:
      case EXACTION_DESTROYLOCALPOINTER:
      case EXACTION_DESTROYLOCALARRAY:
      case EXACTION_DESTROYBASE:
      case EXACTION_DESTROYMEMBER:
      case EXACTION_DESTROYMEMBERCOND:
      case EXACTION_DESTROYMEMBERARRAY:
      case EXACTION_DELETEPOINTER:
      case EXACTION_DELETEPOINTERCOND:
      case EXACTION_CATCHBLOCK:
      case EXACTION_CATCHBLOCK_32:
      case EXACTION_SPECIFICATION:
        continue;
      case EXACTION_TERMINATE:
      default:
        terminate();
      }
      break;
    }

    catchinfo          = (CatchInfo*)(iter.current_FP + ((ex_activecatchblock*)iter.info.action_pointer)->cinfo_ref);
    context->throwtype = (char*)catchinfo->typeinfo;
    context->location  = catchinfo->location;
    context->dtor      = 0;
    context->catchinfo = catchinfo;
  } else {
    context->catchinfo = 0L;
  }

  iter.info        = info;
  iter.current_SP  = context->SP;
  iter.current_FP  = context->FP;
  iter.current_R31 = context->GPR[31];

  for (action = ExPPC_CurrentAction(&iter);; action = ExPPC_NextAction(&iter)) {
    switch (action) {
    case EXACTION_CATCHBLOCK_32:
      if (__throw_catch_compare(context->throwtype, ((ex_catchblock_32*)iter.info.action_pointer)->catch_type, &offset)) {
        break;
      }
      continue;
    case EXACTION_CATCHBLOCK:
      if (__throw_catch_compare(context->throwtype, ((ex_catchblock*)iter.info.action_pointer)->catch_type, &offset)) {
        break;
      }
      continue;
    case EXACTION_SPECIFICATION:
      if (!ExPPC_IsInSpecification(context->throwtype, (ex_specification*)iter.info.action_pointer)) {
        ExPPC_HandleUnexpected(context, &info, (ex_specification*)iter.info.action_pointer);
      }
      continue;
    case EXACTION_ENDOFLIST:
    case EXACTION_DESTROYLOCAL:
    case EXACTION_DESTROYLOCALCOND:
    case EXACTION_DESTROYLOCALPOINTER:
    case EXACTION_DESTROYLOCALARRAY:
    case EXACTION_DESTROYBASE:
    case EXACTION_DESTROYMEMBER:
    case EXACTION_DESTROYMEMBERCOND:
    case EXACTION_DESTROYMEMBERARRAY:
    case EXACTION_DELETEPOINTER:
    case EXACTION_DELETEPOINTERCOND:
    case EXACTION_ACTIVECATCHBLOCK:
      continue;
    case EXACTION_TERMINATE:
    default:
      terminate();
    }
    break;
  }

  if (action == EXACTION_CATCHBLOCK_32) {
    ex_catchblock_32* catchblock_32;
    catchblock_32 = (ex_catchblock_32*)iter.info.action_pointer;

    ExPPC_UnwindStack(context, &info, catchblock_32);

    catchinfo           = (CatchInfo*)(context->FP + catchblock_32->cinfo_ref);
    catchinfo->location = context->location;
    catchinfo->typeinfo = context->throwtype;
    catchinfo->dtor     = context->dtor;

    if (*context->throwtype == '*') {
      catchinfo->sublocation = &catchinfo->pointercopy;
      catchinfo->pointercopy = *(s32*)context->location + offset;
    } else {
      catchinfo->sublocation = (char*)context->location + offset;
    }

    ExPPC_LongJump(context, info.TOC, info.current_function + catchblock_32->catch_pcoffset);
  } else {
    ex_catchblock* catchblock;

    catchblock = (ex_catchblock*)iter.info.action_pointer;
    ExPPC_UnwindStack(context, &info, catchblock);

    catchinfo           = (CatchInfo*)(context->FP + catchblock->cinfo_ref);
    catchinfo->location = context->location;
    catchinfo->typeinfo = context->throwtype;
    catchinfo->dtor     = context->dtor;

    if (*context->throwtype == '*') {
      catchinfo->sublocation = &catchinfo->pointercopy;
      catchinfo->pointercopy = *(s32*)context->location + offset;
    } else {
      catchinfo->sublocation = (char*)context->location + offset;
    }

    ExPPC_LongJump(context, info.TOC, info.current_function + catchblock->catch_pcoffset);
  }
}



extern "C" void __unexpected(CatchInfo* catchinfo)
{
  ex_specification* unexp = (ex_specification*)catchinfo->stacktop;

#pragma exception_magic // allow access to __exception_magic in try/catch blocks

  try {
    unexpected();
  } catch (...) {
    if (ExPPC_IsInSpecification((char*)((CatchInfo*)&__exception_magic)->typeinfo, unexp)) {
      throw;
    }
    if (ExPPC_IsInSpecification("!bad_exception!!", unexp)) {
      throw bad_exception();
    }
    if (ExPPC_IsInSpecification("!std::bad_exception!!", unexp)) {
      throw bad_exception();
    }
  }
  terminate();
}

std::bad_exception::~bad_exception() throw() {}

static inline void ExPPC_DestroyLocal(ThrowContext* context, const ex_destroylocal* ex) { DTORCALL_COMPLETE(ex->dtor, context->FP + ex->local); }

static inline void ExPPC_DestroyLocalCond(ThrowContext* context, const ex_destroylocalcond* ex)
{
  int cond = ex_destroylocalcond_GetRegCond(ex->dlc_field) ? (local_cond_type)context->GPR[ex->cond]
                                                           : *(local_cond_type*)(context->FP + ex->cond);

  if (cond) {
    DTORCALL_COMPLETE(ex->dtor, context->FP + ex->local);
  }
}

static inline void ExPPC_DestroyLocalPointer(ThrowContext* context, const ex_destroylocalpointer* ex)
{
  void* pointer
      = ex_destroylocalpointer_GetRegPointer(ex->dlp_field) ? (void*)context->GPR[ex->pointer] : *(void**)(context->FP + ex->pointer);

  DTORCALL_COMPLETE(ex->dtor, pointer);
}

static inline void ExPPC_DestroyLocalArray(ThrowContext* context, const ex_destroylocalarray* ex)
{
  char* ptr = context->FP + ex->localarray;
  s32 n    = ex->elements;
  s32 size = ex->element_size;

  for (ptr = ptr + size * n; n > 0; n--) {
    ptr -= size;
    DTORCALL_COMPLETE(ex->dtor, ptr);
  }
}

static inline void ExPPC_DestroyBase(ThrowContext* context, const ex_destroymember* ex)
{
  char* objectptr
      = ex_destroymember_GetRegPointer(ex->dm_field) ? (char*)context->GPR[ex->objectptr] : *(char**)(context->FP + ex->objectptr);

  DTORCALL_PARTIAL(ex->dtor, objectptr + ex->offset);
}

static inline void ExPPC_DestroyMember(ThrowContext* context, const ex_destroymember* ex)
{
  char* objectptr
      = ex_destroymember_GetRegPointer(ex->dm_field) ? (char*)context->GPR[ex->objectptr] : *(char**)(context->FP + ex->objectptr);

  DTORCALL_COMPLETE(ex->dtor, objectptr + ex->offset);
}

static inline void ExPPC_DestroyMemberCond(ThrowContext* context, const ex_destroymembercond* ex)
{
  char* objectptr
      = ex_destroymembercond_GetRegPointer(ex->dmc_field) ? (char*)context->GPR[ex->objectptr] : *(char**)(context->FP + ex->objectptr);
  int cond = ex_destroymembercond_GetRegCond(ex->dmc_field) ? (vbase_ctor_arg_type)context->GPR[ex->cond]
                                                            : *(vbase_ctor_arg_type*)(context->FP + ex->cond);

  if (cond) {
    DTORCALL_PARTIAL(ex->dtor, objectptr + ex->offset);
  }
}

static inline void ExPPC_DestroyMemberArray(ThrowContext* context, const ex_destroymemberarray* ex)
{
  char* ptr
      = ex_destroymemberarray_GetRegPointer(ex->dma_field) ? (char*)context->GPR[ex->objectptr] : *(char**)(context->FP + ex->objectptr);
  s32 n    = ex->elements;
  s32 size = ex->element_size;

  ptr += ex->offset;

  for (ptr = ptr + size * n; n > 0; n--) {
    ptr -= size;
    DTORCALL_COMPLETE(ex->dtor, ptr);
  }
}

static inline void ExPPC_DeletePointer(ThrowContext* context, const ex_deletepointer* ex)
{
  char* objectptr
      = ex_deletepointer_GetRegPointer(ex->dp_field) ? (char*)context->GPR[ex->objectptr] : *(char**)(context->FP + ex->objectptr);

  ((DeleteFunc)ex->deletefunc)(objectptr);
}

static inline void ExPPC_DeletePointerCond(ThrowContext* context, const ex_deletepointercond* ex)
{
  char* objectptr
      = ex_deletepointercond_GetRegPointer(ex->dpc_field) ? (char*)context->GPR[ex->objectptr] : *(char**)(context->FP + ex->objectptr);
  int cond = ex_deletepointercond_GetRegCond(ex->dpc_field) ? (local_cond_type)context->GPR[ex->cond]
                                                            : *(local_cond_type*)(context->FP + ex->cond);

  if (cond) {
    ((DeleteFunc)ex->deletefunc)(objectptr);
  }
}

static void ExPPC_UnwindStack(ThrowContext* context, MWExceptionInfo* info, void* catcher)
{
  exaction_type action;

#pragma exception_terminate

  for (;;) {
    if (info->action_pointer == 0) {
      char* return_addr;

      return_addr = ExPPC_PopStackFrame(context, info);
      ExPPC_FindExceptionRecord(return_addr, info);

      if (info->exception_record == 0) {
        terminate();
      }

      context->FP = (ET_GetHasFramePtr(info->exception_record->et_field)) ? (char*)context->GPR[31] : context->SP;
      continue;
    }

    action = ((ex_destroylocal*)info->action_pointer)->action;

    switch (action & EXACTION_MASK) {
    case EXACTION_BRANCH:
      info->action_pointer = ((char*)info->exception_record) + ((ex_branch*)info->action_pointer)->target;
      break;
    case EXACTION_DESTROYLOCAL:
      ExPPC_DestroyLocal(context, (ex_destroylocal*)info->action_pointer);
      info->action_pointer += sizeof(ex_destroylocal);
      break;
    case EXACTION_DESTROYLOCALCOND:
      ExPPC_DestroyLocalCond(context, (ex_destroylocalcond*)info->action_pointer);
      info->action_pointer += sizeof(ex_destroylocalcond);
      break;
    case EXACTION_DESTROYLOCALPOINTER:
      ExPPC_DestroyLocalPointer(context, (ex_destroylocalpointer*)info->action_pointer);
      info->action_pointer += sizeof(ex_destroylocalpointer);
      break;
    case EXACTION_DESTROYLOCALARRAY:
      ExPPC_DestroyLocalArray(context, (ex_destroylocalarray*)info->action_pointer);
      info->action_pointer += sizeof(ex_destroylocalarray);
      break;
    case EXACTION_DESTROYBASE:
      ExPPC_DestroyBase(context, (ex_destroymember*)info->action_pointer);
      info->action_pointer += sizeof(ex_destroymember);
      break;
    case EXACTION_DESTROYMEMBER:
      ExPPC_DestroyMember(context, (ex_destroymember*)info->action_pointer);
      info->action_pointer += sizeof(ex_destroymember);
      break;
    case EXACTION_DESTROYMEMBERCOND:
      ExPPC_DestroyMemberCond(context, (ex_destroymembercond*)info->action_pointer);
      info->action_pointer += sizeof(ex_destroymembercond);
      break;
    case EXACTION_DESTROYMEMBERARRAY:
      ExPPC_DestroyMemberArray(context, (ex_destroymemberarray*)info->action_pointer);
      info->action_pointer += sizeof(ex_destroymemberarray);
      break;
    case EXACTION_DELETEPOINTER:
      ExPPC_DeletePointer(context, (ex_deletepointer*)info->action_pointer);
      info->action_pointer += sizeof(ex_deletepointer);
      break;
    case EXACTION_DELETEPOINTERCOND:
      ExPPC_DeletePointerCond(context, (ex_deletepointercond*)info->action_pointer);
      info->action_pointer += sizeof(ex_deletepointercond);
      break;
    case EXACTION_CATCHBLOCK:
      if (catcher == (void*)info->action_pointer)
        return;
      info->action_pointer += sizeof(ex_catchblock);
      break;
    case EXACTION_CATCHBLOCK_32:
      if (catcher == (void*)info->action_pointer)
        return;
      info->action_pointer += sizeof(ex_catchblock_32);
      break;
    case EXACTION_ACTIVECATCHBLOCK: {
      CatchInfo* catchinfo;

      catchinfo = (CatchInfo*)(context->FP + ((ex_activecatchblock*)info->action_pointer)->cinfo_ref);

      if (catchinfo->dtor) {
        if (context->location == catchinfo->location) {
          context->dtor = catchinfo->dtor;
        } else {
          DTORCALL_COMPLETE(catchinfo->dtor, catchinfo->location);
        }
      }
      info->action_pointer += sizeof(ex_activecatchblock);
    } break;
    case EXACTION_SPECIFICATION:
      if (catcher == (void*)info->action_pointer)
        return;
      info->action_pointer += sizeof(ex_specification) + ((ex_specification*)info->action_pointer)->specs * sizeof(void*);
      break;
    default:
      terminate();
    }

    if (action & EXACTION_ENDBIT)
      info->action_pointer = 0;
  }
}

static char* ExPPC_PopStackFrame(ThrowContext* context, MWExceptionInfo* info)
{
  char *SP, *callers_SP;
  f64* FPR_save_area;
  s32* GPR_save_area;
  int saved_GPRs, saved_FPRs;
  GeckoFPRContext* Vector_save_area;
  int i, j;

  SP         = context->SP;
  callers_SP = *(char**)SP;
  saved_FPRs = ET_GetSavedFPRs(info->exception_record->et_field);

  if (ET_HasElfVector(info->exception_record->et_field)) {
    Vector_save_area = (GeckoFPRContext*)(callers_SP - saved_FPRs * 16);
    FPR_save_area    = (f64*)Vector_save_area;
  } else {
    FPR_save_area = (f64*)(callers_SP - saved_FPRs * 8);
  }

  if (ET_HasElfVector(info->exception_record->et_field)) {
    for (i = 32 - saved_FPRs, j = 0; i < 32; ++i, ++j) {
      context->FPR[i].v.f[0] = Vector_save_area[j].v.f[0];
      context->FPR[i].v.f[1] = Vector_save_area[j].v.f[1];
      context->FPR[i].d      = Vector_save_area[j].d;
    }
  } else {
    for (i = 32 - saved_FPRs, j = 0; i < 32; ++i, ++j) {
      context->FPR[i].d = FPR_save_area[j];
    }
  }

  saved_GPRs    = ET_GetSavedGPRs(info->exception_record->et_field);
  GPR_save_area = (s32*)FPR_save_area;
  GPR_save_area -= saved_GPRs;

  for (i = 32 - saved_GPRs, j = 0; i < 32; ++i, ++j) {
    context->GPR[i] = GPR_save_area[j];
  }

  context->SP = callers_SP;
  return *(char**)(callers_SP + RETURN_ADDRESS);
}

static inline s32 ExPPC_PopR31(char* SP, MWExceptionInfo* info)
{
  f64* FPR_save_area;
  s32* GPR_save_area;
  int saved_GPRs, saved_FPRs;

  saved_FPRs    = ET_GetSavedFPRs(info->exception_record->et_field);
  FPR_save_area = (f64*)(SP - saved_FPRs * 8);
  saved_GPRs    = ET_GetSavedGPRs(info->exception_record->et_field);
  GPR_save_area = (s32*)FPR_save_area;

  return GPR_save_area[-1];
}

static exaction_type ExPPC_NextAction(ActionIterator* iter)
{
  exaction_type action;

  for (;;) {
    if (iter->info.action_pointer == 0 || ((action = ((ex_destroylocal*)iter->info.action_pointer)->action) & EXACTION_ENDBIT) != 0) {
      char *return_addr, *callers_SP;

      callers_SP = *(char**)iter->current_SP;

      if (ET_GetSavedGPRs(iter->info.exception_record->et_field)) {
        iter->current_R31 = ExPPC_PopR31(callers_SP, &iter->info);
      }

      return_addr = *(char**)(callers_SP + RETURN_ADDRESS);

      ExPPC_FindExceptionRecord(return_addr, &iter->info);

      if (iter->info.exception_record == 0) {
        terminate();
      }

      iter->current_SP = callers_SP;
      iter->current_FP = (ET_GetHasFramePtr(iter->info.exception_record->et_field)) ? (char*)iter->current_R31 : iter->current_SP;

      if (iter->info.action_pointer == 0)
        continue;
    } else {
      switch (action) {
      case EXACTION_DESTROYLOCAL:
        iter->info.action_pointer += sizeof(ex_destroylocal);
        break;
      case EXACTION_DESTROYLOCALCOND:
        iter->info.action_pointer += sizeof(ex_destroylocalcond);
        break;
      case EXACTION_DESTROYLOCALPOINTER:
        iter->info.action_pointer += sizeof(ex_destroylocalpointer);
        break;
      case EXACTION_DESTROYLOCALARRAY:
        iter->info.action_pointer += sizeof(ex_destroylocalarray);
        break;
      case EXACTION_DESTROYBASE:
      case EXACTION_DESTROYMEMBER:
        iter->info.action_pointer += sizeof(ex_destroymember);
        break;
      case EXACTION_DESTROYMEMBERCOND:
        iter->info.action_pointer += sizeof(ex_destroymembercond);
        break;
      case EXACTION_DESTROYMEMBERARRAY:
        iter->info.action_pointer += sizeof(ex_destroymemberarray);
        break;
      case EXACTION_DELETEPOINTER:
        iter->info.action_pointer += sizeof(ex_deletepointer);
        break;
      case EXACTION_DELETEPOINTERCOND:
        iter->info.action_pointer += sizeof(ex_deletepointercond);
        break;
      case EXACTION_CATCHBLOCK:
        iter->info.action_pointer += sizeof(ex_catchblock);
        break;
      case EXACTION_CATCHBLOCK_32:
        iter->info.action_pointer += sizeof(ex_catchblock_32);
        break;
      case EXACTION_ACTIVECATCHBLOCK:
        iter->info.action_pointer += sizeof(ex_activecatchblock);
        break;
      case EXACTION_SPECIFICATION:
        iter->info.action_pointer
            += sizeof(ex_specification) + ((ex_specification*)iter->info.action_pointer)->specs * sizeof(void*);
        break;
      default:
        terminate();
      }
    }

    action = ((ex_destroylocal*)iter->info.action_pointer)->action & EXACTION_MASK;

    if (action == EXACTION_BRANCH) {
      iter->info.action_pointer = ((char*)iter->info.exception_record) + ((ex_branch*)iter->info.action_pointer)->target;
      action                    = ((ex_destroylocal*)iter->info.action_pointer)->action & EXACTION_MASK;
    }
    return action;
  }
}

static inline int ExPPC_FindExceptionFragment(char* returnaddr, FragmentInfo* frag)
{
  ProcessInfo* f;
  int i;
  __eti_init_info* eti_info;

  for (i = 0, f = fragmentinfo; i < 1; ++i, ++f) {
    if (f->active) {
      eti_info = f->exception_info;
      while (1) {
        if (eti_info->code_size == 0)
          break;
        if (returnaddr >= eti_info->code_start && returnaddr < (char*)eti_info->code_start + eti_info->code_size) {
          frag->exception_start = (ExceptionTableIndex*)eti_info->eti_start;
          frag->exception_end   = (ExceptionTableIndex*)eti_info->eti_end;
          frag->code_start      = 0;
          frag->code_end        = 0;
          frag->data_start      = 0;
          frag->data_end        = 0;
          frag->TOC             = f->TOC;
          frag->active          = f->active;
          return 1;
        }
        eti_info++;
      }
    }
  }

  return 0;
}

static void ExPPC_FindExceptionRecord(char* returnaddr, MWExceptionInfo* info)
{
  FragmentInfo* fragment;
  FragmentInfo frag;
  ExceptionTableIndex *exceptionindex, *p;
  u32 returnoffset;
  s32 i, m, n;

  info->exception_record = 0;
  info->action_pointer   = 0;

  if ((ExPPC_FindExceptionFragment(returnaddr, &frag)) == 0)
    return;
  fragment = &frag;

  info->code_section = fragment->code_start;
  info->data_section = fragment->data_start;
  info->TOC          = fragment->TOC;

  returnoffset   = returnaddr - fragment->code_start;
  exceptionindex = fragment->exception_start;
  for (i = 0, n = fragment->exception_end - fragment->exception_start;;) {
    if (i > n)
      return;
    p = &exceptionindex[m = (i + n) / 2];

    if (returnoffset < p->functionoffset) {
      n = m - 1;
    } else if (returnoffset > p->functionoffset + ETI_GetFunctionSize(p->eti_field)) {
      i = m + 1;
    } else
      break;
  }
  info->current_function = fragment->code_start + p->functionoffset;
  info->exception_record = ETI_GetDirectStore(p->eti_field) ? (ExceptionTableSmall*)(&p->exceptionoffset)
                                                            : (ExceptionTableSmall*)(fragment->data_start + p->exceptionoffset);

  returnoffset -= p->functionoffset;

  if (ET_IsLargeTable(info->exception_record->et_field)) {
    ExceptionTableLarge* etl = (ExceptionTableLarge*)info->exception_record;
    ExceptionRangeLarge* erl;

    for (erl = etl->ranges; erl->start != 0; erl++) {
      u32 range_end = erl->start + (erl->size * 4);

      if (erl->start <= returnoffset && range_end >= returnoffset) {
        info->action_pointer = (char*)etl + erl->action;
        break;
      }
    }
  } else {
    ExceptionTableSmall* ets = (ExceptionTableSmall*)info->exception_record;
    ExceptionRangeSmall* ers;

    for (ers = ets->ranges; ers->start != 0; ers++) {
      if (ers->start <= returnoffset && ers->end >= returnoffset) {
        info->action_pointer = (char*)ets + ers->action;
        break;
      }
    }
  }
}

extern "C" void __unregister_fragment(int fragmentId) {
  ProcessInfo* f;
  if (fragmentId >= 0 && fragmentId < 1) {
    f = &fragmentinfo[fragmentId];
    f->exception_info = 0;
    f->TOC = 0;
    f->active = 0;
  }
}

extern "C" int __register_fragment(struct __eti_init_info* info, char* TOC) {
  ProcessInfo* f;
  int i;
  for (i = 0, f = fragmentinfo; i < 1; ++i, ++f) {
    if (f->active == 0) {
      f->exception_info = info;
      f->TOC = TOC;
      f->active = 1;
      return i;
    }
  }
  return -1;
}


