#ifndef BUILDINGS_HPP
#define BUILDINGS_HPP


#include <render/glad/gl.h>
#include <GLFW/glfw3.h>
#include <render/glm/glm.hpp>
#include <render/glm/gtc/matrix_transform.hpp>
#include <render/glm/gtc/type_ptr.hpp>

#include <map.hpp>

namespace game::render::objects{



class Buildings{

    GLuint vao, vbo, shader_program;
    size_t vertex_count;

public:

    Buildings(game::map::Map& map, const char* vs, const char* fs);

    void draw( glm::vec2 middle, float scale );

};



}



#endif