
#ifndef TYPES_H_INCLUDED
#define TYPES_H_INCLUDED

#include <limits>
#include <string>
#include <stdexcept>

typedef std::size_t     jobid_t;
typedef std::size_t     procid_t;

namespace
{
    constexpr jobid_t   NoJob = std::numeric_limits<jobid_t>::max();
    constexpr procid_t  NoProc = std::numeric_limits<procid_t>::max();
}

class SchedulerException : public std::runtime_error
{
public:
    SchedulerException(const char* desc) : runtime_error(desc) {}
    SchedulerException(const std::string& desc) : runtime_error(desc) {}
};

#endif