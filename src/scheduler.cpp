
#include "scheduler.h"

Scheduler::Scheduler(unsigned numprocs)
{
    processors.resize(numprocs, NoProc);
    availProcs.resize(numprocs);
    for(unsigned i = 0; i < numprocs; ++i)
        availProcs[i] = i;

    lastJobId = 0;
    usedJobIds.insert(NoJob);       // 'NoJob' is a reserved Job ID, it can never be assigned
    needProcAssign = false;
}


void Scheduler::addJob(const JobInfo& jobinfo)
{
    if(jobinfo.numTicks <= 0)       // no ticks in this job -- it's immediately complete
        return;

    if(jobinfo.numProcs <= 0)       // this job uses no processors -- this is nonsense
        throw SchedulerException("Attempted to add a job with no processors");


    ScheduledJob job;
    job.info = jobinfo;
    job.ticksRemaining = jobinfo.numTicks;

    job.procsUsed.reset(new procid_t[jobinfo.numProcs]);
    for(unsigned i = 0; i < jobinfo.numProcs; ++i)
        job.procsUsed[i] = NoProc;
    
    job.id = getUniqueJobId();
    usedJobIds.insert(job.id);

    putJobInWaitQueue( std::move(job) );

    // if we have the available processors to run this job, flag that we need to assign procs
    if(availProcs.size() >= jobinfo.numProcs)
        needProcAssign = true;
}


jobid_t Scheduler::getUniqueJobId()
{
    do 
    {
        ++lastJobId;
    }while( isJobIdInUse(lastJobId) );

    return lastJobId;
}

bool Scheduler::isJobIdInUse(jobid_t id) const
{
    return usedJobIds.find(id) != usedJobIds.end();
}


void Scheduler::putJobInWaitQueue( ScheduledJob&& job )
{
    waitQueue.insert( queue_t::value_type( job.ticksRemaining, std::move(job) ) );
}

//////////////////////////////////////////////

void Scheduler::tick()
{
    runActiveJobs();

    if(needProcAssign)
        assignProcs();
}

void Scheduler::runActiveJobs()
{
    auto i = activeJobs.begin();
    while(i != activeJobs.end())
    {
        i->ticksRemaining--;
        if(i->ticksRemaining == 0)      // this job is complete!
        {
            freeProcessors(*i);     // free the processors used by this job
            usedJobIds.erase(i->id);
            i = activeJobs.erase(i);
        }
        else
            ++i;
    }
}

void Scheduler::freeProcessors(const ScheduledJob& job)
{
    for(unsigned i = 0; i < job.info.numProcs; ++i)
    {
        auto prid = job.procsUsed[i];
        if(prid == NoProc)
            throw SchedulerException("Internal Error:  freeProcessors called on a job with unassigned procs");

        availProcs.push_back(prid);
        processors[prid] = NoJob;
    }

    needProcAssign = true;
}

void Scheduler::assignProcs()
{
    auto i = waitQueue.begin();

    //  keep looping as long as we have waiting jobs and available processors
    while(i != waitQueue.end() && !availProcs.empty())
    {

    }
}