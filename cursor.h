#ifndef __CURSOR_H_
#define __CURSOR_H_

#include <cstdint>
#include <utility>

// For the structure of cursors

union cursor
{
    long buffer_location;
    std::pair<long, long> buffer_region;
    std::pair<double, double> screen_position;
};

#endif // __CURSOR_H_
