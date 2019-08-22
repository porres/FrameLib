
#ifndef FRAMELIB_PROCESSINGQUEUE_H
#define FRAMELIB_PROCESSINGQUEUE_H

#include "FrameLib_Types.h"
#include "FrameLib_Errors.h"
#include "FrameLib_Memory.h"
#include "FrameLib_Threading.h"

#include "FrameLib_LockFree.h"

#include <chrono>
#include <vector>

// Forward Declarations

class FrameLib_Global;
class FrameLib_DSP;

/**
 
 @defgroup ProcessingQueue Processing Queue
 
 */

/**
 
 @class FrameLib_ProcessingQueue
 
 @ingroup ProcessingQueue
 
 @brief a minimal processing queue that is used to non-recursively process FrameLIB_DSP objects in a network.
 
 */

class FrameLib_ProcessingQueue
{
    
public:
    
    using Queue = FrameLib_LockFreeStack<FrameLib_DSP>;
    using Node = Queue::Node;
    using NodeList = Queue::NodeList;

    /**
     
     @class IntervalSecondsClock
     
     @brief a clock for measuring time intervals in seconds.
     
     */

    class IntervalSecondsClock
    {
        
    public:
        
        void start() { mStartTime = getTime(); }
        long long elapsed() { return std::chrono::duration_cast<std::chrono::seconds>(getTime() - mStartTime).count(); }
        
    private:
        
        std::chrono::steady_clock::time_point getTime() { return mClock.now(); }

        std::chrono::steady_clock mClock;
        std::chrono::steady_clock::time_point mStartTime;
    };
    
    static const int sProcessPerTimeCheck = 200;
    static const int sMaxTime = 5;
    
    /**
     
     @class WorkerThreads
     
     @brief a set of worker threads for processing.
     
     */
    
    class WorkerThreads : public FrameLib_TriggerableThreadSet
    {
        
    public:
        
        WorkerThreads(FrameLib_ProcessingQueue *queue)
        : FrameLib_TriggerableThreadSet(FrameLib_Thread::kAudioPriority, FrameLib_Thread::maxThreads() - 1), mQueue(queue)
        {}
        
    private:
        
        void doTask(unsigned int index) override { mQueue->serviceQueue(index + 1); }

        FrameLib_ProcessingQueue *mQueue;
    };
    
public:
    
    // Constructor / Destructor
    
    FrameLib_ProcessingQueue(FrameLib_Global& global);
    ~FrameLib_ProcessingQueue();
    
    // Non-copyable
    
    FrameLib_ProcessingQueue(const FrameLib_ProcessingQueue&) = delete;
    FrameLib_ProcessingQueue& operator=(const FrameLib_ProcessingQueue&) = delete;
    
    // Start and add items to the queue
    
    void start(NodeList &list);
    void start(FrameLib_DSP *object);
    void add(NodeList &list, FrameLib_DSP *addedBy);
    void add(FrameLib_DSP *object, FrameLib_DSP *addedBy);
    
    // Additional functionality
    
    void reset() { mTimedOut = false; }
    bool isTimedOut() { return mTimedOut; }
    void setMultithreading(bool multihread) { mMultithread = multihread; }
    
private:
    
    void wakeWorkers(bool countThisThread);
    void serviceQueue(int32_t index);
    
    WorkerThreads mWorkers;
    FrameLib_OwnedList<FrameLib_FreeBlocks> mFreeBlocks;

    Queue mQueue;
    
    std::atomic<int32_t> mNumItems;
    std::atomic<int32_t> mNumWorkersActive;
    
    bool mMultithread;
    bool mTimedOut;
    IntervalSecondsClock mClock;
    
    FrameLib_ErrorReporter& mErrorReporter;
};

class FrameLib_AudioQueue : private FrameLib_ProcessingQueue::NodeList
{
    friend class FrameLib_DSP;
    
public:
    
    FrameLib_AudioQueue() : mUser(nullptr) {}
    ~FrameLib_AudioQueue(); 
    
private:
    
    void setUser(FrameLib_DSP *object) { mUser = object; }
    
    FrameLib_DSP *mUser;
};

#endif /* FRAMELIB_PROCESSINGQUEUE_H */