#include "n_npc.hpp"
#include <algorithm>
#include <background_traffic.hpp>
#include <limits>
#include <fstream>


namespace game::traffic::backgound{



    Graph Graph::read_from_text_file(const char* path){
        std::ifstream file{path};
        if( ! file.is_open() )
            throw std::string{"Graph::read_from_text_file path cannot be opened"};

        Graph graph;

        Edge_idx edge_count;
        Node_idx node_count;
        file >> node_count >> edge_count;

        std::map<Node_idx, Node_idx> node_idx_translation_map;

        for( Node_idx i=0;  i<node_count;  i++){
            Node_idx id;
            double x, y;
            file >> id >> x >> y;

            Node_idx given_idx = graph.add_node( Node{Position{x, y}} );
            if( id != given_idx )
                node_idx_translation_map[id] = given_idx;
        }

        for( Edge_idx i=0;  i<edge_count;  i++){
            Edge_idx id;
            Node_idx from, to;
            file >> id >> from >> to;

            auto from_it = node_idx_translation_map.find(from);
            auto to_it = node_idx_translation_map.find(to);

            if(from_it != node_idx_translation_map.end())
                from = from_it->second;
            if(to_it != node_idx_translation_map.end())
                to = to_it->second;

            graph.add_edge( Edge{ 50, to }, from );
        }

        return graph;


    }


    /// dijkstra
    std::map<Node_idx, Node_idx> Graph::create_full_prev_table(Node_idx origin) const{
        
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
        auto prev_map = create_full_prev_table(from);

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

    std::vector<Node_idx> Graph::get_shortest_path_from_prev_table(
            const std::map<Node_idx, Node_idx>& prev_map, Node_idx from, Node_idx to) const{

        assert_node_valid(from);
        assert_node_valid(to);

        std::vector<Node_idx> path;

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


    void Time_table::add_trip( npc::People_idx people, double stime, const std::vector<Node_idx>& path, Graph& graph){

        double time = stime;
        for( size_t i=1;  i<path.size();  i++){
            Edge_idx edgei;
            if( ! graph.get_edge_between(path[i-1], path[i], &edgei) )
                throw std::string{"game::traffic::background::Time_table::add_trip edge not found in path"};
            const auto& edge = graph.edge(edgei);
            
            double dur = edge.lenght * 3.6 / edge.speed_limit;

            auto it = m_table.find(edgei);
            if(it == m_table.end() ){
                m_table[edgei] = std::vector{
                    Time_range{ .id=people, .start=time, .end=time+dur }
                };
            }else{
                it->second.push_back(Time_range{
                        .id=people, 
                        .start=time, 
                        .end=time+dur 
                });
            }

            time += dur;

        }

    }

    
    std::vector<npc::People_idx> Time_table::get_peoples_at( double time_point, Edge_idx edge) const {

        auto it = m_table.find( edge );
        
        std::vector<npc::People_idx> peoples;
        if(it == m_table.end() ){
            for( const auto& time_range : it->second ){
                if( time_range.start <= time_point && time_point <= time_range.end ){
                    peoples.push_back(time_range.id);
                }
            }
        }

        return peoples;
    }


    void Time_table::do_all( std::function< void(Edge_idx,const std::vector<Time_range>&) > func ) const{
        for( const auto& pair : m_table ){
            func( pair.first, pair.second );
        }
    }






}





