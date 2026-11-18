#ifndef LOCATION_HPP
#define LOCATION_HPP

#include <cmath>

namespace game::map::location{



struct Position{
    double x=0,y=0;
    Position() = default;
    Position(double x, double y): x{x}, y{y} {}
    static inline double dist(const Position& l1, const Position& l2){
        double  dx = l1.x - l2.x,
                dy = l1.y - l2.y;
        return sqrt(dx*dx + dy*dy);
    }
};


struct Location{
    Position    position,
                top_left,
                bottom_right;
    Location(const Position& p, const Position&  tl, const Position& br)
        : position(p), top_left(tl), bottom_right(br) {}
};




}


#endif
