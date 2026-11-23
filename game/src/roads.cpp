#include <render/objects/roads.hpp>

#include <render/my/helper_functions.hpp>

#include <render/glad/gl.h>
#include <GLFW/glfw3.h>
#include <render/glm/glm.hpp>
#include <render/glm/gtc/matrix_transform.hpp>
#include <render/glm/gtc/type_ptr.hpp>

using namespace game::render::objects;
namespace bg = game::traffic::backgound;

Roads::Roads( bg::Graph& graph, const char* vs, const char* fs ){

    // load vectecies
    std::vector<glm::vec2> vertecies;

    for( bg::Node_idx node=0;  node<graph.node_count();  node++ ){
        auto& from = graph.node(node);
        const auto& edges = from.edges;

        for( bg::Edge_idx edge : edges ){

            bg::Node_idx toi = graph.edge(edge).to;
            auto& to = graph.node(toi);

            vertecies.push_back(
                glm::vec2{ from.position.x, from.position.y }
            );
            vertecies.push_back(
                glm::vec2{ to.position.x, to.position.y }
            );

        }
    }

    vertex_count = vertecies.size();

    // init gpu buffers and shaders
    shader_program = create_shader_program( vs, fs );

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(glm::vec2) * vertecies.size(),
        vertecies.data(),
        GL_STATIC_DRAW
    );

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
    glEnableVertexAttribArray(0);


    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

}


void Roads::draw( glm::vec2 middle, float scale ){
    auto model = glm::scale( glm::translate(  glm::mat4{1}, glm::vec3{ middle.x, 1, middle.y } ), glm::vec3{scale, 1, scale} );
    
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
    
    glDrawArrays( GL_LINES, 0, vertex_count );

}