#ifndef MAP_HPP
#define MAP_HPP

#include <istream>
#include <vector>
#include <location.hpp>
#include <cstdint>

namespace game::map{


struct Building{
    std::string type; // TODO refactor to enum
    location::Location location;
};

std::istream& operator>>(std::istream& in, Building& b);


typedef uint32_t Building_idx;
class Map{

    std::vector<Building> m_buildings;

public:

    void load_buildings( const char* path ); 
    Building_idx building_count() const { return (Building_idx)m_buildings.size(); }
    Building& building(Building_idx i) {
        if ( i >= m_buildings.size() )
            throw std::string{"building index out of range"};
        return m_buildings[i];
    }


};


}


#endif
