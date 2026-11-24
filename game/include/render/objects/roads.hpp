#ifndef ROADS_HPP
#define ROADS_HPP

#include <render/glad/gl.h>
#include <GLFW/glfw3.h>
#include <render/glm/glm.hpp>
#include <render/glm/gtc/matrix_transform.hpp>
#include <render/glm/gtc/type_ptr.hpp>

#include <background_traffic.hpp>


namespace game::render::objects{


class Roads{

    GLuint vao, vbo, shader_program;
    size_t vertex_count;

public:

    Roads( game::traffic::backgound::Graph& graph, const char* vs, const char* fs );

    void draw( glm::vec2 middle, float scale );

};


}





#endif
