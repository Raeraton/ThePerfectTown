#include <algorithm>
#include <background_traffic.hpp>
#include <limits>


namespace game::traffic::backgound{


    /// dijkstra
    std::map<Node_idx, Node_idx> Graph::create_full_distance_table(Node_idx origin) const{
        
        assert_node_valid(origin);

        std::map<Node_idx, Node_idx> prev_map;
        std::map<Node_idx, double> dist_map;
        std::set<Node_idx> remaining;
        for(Node_idx i=0;  i<m_nodes.size();  i++)
            remaining.insert(i);

        prev_map[origin] = origin;
        dist_map[origin] = 0;

        Node_idx current=origin;
        while( true ) {
            
            remaining.erase(current);

            double current_dist = dist_map.at(current);
            const Node& cnode = m_nodes[current];

            for( Edge_idx e : cnode.edges ) {

                const Edge& edge = m_edges[e];
                
                double cdist = current_dist + edge.lenght;
                auto edist_it = dist_map.find(edge.to);
                
                if( edist_it == dist_map.end() ){
                    dist_map[edge.to] = cdist;
                    prev_map[edge.to] = current;
                }else{
                    if( edist_it->second > cdist ){
                        dist_map[edge.to] = cdist;
                        prev_map[edge.to] = current;
                    }
                }
            }

            double min_dist=std::numeric_limits<double>::infinity();
            Node_idx next_idx = current; // current is not in remaining so its
                                         // an invalid value
            for( Node_idx node : remaining ){
                auto dist_it = dist_map.find(node);
                if( dist_it == dist_map.end() ) continue;
                if( dist_it->second < min_dist ){
                    min_dist = dist_it->second;
                    next_idx = node;
                }
            }

            if( next_idx == current ) break;
            current = next_idx;
        }

        return prev_map;

    }

    std::vector<Node_idx> Graph::calculate_shortest_path(Node_idx from, Node_idx to) const{
        assert_node_valid(from);
        assert_node_valid(to);

        std::vector<Node_idx> path;
        auto prev_map = create_full_distance_table(from);

        Node_idx current = to;
        path.push_back(current);
        while( current != from ){
            
            auto it = prev_map.find(current);
            if( it == prev_map.end() )
                return {};

            current = it->second;

            path.push_back(current);
        }

        std::reverse(path.begin(), path.end());

        return path;
    }

    bool Graph::path_is_valid( const std::vector<Node_idx>& path ) const noexcept{
        
        for( size_t i=0;  i<path.size()-1;  i++ ){
            if( ! is_edge_between(path[i], path[i+1]) ) return false;
        }

        return true;

    }


}





