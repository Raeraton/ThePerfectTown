#include <render/my/vertex_magic.hpp>


namespace triangulation{


static inline size_t get_prev(const std::vector<glm::vec2>& points, size_t i){
    if( i == 0 ) return points.size()-1;
    return i-1;
}
static inline size_t get_next(const std::vector<glm::vec2>& points, size_t i){
    if(i >= points.size()-1) return 0;
    return i+1;
}

inline static float cross( glm::vec2 a, glm::vec2 b){
    return a.x * b.y - a.y * b.x;
}



bool intersect(
    glm::vec2 p1, glm::vec2 p2,
    glm::vec2 q1, glm::vec2 q2
) {
    glm::vec2 r = p2 - p1;
    glm::vec2 s = q2 - q1;

    float denominator = cross(r, s);

    if (std::abs(denominator) < 1e-6f)
        return false;

    glm::vec2 qp = q1 - p1;

    float t = cross(qp, s) / denominator;
    float u = cross(qp, r) / denominator;

    return t >= 0 && t <= 1 &&
           u >= 0 && u <= 1;
}


bool point_in_triangle(
    glm::vec2 p,
    glm::vec2 a,
    glm::vec2 b,
    glm::vec2 c
) {
    float c1 = cross(b-a, p-a);
    float c2 = cross(c-b, p-b);
    float c3 = cross(a-c, p-c);

    bool has_neg = c1 < 0 || c2 < 0 || c3 < 0;
    bool has_pos = c1 > 0 || c2 > 0 || c3 > 0;

    return !(has_neg && has_pos);
}

size_t find_ear( const std::vector<glm::vec2>& points ){

    if( points.size() <= 3 ) throw std::string{"find ear too small"};

    for( size_t i=0;  i<points.size();  i++ ){

        size_t pi = get_prev(points, i),  ni = get_next(points, i);

        glm::vec2   prev = points[pi],
                    next = points[ni];
        
        bool good = true;
        
        for( size_t j=0;  j<points.size();  j++ ){
            size_t jp = get_prev(points, j);
            if (j == pi || j == ni || jp == pi || jp == ni)
                continue;

            if (point_in_triangle(points[j], prev, points[i], next)) {
                good = false;
                break;
            }

            glm::vec2   cpoint = points[j],
                        cprev = points[jp];
            
            if( intersect( prev, next, cpoint, cprev ) ){
                good = false;
                break;
            }
        }

        if ( good ) return i;

    }

    throw std::string{ "find_ear impossible" };
}

// return a list of triangles.... triangles not triangle strip
std::vector<glm::vec2> triangulate( std::vector<glm::vec2> points ){
    if( points.size() < 3 ) return {};

    std::vector<glm::vec2> triangles;

    while( points.size() > 3 ){
        size_t ear = find_ear(points);

        triangles.push_back( points[get_prev(points, ear)] );
        triangles.push_back( points[ear] );
        triangles.push_back( points[get_next(points, ear)] );

        points.erase( points.begin() + ear );
    }

    triangles.push_back( points[0] );
    triangles.push_back( points[1] );
    triangles.push_back( points[2] );
    
    return triangles;

}









}
