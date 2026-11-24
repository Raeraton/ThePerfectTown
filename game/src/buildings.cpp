#include <render/objects/buildings.hpp>

#include <algorithm>
#include <render/my/helper_functions.hpp>
#include <render/my/vertex_magic.hpp>

using namespace game::render::objects;



Buildings::Buildings(game::map::Map& map, const char* vs, const char* fs){

    std::vector<glm::vec2> vertecies;

    for( game::map::Building_idx building_idx=0;  building_idx<map.building_count();  building_idx++ ){
        auto& b = map.building(building_idx);

        std::vector<glm::vec2> points;
        std::transform( b.location.corners.begin(), b.location.corners.end(), std::back_inserter(points), 
                [](const game::map::location::Position& p){
            return glm::vec2{ p.x, p.y };
        } );

        std::vector<glm::vec2> triangles = triangulation::triangulate( points );

        vertecies.insert( vertecies.end(), triangles.begin(), triangles.end() );
    }

    vertex_count = vertecies.size();

    shader_program = create_shader_program( vs, fs );

    glGenVertexArrays( 1, &vao );
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glBufferData(
        GL_ARRAY_BUFFER,
        vertecies.size() * sizeof(glm::vec2),
        vertecies.data(),
        GL_STATIC_DRAW
    );

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

}

void Buildings::draw( glm::vec2 middle, float scale ){

    glUseProgram(shader_program);
    glBindVertexArray(vao);

    set_uniform(
        get_uniform_location(shader_program, "middle"),
        middle
    );
    set_uniform(
        get_uniform_location(shader_program, "scale"),
        scale
    );
    
    glDrawArrays( GL_TRIANGLES, 0, vertex_count );

}