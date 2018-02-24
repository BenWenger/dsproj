
#include <iomanip>
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


bool Scheduler::addJob(const JobInfo& jobinfo)
{
    if(jobinfo.numTicks <= 0)       // no ticks in this job -- it's immediately complete
        return false;
    if(jobinfo.numProcs <= 0)       // this job uses no processors -- this is nonsense
        return false;
    if(jobinfo.numProcs > processors.size())    // requires more processors than we have
        return false;


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

    return true;
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
    if(needProcAssign)
        assignProcs();

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
        // can we service this job?
        if(availProcs.size() >= i->second.info.numProcs)
        {
            allocateProcessors(i->second);
            activeJobs.push_back( std::move(i->second) );
            i = waitQueue.erase(i);
        }
        else
            ++i;
    }

    needProcAssign = false;
}

void Scheduler::allocateProcessors(ScheduledJob& job)
{
    for(unsigned i = 0; i < job.info.numProcs; ++i)
    {
        auto prid = availProcs.back();

        job.procsUsed[i] = prid;
        processors[prid] = job.id;

        availProcs.pop_back();
    }
}


/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

namespace
{
    std::string getProcString(unsigned count, const procid_t* ids)
    {
        std::string out;
        for(unsigned i = 0; i < count; ++i)
        {
            if(i)       out += ", ";
            out += std::to_string(ids[i]);
        }
        return out;
    }
}

void Scheduler::printActiveJobs(std::ostream& s) const
{
    using namespace std;

    s << "Active Jobs:\n";
    s << "Job Id  | Job Description         | Ticks Left |  Procs Used\n";
    s << "----------------------------------------------------------------\n";

    if(activeJobs.empty())
    {
        s << "(No Active Jobs)\n";
    }
    else
    {
        for(auto& i : activeJobs)
        {
            s << left << setw(8) << setfill(' ') << i.id << "| ";
            s << left << setw(24) << setfill(' ') << i.info.description << "| ";
            s << left << setw(11) << setfill(' ') << i.ticksRemaining << "| ";
            s << getProcString(i.info.numProcs, i.procsUsed.get()) << '\n';
        }
    }
}

/*

    typedef std::multimap<unsigned, ScheduledJob>   queue_t;
    queue_t                     waitQueue;
    std::list<ScheduledJob>     activeJobs;
    */

void Scheduler::printWaitQueue(std::ostream& s) const
{
    using namespace std;
    
    s << "Wait Queue (top is next in queue):\n";
    s << "Job Id  | Job Description         | Ticks Left |  Num Procs Needed\n";
    s << "----------------------------------------------------------------\n";
    
    if(waitQueue.empty())
    {
        s << "(Wait Queue is empty)\n";
    }
    else
    {
        for(auto& item : waitQueue)
        {
            auto& i = item.second;
            s << left << setw(8) << setfill(' ') << i.id << "| ";
            s << left << setw(24) << setfill(' ') << i.info.description << "| ";
            s << left << setw(11) << setfill(' ') << i.ticksRemaining << "| ";
            s << i.info.numProcs << '\n';
        }
    }
}