
#include <string>
#include <iostream>
#include "scheduler.h"

void doTicks(Scheduler& sch, int ticks)
{
    for(int i = 0; i < ticks; ++i)
        sch.tick();

    std::cout << "\n";
    sch.printActiveJobs(std::cout);
    std::cout << "\n\n";
    sch.printWaitQueue(std::cout);
    std::cout << std::endl;
}

void runprogram(unsigned procs)
{
    bool run = true;
    Scheduler sch{procs};
    
    JobInfo info;
    int i;

    while(run)
    {
        std::cout << "\n>> ";
        // try to read an integer first
        if(std::cin >> i)
        {
            // if it was an integer, that's the number of ticks to perform
            if(i >= 0)
                doTicks(sch, i);
            else
            {
                std::cout << "Invalid number of ticks specified.\n";
            }
        }
        else
        {
            // They didn't input an integer, it must be a string!
            std::cin.clear();
            std::cin >> info.description;

            // special case if they typed "exit"
            if(info.description == "exit")
                run = false;
            else
            {
                info.numProcs = info.numTicks = 0;
                std::cin >> info.numProcs >> info.numTicks;
                std::cin.clear();

                if(sch.addJob(info))
                    std::cout << "Job added successfully\n";
                else
                    std::cout << "Failed to add job. Possibly invalid number of processors or ticks specified.\n";
            }
        }
    }
}

int main(int argc, char* argv[])
{
    static const unsigned defNumProcs = 5;       // default to 5 procs

    unsigned numprocs = defNumProcs;

    // get the number of procs from argv
    if(argc >= 2) {
        numprocs = std::stoul(argv[1]);
        if(numprocs < 1)
        {
            numprocs = defNumProcs;
            std::cout << "Invalid number of processors specified. Defaulting to " << numprocs << ".\n";
        }
    }

    std::cout << "Scheduler started with " << numprocs << " processors.\n";
    runprogram(numprocs);
}