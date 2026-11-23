#ifndef LOCATION_HPP
#define LOCATION_HPP

#include <cmath>
#include <vector>

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
    Position    position;
    std::vector<Position> corners;
    Location() = default;
    Location(const Position& p, const std::vector<Position>& cs)
        : position(p), corners(cs) {}
};




}


#endif
