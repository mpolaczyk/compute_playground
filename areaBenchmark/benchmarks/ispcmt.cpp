
#include "benchmark/benchmark.h"
#include "benchmarks/getAreaMT.h"
#include "tools/cpuid.h"
#include "simd.h"


#define NOMINMAX
#include <windows.h>

#define ISPC_USE_CONCRT
#include <concrt.h>

using namespace Concurrency;

// ispc expects these functions to have C linkage / not be mangled
extern "C"
{
  void ISPCLaunch(void** handlePtr, void* f, void* data, int countx, int county, int countz);
  void* ISPCAlloc(void** handlePtr, int64_t size, int32_t alignment);
  void ISPCSync(void* handle);
}

// Signature of ispc-generated 'task' functions
typedef void (*TaskFuncType)(void* data, int threadIndex, int threadCount, int taskIndex, int taskCount, int taskIndex0,
  int taskIndex1, int taskIndex2, int taskCount0, int taskCount1, int taskCount2);

// Small structure used to hold the data for each task
struct TaskInfo {
  TaskFuncType func;
  void* data;
  int taskIndex;
  int taskCount3d[3];
#if defined(ISPC_USE_CONCRT)
  event taskEvent;
#endif
  int taskCount() const { return taskCount3d[0] * taskCount3d[1] * taskCount3d[2]; }
  int taskIndex0() const { return taskIndex % taskCount3d[0]; }
  int taskIndex1() const { return (taskIndex / taskCount3d[0]) % taskCount3d[1]; }
  int taskIndex2() const { return taskIndex / (taskCount3d[0] * taskCount3d[1]); }
  int taskCount0() const { return taskCount3d[0]; }
  int taskCount1() const { return taskCount3d[1]; }
  int taskCount2() const { return taskCount3d[2]; }
  TaskInfo() = default;
};

#define LOG_TASK_QUEUE_CHUNK_SIZE 14
#define MAX_TASK_QUEUE_CHUNKS 128
#define TASK_QUEUE_CHUNK_SIZE (1 << LOG_TASK_QUEUE_CHUNK_SIZE)

#define MAX_LAUNCHED_TASKS (MAX_TASK_QUEUE_CHUNKS * TASK_QUEUE_CHUNK_SIZE)

#define NUM_MEM_BUFFERS 16

class TaskGroupBase {
public:
  void Reset();

  int AllocTaskInfo(int count);
  TaskInfo* GetTaskInfo(int index);

  void Launch(int baseIndex, int count);
  void Sync();

  void* AllocMemory(int64_t size, int32_t alignment);

  TaskGroupBase();
  ~TaskGroupBase();

  int nextTaskInfoIndex;

private:
  /* We allocate blocks of TASK_QUEUE_CHUNK_SIZE TaskInfo structures as
     needed by the calling function.  We hold up to MAX_TASK_QUEUE_CHUNKS
     of these (and then exit at runtime if more than this many tasks are
     launched.)
   */
  TaskInfo* taskInfo[MAX_TASK_QUEUE_CHUNKS];

  /* We also allocate chunks of memory to service ISPCAlloc() calls.  The
     memBuffers[] array holds pointers to this memory.  The first element
     of this array is initialized to point to mem and then any subsequent
     elements required are initialized with dynamic allocation.
   */
  int curMemBuffer, curMemBufferOffset;
  int memBufferSize[NUM_MEM_BUFFERS];
  char* memBuffers[NUM_MEM_BUFFERS];
  char mem[256];
};

inline TaskGroupBase::TaskGroupBase() {
  nextTaskInfoIndex = 0;

  curMemBuffer = 0;
  curMemBufferOffset = 0;
  memBuffers[0] = mem;
  memBufferSize[0] = sizeof(mem) / sizeof(mem[0]);
  for (int i = 1; i < NUM_MEM_BUFFERS; ++i) {
    memBuffers[i] = NULL;
    memBufferSize[i] = 0;
  }

  for (int i = 0; i < MAX_TASK_QUEUE_CHUNKS; ++i)
    taskInfo[i] = NULL;
}

inline TaskGroupBase::~TaskGroupBase() {
  // Note: don't delete memBuffers[0], since it points to the start of
  // the "mem" member!
  for (int i = 1; i < NUM_MEM_BUFFERS; ++i)
    delete[](memBuffers[i]);
}

inline void TaskGroupBase::Reset() {
  nextTaskInfoIndex = 0;
  curMemBuffer = 0;
  curMemBufferOffset = 0;
}

inline int TaskGroupBase::AllocTaskInfo(int count) {
  int ret = nextTaskInfoIndex;
  nextTaskInfoIndex += count;
  return ret;
}

inline TaskInfo* TaskGroupBase::GetTaskInfo(int index) {
  int chunk = (index >> LOG_TASK_QUEUE_CHUNK_SIZE);
  int offset = index & (TASK_QUEUE_CHUNK_SIZE - 1);

  if (chunk == MAX_TASK_QUEUE_CHUNKS) {
    fprintf(stderr,
      "A total of %d tasks have been launched from the "
      "current function--the simple built-in task system can handle "
      "no more. You can increase the values of TASK_QUEUE_CHUNK_SIZE "
      "and LOG_TASK_QUEUE_CHUNK_SIZE to work around this limitation.  "
      "Sorry!  Exiting.\n",
      index);
    exit(1);
  }

  if (taskInfo[chunk] == NULL)
    taskInfo[chunk] = new TaskInfo[TASK_QUEUE_CHUNK_SIZE];
  return &taskInfo[chunk][offset];
}

inline void* TaskGroupBase::AllocMemory(int64_t size, int32_t alignment) {
  char* basePtr = memBuffers[curMemBuffer];
  intptr_t iptr = (intptr_t)(basePtr + curMemBufferOffset);
  iptr = (iptr + (alignment - 1)) & ~(alignment - 1);

  int newOffset = int(iptr - (intptr_t)basePtr + size);
  if (newOffset < memBufferSize[curMemBuffer]) {
    curMemBufferOffset = newOffset;
    return (char*)iptr;
  }

  ++curMemBuffer;
  curMemBufferOffset = 0;
  assert(curMemBuffer < NUM_MEM_BUFFERS);

  int allocSize = 1 << (12 + curMemBuffer);
  allocSize = std::max(int(size + alignment), allocSize);
  char* newBuf = new char[allocSize];
  memBufferSize[curMemBuffer] = allocSize;
  memBuffers[curMemBuffer] = newBuf;
  return AllocMemory(size, alignment);
}

static void* lAtomicCompareAndSwapPointer(void** v, void* newValue, void* oldValue) 
{
  return InterlockedCompareExchangePointer(v, newValue, oldValue);
}

static int32_t lAtomicCompareAndSwap32(volatile int32_t* v, int32_t newValue, int32_t oldValue) 
{
  return InterlockedCompareExchange((volatile LONG*)v, newValue, oldValue);
}

static inline int32_t lAtomicAdd(volatile int32_t* v, int32_t delta) 
{
  return InterlockedExchangeAdd((volatile LONG*)v, delta) + delta;
}

static void InitTaskSystem() {
  // No initialization needed
}

static void __cdecl lRunTask(LPVOID param) {
  TaskInfo* ti = (TaskInfo*)param;

  // Actually run the task.
  // FIXME: like the GCD implementation for OS X, this is passing bogus
  // values for the threadIndex and threadCount builtins, which in turn
  // will cause bugs in code that uses those.
  int threadIndex = 0;
  int threadCount = 1;
  ti->func(ti->data, threadIndex, threadCount, ti->taskIndex, ti->taskCount(), ti->taskIndex0(), ti->taskIndex1(),
    ti->taskIndex2(), ti->taskCount0(), ti->taskCount1(), ti->taskCount2());

  // Signal the event that this task is done
  ti->taskEvent.set();
}

inline void TaskGroupBase::Launch(int baseIndex, int count) {
  for (int i = 0; i < count; ++i)
    CurrentScheduler::ScheduleTask(lRunTask, GetTaskInfo(baseIndex + i));
}

inline void TaskGroupBase::Sync() {
  for (int i = 0; i < nextTaskInfoIndex; ++i) {
    TaskInfo* ti = GetTaskInfo(i);
    ti->taskEvent.wait();
    ti->taskEvent.reset();
  }
}

#define MAX_FREE_TASK_GROUPS 64
static TaskGroupBase* freeTaskGroups[MAX_FREE_TASK_GROUPS];

static inline TaskGroupBase* AllocTaskGroup() {
  for (int i = 0; i < MAX_FREE_TASK_GROUPS; ++i) {
    TaskGroupBase* tg = freeTaskGroups[i];
    if (tg != NULL) {
      void* ptr = lAtomicCompareAndSwapPointer((void**)(&freeTaskGroups[i]), NULL, tg);
      if (ptr != NULL) {
        return (TaskGroupBase*)ptr;
      }
    }
  }

  return new TaskGroupBase;
}

static inline void FreeTaskGroup(TaskGroupBase* tg) {
  tg->Reset();

  for (int i = 0; i < MAX_FREE_TASK_GROUPS; ++i) {
    if (freeTaskGroups[i] == NULL) {
      void* ptr = lAtomicCompareAndSwapPointer((void**)&freeTaskGroups[i], tg, NULL);
      if (ptr == NULL)
        return;
    }
  }

  delete tg;
}


void ISPCLaunch(void** taskGroupPtr, void* func, void* data, int count0, int count1, int count2) {
  const int count = count0 * count1 * count2;
  TaskGroupBase* taskGroup;
  if (*taskGroupPtr == NULL) {
    InitTaskSystem();
    taskGroup = AllocTaskGroup();
    *taskGroupPtr = taskGroup;
  }
  else
    taskGroup = (TaskGroupBase*)(*taskGroupPtr);

  int baseIndex = taskGroup->AllocTaskInfo(count);
  for (int i = 0; i < count; ++i) {
    TaskInfo* ti = taskGroup->GetTaskInfo(baseIndex + i);
    ti->func = (TaskFuncType)func;
    ti->data = data;
    ti->taskIndex = i;
    ti->taskCount3d[0] = count0;
    ti->taskCount3d[1] = count1;
    ti->taskCount3d[2] = count2;
  }
  taskGroup->Launch(baseIndex, count);
}

void ISPCSync(void* h) {
  TaskGroupBase* taskGroup = (TaskGroupBase*)h;
  if (taskGroup != NULL) {
    taskGroup->Sync();
    FreeTaskGroup(taskGroup);
  }
}

void* ISPCAlloc(void** taskGroupPtr, int64_t size, int32_t alignment) {
  TaskGroupBase* taskGroup;
  if (*taskGroupPtr == NULL) {
    InitTaskSystem();
    taskGroup = AllocTaskGroup();
    *taskGroupPtr = taskGroup;
  }
  else
    taskGroup = (TaskGroupBase*)(*taskGroupPtr);

  return taskGroup->AllocMemory(size, alignment);
}

static SIMD::shapes shapesFactoryISPC(int numShapes)
{
  SIMD::shapes ans = SIMD::shapes(numShapes);

  for (int i = 0; i < numShapes; i += 4)
  {
    ans.initAndRandomize(i, SIMD::shapeType::circle);
    ans.initAndRandomize(i + 1, SIMD::shapeType::rectangle);
    ans.initAndRandomize(i + 2, SIMD::shapeType::square);
    ans.initAndRandomize(i + 3, SIMD::shapeType::triangle);
  }
  return ans;
}


void BM_getAreaISPCMT(benchmark::State& state)
{
  if (!InstructionSet::AVX()) return;

  int numShapes = static_cast<int>(state.range(0));
  auto shapes = shapesFactoryISPC(numShapes);

  for (auto _ : state)
  {
    volatile float sum = 0.0f;      // removing volatile causes the compiler to optimize it away, together with a call to areaAVX
    benchmark::DoNotOptimize(sum);
    sum = ispc::getAreaMT(shapes.a.data(), shapes.b.data(), shapes.coeff.data(), numShapes, 256);

  }
  state.SetComplexityN(state.range(0));

}