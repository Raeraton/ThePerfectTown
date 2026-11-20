#include "location.hpp"
#include <cstdlib>
#include <iostream>
#include <n_npc.hpp>
#include <background_traffic.hpp>
#include <ctime>
#include <chrono>
#include <vector>


void npc_bm(int argc, char** argv){
try{
   
    size_t population = 1000;
    size_t iteration = 120;
    if(argc > 1){
        population = std::atoll(argv[1]);
    }
    if(argc > 2){
        iteration = std::atoll(argv[2]);
    }


    auto tp = std::chrono::high_resolution_clock::now();
    std::vector<game::map::location::Location> locations = { {{0,0},{0,0},{0,0} } };
    game::npc::World world{population, locations};
    auto tp2 =  std::chrono::high_resolution_clock::now();
    double dur1 = std::chrono::duration<double>(tp2 - tp).count();


    for( size_t _=0; _<iteration; _++ ){
        int day = (_ / 24) % 7;
        int hour = _ % 24;
        world.update(game::npc::Time_range{day, hour}, (_+1)%(168*4)==0, locations);
    }
    
    tp = std::chrono::high_resolution_clock::now();
    double dur2 = std::chrono::duration<double>(tp-tp2).count();

    if(  population <= 1000000 )
        world.print();
    
    std::cout << "init time: " << dur1 << " sec\n";
    std::cout << "sim  took: " << dur2 << " sec\n";
    std::cout << "all      : " << dur1 + dur2 << " sec\n";
}catch(std::string err){
    std::cerr<<"[error] "<<err<<"\n";
}catch(...){
    std::cerr<<"[unknown error]\n";
}
}




void bg_graph_bm(int argc, char** argv) {
    
    int width=10;
    int start=99;
    int end = 0;
    int range = 1;
    if(argc >= 2){
        width = std::atoi(argv[1]);
    }
    if( argc >= 3 ) {
        start = std::atoi(argv[2]);
    }if(argc >= 4 ){
        end = std::atoi(argv[3]);
    }if(argc >= 5 ){
        range = std::atoi(argv[4]) + 1;
    }
    
    std::cout << "width: " << width << "\nstart: " << start << "\nend: " << end
                << "\nrange " << range << "\n";

    game::traffic::backgound::Graph graph{};

    int node_count = 0;
    for( int i=0;  i<width*width;  i++){
        node_count++;
        int y = i/width;
        int x = i%width;
        double lambde = rand() % range;
        graph.add_node(game::traffic::backgound::Node{
            game::traffic::backgound::Position{
                (double)x * lambde, (double)y * lambde // geometric bulshit to
                                                       // imagine :)
            }
        });
    }

    int edge_count = 0;
    for(int y=0;  y<width;  y++){
        for(int x=0;  x<width;  x++){
            for(int y0=-1;  y0<2;  y0++){
                for(int x0=-1;  x0<2;  x0++){
                    int xx = x+x0;
                    int yy = y+y0;
                    if( xx<0 || yy<0 || xx>=width || yy>=width ) continue;
                    if( x0==0 && y0 == 0 ) continue;
                    if( y0!=0 && x0 != 0 ) continue;
                    int idx = y*width + x;
                    int idx0 = yy*width + xx;
                    edge_count++;

                    graph.add_edge(game::traffic::backgound::Edge{ 50, (game::traffic::backgound::Node_idx)idx0 }, idx);
                }
            }
        }
    }
    
    auto tp = std::chrono::high_resolution_clock::now();
    auto path = graph.calculate_shortest_path(start, end);
    
    game::traffic::backgound::Time_table time_table{};
    time_table.add_trip(0, 3.0, path, graph);

    double time = std::chrono::duration<double>(std::chrono::high_resolution_clock::now() - tp).count();
    

    char** lines = new char*[width*2];
    for( int i=0;  i<width*2;  i++ ){
        lines[i] = new char[width*2+1];
        for(int j=0;  j<width*2;  j++)
            lines[i][j] = ' ';
        lines[i][width*2] = '\0';
    }

    for( int y=0;  y<width;  y++ ){
        for( int x=0;  x<width;  x++ ){
            lines[2*x][2*y] = '0';
        }
    }

    for( size_t i=0;  i<path.size()-1;  i++){

        int from = path[i];
        int to = path[i+1];

        int fx = from % width;
        int fy = from / width;
        int tx = to % width;
        int ty = to / width;

        int dx = tx - fx;
        int dy = ty - fy;
        lines[2*fy+dy][2*fx+dx] = dx==0?'|':'-';
    }

    for(int i=0;  i<width*2; i++){
        std::cout << lines[i] << "\n";
    }

    std::cout << "=================\\ time_table \\=================\n";
    
    time_table.do_all([&](game::traffic::backgound::Edge_idx edge, const std::vector<game::traffic::backgound::Time_table::Time_range>& tr){
        std::cout << "edge " << edge << " lenght: " << graph.edge(edge).lenght << " speed limit: " << graph.edge(edge).speed_limit << "\n";
        for( const auto& time_range : tr ){
            std::cout << "\t id: " << time_range.id << " enter: " << time_range.start << " leave: " << time_range.end << "\n";
        }
    });
    
    std::cout <<    "=======================\n     results       \n" <<
                    "=======================\n";
    std::cout << "edge count " << edge_count << "\n";
    std::cout << "node count " << node_count << "\n";
    std::cout << "it took " << time << " secs\n";

}




void map_test(int argc, char** argv){


    int start_idx=0;
    int end_idx = 100;
    if(argc > 0){
        start_idx = std::atoi(argv[0]);
    }
    if(argc > 1){
        end_idx = std::atoi(argv[1]);
    }

    using namespace game::traffic::backgound;
    using namespace game::npc;
    using namespace std;

    using clock = std::chrono::high_resolution_clock;
    using duration = std::chrono::duration<double>;

    
    auto time_point = clock::now();
    Graph graph = Graph::read_from_text_file("graph.txt");
    double time = duration(clock::now() - time_point).count();

    auto path = graph.calculate_shortest_path(start_idx, end_idx);

    for(auto& node : path ){
        cout << node << "\n";
    }

    cout << "========================\n time: " << time << "\tsec\n========================\n";

}




int main(int argc, char** argv){
    srand(std::time(NULL));


    if(argc < 2){
        std::cout << "graph or npc\n";
        return -1;
    }

    if(argv[1][0] == 'g'){
        bg_graph_bm(argc-1, argv+1);
    }else if(argv[1][0] == 'n'){
        npc_bm(argc-1, argv+1);
    }else if(argv[1][0] == 'm'){
        map_test(argc-2, argv+2); // different offset used :)
    }else{
        std::cout << "wtf\n";
    }
}







