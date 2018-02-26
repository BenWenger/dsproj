
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
    waitQueue.insert( std::move(job) );
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

void Scheduler::freeProcessors(ScheduledJob& job)
{
    for(unsigned i = 0; i < job.info.numProcs; ++i)
    {
        auto prid = job.procsUsed[i];
        job.procsUsed[i] = NoProc;
        if(prid == NoProc)
            throw SchedulerException("Internal Error:  freeProcessors called on a job with unassigned procs");

        availProcs.push_back(prid);
        processors[prid] = NoJob;
    }

    needProcAssign = true;
}

// This is the logic for actually determining which processors get assigned to which jobs.
//   This will pull items out of the wait queue and put them in the active list.
//   (and vice versa, depending on the algorithm)
//
// Current algorithm:
//  - Wait queue is ordered as follows:
//          -- lowest ticks remaining first
//          -- highest proc count next
//  - This function will walk through the queue and fill up processors as able.
//  - If the next entry in the queue needs more procs than is available, skip it
//      and keep walking through the queue and take the next item that fits
//  - Continue until all procs used or we walked through the entire wait queue
//
//
// Additional tidbit:
//    Before running above logic, look at the first entry in the wait queue.
//  If we can swap out jobs that are currently running (but have a higher tick count) than
//  that job to make room for that job, do so.

void Scheduler::assignProcs()
{
    // nothing to do here if the wait queue is empty
    if(waitQueue.empty())       return;     

    // Next job in the wait queue is 'next'.  If there are jobs running that have a higher tick count
    //   than 'next', see if booting them out will create enough room for next.  If yes, do that.
    auto& next = *waitQueue.begin();

    auto avail = availProcs.size();
    if(avail < next.info.numProcs)     // only do this if we don't have enough to run 'next'
    {
        std::vector<activelst_t::iterator>  bootable;
        for(auto i = activeJobs.begin(); i != activeJobs.end(); ++i)
        {
            if(next.ticksRemaining < i->ticksRemaining)
            {
                bootable.push_back(i);
                avail += i->info.numProcs;
            }
        }

        // if we boot all bootable jobs, would that free up enough procs?  If yes, do it
        if(avail >= next.info.numProcs)
        {
            for(auto& i : bootable)
            {
                freeProcessors(*i);
                waitQueue.insert( std::move(*i) );
                activeJobs.erase(i);
            }
        }
    }

    //  Now run the "main" logic... just walk through the wait queue in order and make
    //     jobs active.
    auto i = waitQueue.begin();

    //  keep looping as long as we have waiting jobs and available processors
    while(i != waitQueue.end() && !availProcs.empty())
    {
        // can we service this job?
        if(availProcs.size() >= i->info.numProcs)
        {
            allocateProcessors(*i);
            activeJobs.push_back( std::move(*i) );
            i = waitQueue.erase(i);
        }
        else
            ++i;
    }

    needProcAssign = false;
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
        for(auto& i : waitQueue)
        {
            s << left << setw(8) << setfill(' ') << i.id << "| ";
            s << left << setw(24) << setfill(' ') << i.info.description << "| ";
            s << left << setw(11) << setfill(' ') << i.ticksRemaining << "| ";
            s << i.info.numProcs << '\n';
        }
    }
}