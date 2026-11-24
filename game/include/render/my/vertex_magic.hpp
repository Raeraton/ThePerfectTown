#ifndef VERTEX_MAGIC_HPP
#define VERTEX_MAGIC_HPP

#include <vector>
#include <string>


#include <render/glm/glm.hpp>
#include <render/glm/gtc/matrix_transform.hpp>
#include <render/glm/gtc/type_ptr.hpp>


namespace triangulation{


bool intersect( glm::vec2 l1p1, glm::vec2 l1p2, glm::vec2 l2p1, glm::vec2 l2p2 );


bool point_in_triangle(
    glm::vec2 p,
    glm::vec2 a,
    glm::vec2 b,
    glm::vec2 c
);

size_t find_ear( const std::vector<glm::vec2>& points );

// return a list of triangles.... triangles not triangle strip
std::vector<glm::vec2> triangulate( std::vector<glm::vec2> points );

}



#endif