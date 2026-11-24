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



bool intersect( glm::vec2 l1p1, glm::vec2 l1p2, glm::vec2 l2p1, glm::vec2 l2p2 ){    
    // p11(a,b) p12(c,d)
    // p21(e,f) p22(g,h)
    //
    // l1(t) = t*p11 + (1-t)*p12
    // l2(t) = t*p21 + (1-t)*p22
    // -------------------------------------
    // t1*a + (1-t1)*c = t2*e + (1-t2)*g
    // t1*b + (1-t1)*d = t2*f + (1-t2)*h
    // -------------------------------------
    // t1*a + (1-t1)*c = t1*(a-c) + c
    // t1*b + (1-t1)*d = t1*(b-d) + d
    // t2*e + (1-t2)*g = t2*(e-g) + g
    // t2*f + (1-t2)*h = t2*(f-h) + h
    // -------------------------------------
    // t1 = (t2*(e-g) + g-c) / (a-c)
    // t2 = (t1*(b-d) + d-h) / (f-h)
    // ------------------------------------- t1
    // t1 = ((t1*(b-d) + d-h) / (f-h) * (e-g) + g-c) / (a-c)
    // t1 = (t1*(b-d) + d-h) / (f-h) * (e-g) / (a-c) + (g-c) / (a-c)
    //
    // K = (e-g) / ((f-h)*(a-c))
    // G = (g-c) / (a-c)
    //
    // t1 = K * (t1*(b-d) + d-h) + G
    // t1 = t1*K*(b-d) + K*(d-h) + G
    // t1 - t1*K*(b-d) = K*(d-h) + G
    // t1 * (1 - K*(b-d)) = K*(d-h) + G
    //
    // C = K*(d-h) + G
    // L = 1 - K*(b-d)
    //
    // t1 = C / L
    // ------------------------------------ t2
    // t2 = (t1*(b-d) + d-h) / (f-h)

    float   a = l1p1.x,
            b = l1p1.y,
            c = l1p2.x,
            d = l1p2.y,
            e = l2p1.x,
            f = l2p1.y,
            g = l2p2.x,
            h = l2p2.y;
    
    float   K = (e-g) / ((f-h)*(a-c)),
            G = (g-c) / (a-c);

    float   C = K*(d-h) + G,
            L = 1 - K*(b-d);
    
    float t1 = C / L;
    float t2 = (t1*(b-d) + d-h) / (f-h);

    return 0<=t1&&t1<=1 && 0<=t2&&t2<=1;
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
