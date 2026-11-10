#include <iostream>
#include <n_npc.hpp>
#include <ctime>
#include <chrono>


int main(int argc, char** argv){
try{
    srand(std::time(NULL));


    size_t population = 1000;
    size_t iteration = 120;
    if(argc > 1){
        population = std::atoll(argv[1]);
    }
    if(argc > 2){
        iteration = std::atoll(argv[2]);
    }


    auto tp = std::chrono::high_resolution_clock::now();
    game::npc::World world{population};
    auto tp2 =  std::chrono::high_resolution_clock::now();
    double dur1 = std::chrono::duration<double>(tp2 - tp).count();


    for( size_t _=0; _<iteration; _++ ){
        int day = (_ / 24) % 7;
        int hour = _ % 24;
        world.update(game::npc::Time_range{day, hour}, (_+1)%(168*4)==0);
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
