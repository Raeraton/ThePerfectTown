#ifndef BACKGROUND_TRAFFIC_HPP
#define BACKGROUND_TRAFFIC_HPP

#include <cstdint>
#include <functional>
#include <string>
#include <vector>
#include <n_npc.hpp>
#include <rotating_buffer.hpp>
#include <map>
#include <location.hpp>

namespace game::traffic::backgound{

typedef uint32_t Edge_idx;
typedef uint32_t Node_idx;

struct Edge;

using Position = game::map::location::Position;


struct Node{
    std::vector<Edge_idx> edges;
    Position position;
    Node() = default;
    Node(const Position& p)
            : position{p} {}
};

struct Edge{
    double speed_limit=0;
    double lenght=0;
    Node_idx to=0;
   
    Edge() = default;
    Edge(double _speed_limit, Node_idx to )
            : speed_limit(_speed_limit), to(to) {}
};



class Graph{

    std::vector<Edge> m_edges;
    std::vector<Node> m_nodes;

public:

    static Graph read_from_text_file(const char* path);

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


    inline Node_idx node_count() const {return (Node_idx)m_nodes.size();}
    inline Node& node(Node_idx i){
        if( i >= m_nodes.size() )
            throw std::string{"game::traffic::background::Graph node   node out of range " };
        return m_nodes[i];
    }

    inline Edge_idx edge_count() const {return (Edge_idx)m_edges.size();}
    inline Edge edge(Edge_idx i){
        if( i >= m_edges.size() )
            throw std::string{"game::traffic::background::Graph node   node out of range " };
        return m_edges[i];
    }

    inline Node_idx add_node(const Node& node) {
        for( auto ei : node.edges )
            if(ei >= m_edges.size())
                throw std::string{"game::traffic::background::Graph add_node one of the nodes edge doesent exists"};
        
        Node_idx out = m_nodes.size();
        m_nodes.push_back(node);
        return out;
    }

    inline Edge_idx add_edge(Edge edge, Node_idx from){
        if(edge.to >= m_nodes.size() || from >= m_nodes.size() )throw std::string{"game::traffic::background::Graph add_edge to node doesent exists"};

        edge.lenght = Position::dist(
            m_nodes[from].position,
            m_nodes[edge.to].position
        );

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

    inline bool get_edge_between( Node_idx from, Node_idx to, Edge_idx* output ){
        for( auto edge : m_nodes[from].edges ){
            if( m_edges[edge].to == to ) {
                *output = edge;
                return true;
            }
        }
        return false;
    }

    /// map<node, prevnode>... origin mapped to itself
    std::map<Node_idx, Node_idx> create_full_prev_table(Node_idx origin) const;

    /// nodes in order to the destination
    std::vector<Node_idx> calculate_shortest_path(Node_idx from, Node_idx to) const;

    std::vector<Node_idx> get_shortest_path_from_prev_table(
            const std::map<Node_idx, Node_idx>& table, Node_idx from, Node_idx to) const;

    bool path_is_valid( const std::vector<Node_idx>& ) const noexcept;

};



class Time_table{
public:
    struct Time_range{
        npc::People_idx id;
        // hours [0-23.(9)]
        double start, end;
    };

private:
    std::map<Edge_idx, std::vector<Time_range>> m_table;

public:

    void add_trip( npc::People_idx people, double stime, const std::vector<Node_idx>& path, Graph& graph);
    
    // returns the list of the peoples on the edge, at the current time
    // returns empty list if edge not found (for now)
    std::vector<npc::People_idx> get_peoples_at( double time_point, Edge_idx edge ) const;


    void do_all( std::function< void(Edge_idx,const std::vector<Time_range>&) > func ) const;

};




}



#endif
