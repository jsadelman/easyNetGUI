#ifndef JOBQUEUE_H
#define JOBQUEUE_H

#include <QQueue>
#include <QMutex>
#include <QDebug>

template <class JOB, class DerivedQueue>
class JobQueue
{
    // http://www.codeproject.com/Articles/268849/An-Idiots-Guide-to-Cplusplus-Templates-Part#Virtuals
    // http://en.wikipedia.org/wiki/Curiously_recurring_template_pattern

public:
    JobQueue();
    void tryRun(JOB *job);
    void freeToRun();
    bool isReady();
    int jobsInQueue();
    void pause();
    void stop();
    bool isPaused() const {return paused;}
    //bool isStopped() const {return stopped;}


protected:
    void tryRunNext();
    QQueue<JOB*> queue;
    QMutex mutex;
    bool paused;
    //bool stopped;
    JOB* currentJob;

};

// http://stackoverflow.com/questions/8752837/undefined-reference-to-template-class-constructor

template <class JOB, class DerivedQueue>
JobQueue<JOB, DerivedQueue>::JobQueue()
    : paused(false), currentJob(nullptr)
{
}

template <class JOB, class DerivedQueue>
void JobQueue<JOB, DerivedQueue>::tryRun(JOB *job)
{

    queue.enqueue(job);
    tryRunNext();

}

template <class JOB, class DerivedQueue>
void JobQueue<JOB, DerivedQueue>::freeToRun()
{
    if (currentJob)
        mutex.unlock();
    currentJob = nullptr;
    tryRunNext();
}

template <class JOB, class DerivedQueue>
void JobQueue<JOB, DerivedQueue>::tryRunNext()
{
    if(!queue.isEmpty() && !paused && mutex.tryLock())
    {
        DerivedQueue *derivedQueue = (DerivedQueue*)this;
        currentJob = queue.dequeue();
        derivedQueue->run(currentJob);
    }
}


template <class JOB, class DerivedQueue>
bool JobQueue<JOB, DerivedQueue>::isReady()
{
    return !currentJob;
}


//template <class JOB, class DerivedQueue>
//bool JobQueue<JOB, DerivedQueue>::isReady()
//{
//    // this implementation is incorrect, maybe use a post return guard as in
//    // http://stackoverflow.com/questions/8763182/execution-of-code-in-a-function-after-the-return-statement-has-been-accessed-in

//    if (mutex.tryLock())
//    {
//        mutex.unlock();
//        // here mutex could be locked
//        return true;
//    }
//    else
//        return false;
//}

template <class JOB, class DerivedQueue>
int JobQueue<JOB, DerivedQueue>::jobsInQueue()
{
    return queue.size();
}

template <class JOB, class DerivedQueue>
void JobQueue<JOB, DerivedQueue>::pause()
{
    if (!queue.isEmpty())
    {
        paused = !paused;
        if (!paused)
            tryRunNext();
    }
}

template <class JOB, class DerivedQueue>
void JobQueue<JOB, DerivedQueue>::stop()
{
    DerivedQueue *derivedQueue = (DerivedQueue*)this;
    derivedQueue->reset();
    freeToRun();
}


#endif // JOBQUEUE_H
