#ifndef JOBQUEUE_H
#define JOBQUEUE_H

#include <QQueue>
#include <QMutex>
#include <QDebug>

// http://stackoverflow.com/questions/14466620/c-template-specialization-calling-methods-on-types-that-could-be-pointers-or/14466705
template<typename T>
T * ptr(T & obj) { return &obj; } //turn reference into pointer!

template<typename T>
T * ptr(T * obj) { return obj; } //obj is already pointer, return it!


template <class Job>
class JobQueue
{

public:
    JobQueue();
    void tryRun(Job job);
    void tryRun(QList<Job> jobs);
    void freeToRun();
    bool isReady();
    int jobsInQueue();
    void pause();
    void stop();
    bool isPaused() const {return paused;}
    //bool isStopped() const {return stopped;}


protected:
    void tryRunNext();
    QQueue<Job> queue;
    QMutex mutex;
    bool busy;
    bool paused;
    //bool stopped;
//    Job* currentJob;

};

// http://stackoverflow.com/questions/8752837/undefined-reference-to-template-class-constructor

template <class Job>
JobQueue<Job>::JobQueue()
    : paused(false), busy(false)
{
}

template <class Job>
void JobQueue<Job>::tryRun(Job job)
{
    queue.enqueue(job);
    tryRunNext();
}


template <class Job>
void JobQueue<Job>::tryRun(QList<Job> jobs)
{
    foreach(Job job, jobs)
        queue.enqueue(job);
    tryRunNext();
}

template <class Job>
void JobQueue<Job>::freeToRun()
{
    if (busy)
        mutex.unlock();
    busy = false;
    tryRunNext();
}

template <class Job>
void JobQueue<Job>::tryRunNext()
{
    if(!queue.isEmpty() && !paused && mutex.tryLock())
    {
        busy = true;
        ptr(queue.dequeue())->run();
    }
}


template <class Job>
bool JobQueue<Job>::isReady()
{
    return !busy;
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

