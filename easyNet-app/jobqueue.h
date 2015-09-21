#ifndef JOBQUEUE_H
#define JOBQUEUE_H



#include <QQueue>
#include <QMutex>
#include <QDebug>

template <class Job>
class JobQueue
{

public:
    JobQueue();
    void tryRun(Job *job);
    void freeToRun();
    bool isReady();
    int jobsInQueue();
    void pause();
    void stop();
    bool isPaused() const {return paused;}
    //bool isStopped() const {return stopped;}


protected:
    void tryRunNext();
    QQueue<Job*> queue;
    QMutex mutex;
    bool paused;
    //bool stopped;
    Job* currentJob;

};

// http://stackoverflow.com/questions/8752837/undefined-reference-to-template-class-constructor

template <class Job>
JobQueue<Job>::JobQueue()
    : paused(false), currentJob(nullptr)
{
}

template <class Job>
void JobQueue<Job, DerivedQueue>::tryRun(Job *job)
{
    queue.enqueue(job);
    tryRunNext();
}

template <class Job>
void JobQueue<Job>::freeToRun()
{
    if (currentJob)
        mutex.unlock();
    currentJob = nullptr;
    tryRunNext();
}

template <class Job>
void JobQueue<Job>::tryRunNext()
{
    if(!queue.isEmpty() && !paused && mutex.tryLock())
    {
        currentJob = queue.dequeue();
        currentJob->run();
    }
}


template <class Job>
bool JobQueue<Job>::isReady()
{
    return !currentJob;
}


template <class Job>
int JobQueue<Job>::jobsInQueue()
{
    return queue.size();
}

template <class Job>
void JobQueue<Job>::pause()
{
    if (!queue.isEmpty())
    {
        paused = !paused;
        if (!paused)
            tryRunNext();
    }
}

template <class Job>
void JobQueue<Job>::stop()
{
//    DerivedQueue *derivedQueue = (DerivedQueue*)this;
//    derivedQueue->reset();
//    freeToRun();
}

#endif // JOBQUEUE_H

