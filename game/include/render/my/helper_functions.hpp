#ifndef HELPERS_HPP
#define HELPERS_HPP

#include <filesystem>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>


#include <render/glad/gl.h>
#include <GLFW/glfw3.h>
#include <render/glm/glm.hpp>
#include <render/glm/gtc/matrix_transform.hpp>
#include <render/glm/gtc/type_ptr.hpp>


std::string load_shader_code(const char* path);
GLuint create_shader_program( const char* vertex_shader_path, const char* fragment_shader_path );


GLint get_uniform_location( GLuint shader_program, const char* unif_name );

std::vector<std::string> list_available_uniforms(GLuint program);

void set_uniform( GLint loc, const glm::mat4& mat );
void set_uniform( GLint loc, const glm::vec3& vec );
void set_uniform( GLint loc, const glm::vec2& vec );

void set_uniform( GLint loc, float f );

void create_render_target( GLuint* fbo, GLuint* texture, GLuint* depth, int w, int h );



#endif