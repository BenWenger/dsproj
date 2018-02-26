
#ifndef JOB_H_INCLUDED
#define JOB_H_INCLUDED

#include <string>
#include <memory>
#include "types.h"

struct JobInfo
{
    std::string     description;
    unsigned        numProcs;
    unsigned        numTicks;
};

struct ScheduledJob
{
    JobInfo                     info;           // supplied info about the job
    jobid_t                     id;             // unique ID assigned to this job
    unsigned                    ticksRemaining; // number of ticks remaining until the job is complete
    std::unique_ptr<procid_t[]> procsUsed;      // list of processors currently occupied by the job.

    bool operator < (const ScheduledJob& rhs) const
    {
        // sort by ticksRemaining first (ascending)
        if(ticksRemaining < rhs.ticksRemaining)     return true;
        if(ticksRemaining > rhs.ticksRemaining)     return false;

        // sort by numProcs next (descending)
        if(info.numProcs > rhs.info.numProcs)       return true;
        if(info.numProcs < rhs.info.numProcs)       return false;

        // and by job id last because whynot (ascending)
        return id < rhs.id;
    }
};

#endif