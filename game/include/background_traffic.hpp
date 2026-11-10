#ifndef BACKGROUND_TRAFFIC_HPP
#define BACKGROUND_TRAFFIC_HPP

#include <cstdint>
#include <string>
#include <vector>
#include <n_npc.hpp>
#include <rotating_buffer.hpp>
#include <map>

namespace game::traffic::backgound{

typedef uint32_t Edge_idx;
typedef uint32_t Node_idx;

struct Edge;
struct Location{
};


struct Node{
    std::vector<Edge_idx> edges;
    Location location;
};

struct Edge{
    double speed_limit=0;
    double lenght=0;
    Node_idx to=0;
   
    Edge() = default;
    Edge(double speed_limit, double lenght, Node_idx to )
            : speed_limit(speed_limit), lenght(lenght), to(to) {}
};



class Graph{

    std::vector<Edge> m_edges;
    std::vector<Node> m_nodes;

public:

    inline void assert_node_valid( Node_idx node ) const {
        if(node >= m_nodes.size() )
            throw std::string{"game::traffic::background::Graph node is invalid: "}
            + std::to_string(node);
    }

    inline void assert_edge_valid( Edge_idx edge ) const {
        if( edge >= m_edges.size() )
            throw std::string{"game::traffic::background::Graph edge is invalid: "}
            + std::to_string(edge);
    }
    inline bool is_node_valid( Node_idx node ) const noexcept {
        return node < m_nodes.size();
    }

    inline bool is_edge_valid( Edge_idx edge ) const noexcept{
        return edge < m_edges.size();
    }




    inline Node_idx add_node(const Node& node) {
        for( auto ei : node.edges )
            if(ei >= m_edges.size())
                throw std::string{"game::traffic::background::Graph add_node one of the nodes edge doesent exists"};
        
        Node_idx out = m_nodes.size();
        m_nodes.push_back(node);
        return out;
    }

    inline Edge_idx add_edge(const Edge& edge, Node_idx from){
        if(edge.to >= m_nodes.size() || from >= m_nodes.size() )throw std::string{"game::traffic::background::Graph add_edge to node doesent exists"};

        Edge_idx out = m_edges.size();
        m_edges.push_back(edge);
        m_nodes[from].edges.push_back(out);
        return out;
    }

    inline bool is_edge_between(Node_idx from, Node_idx to ) const noexcept {
        
        for( auto edge : m_nodes[from].edges ){
            if( m_edges[edge].to == to ) return true;
        }

        return false;
    }

    /// map<node, prevnode>... origin mapped to itself
    std::map<Node_idx, Node_idx> create_full_distance_table(Node_idx origin) const;

    /// nodes in order to the destination
    std::vector<Node_idx> calculate_shortest_path(Node_idx from, Node_idx to) const;

    bool path_is_valid( const std::vector<Node_idx>& ) const noexcept;

};




}



#endif
