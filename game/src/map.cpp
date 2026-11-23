#include <fstream>
#include <map.hpp>


namespace game::map {


std::istream& operator>>(std::istream& in, Building& b) {
    in >> b.type;
    size_t corner_count;       
    in >> corner_count;
    location::Location loc{};
    for( size_t i=0; i<corner_count; i++ ){
        location::Position pos;
        in >> pos.x;
        in >> pos.y;
        loc.corners.push_back(pos);
    }      
    b.location = loc;
    return in;
}


void Map::load_buildings( const char* path ){
    std::ifstream file{path};
    if( !file.is_open() )
        throw std::string{"load_building file cannot be opened: "} + path;

    size_t building_count;
    file >> building_count;
    for( size_t i=0;  i<building_count;  i++){
        Building b;
        file >> b;
        m_buildings.push_back(b);
    }

    file.close();
}


}
