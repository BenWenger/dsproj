
#ifndef SCHEDULER_H_INCLUDED
#define SCHEDULER_H_INCLUDED

#include <set>
#include "treelist.h"
#include <vector>
#include <list>
#include <iostream>
#include "types.h"
#include "job.h"

class Scheduler
{
public:
                Scheduler(unsigned numprocs);
    bool        addJob(const JobInfo& jobinfo);
    void        tick();
    
    void        printActiveJobs(std::ostream& s) const;
    void        printWaitQueue(std::ostream& s) const;

private:
    typedef TreeList<ScheduledJob>      queue_t;
    typedef std::list<ScheduledJob>     activelst_t;
    queue_t                     waitQueue;
    activelst_t                 activeJobs;
    std::vector<jobid_t>        processors;     // each entry is the job ID the processor is using
    std::vector<procid_t>       availProcs;

    jobid_t                     lastJobId;      // last assigned job ID
    std::set<jobid_t>           usedJobIds;     // all job IDs that are currently used
    bool                        needProcAssign;

    jobid_t     getUniqueJobId();
    bool        isJobIdInUse(jobid_t id) const;

    void        putJobInWaitQueue(ScheduledJob&& job);

    
    void        runActiveJobs();
    void        assignProcs();

    void        freeProcessors(ScheduledJob& job);
    void        allocateProcessors(ScheduledJob& job);
};


#endif