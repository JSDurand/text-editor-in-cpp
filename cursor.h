#ifndef __CURSOR_H_
#define __CURSOR_H_

/* #include <cstdint> */
#include <utility>

// NOTE: For the structure of cursors. I intended to use a union to represent
// it, but it turns out this is a bad idea, so I changed it to a plain struct,
// and add a member to determine its type.

enum cursorIndex { BUFFER_LOC, BUFFER_REG, SCREEN_POS };

struct cursor
{
public:
    cursor ()
        : buffer_location (0),
        buffer_region (std::make_pair<long, long>(0, 0)),
        screen_position (std::make_pair<double, double>(0, 0)),
        index(BUFFER_LOC)
    {};

    cursor (long pos)
        : buffer_location (pos),
        buffer_region (std::make_pair<long, long>(0, 0)),
        screen_position (std::make_pair<double, double>(0, 0)),
        index(BUFFER_LOC)
    {};

    cursor (std::pair<long, long> region)
        : buffer_location (0),
        buffer_region (region),
        screen_position (std::make_pair<double, double>(0, 0)),
        index(BUFFER_REG)
    {};

    cursor (std::pair<double, double> screen_pos)
        : buffer_location (0),
        buffer_region (std::make_pair<long, long>(0, 0)),
        screen_position (screen_pos),
        index(SCREEN_POS)
    {};

    cursorIndex cursor_branch () const;

    long get_buffer_location () const;

    std::pair<long, long> get_buffer_region () const;

    std::pair<double, double> get_screen_position () const;

    // TODO: Implement methods for a cursor to move.

private:
    long buffer_location;
    std::pair<long, long> buffer_region;
    std::pair<double, double> screen_position;

    cursorIndex index;
};

#endif // __CURSOR_H_
