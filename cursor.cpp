#include "cursor.h"

cursorIndex cursor::cursor_branch () const
{
    return this->index;
};

long cursor::get_buffer_location () const
{
    return this->buffer_location;
};

std::pair<long, long> cursor::get_buffer_region () const
{
    return this->buffer_region;
};

std::pair<double, double> cursor::get_screen_position () const
{
    return this->screen_position;
};
