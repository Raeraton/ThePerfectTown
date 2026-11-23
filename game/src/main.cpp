#ifndef TEST_APP_BENCHMARK


#include <chrono>
#include <iostream>

#include <render/my/app.hpp>


int main()
{

    typedef std::chrono::high_resolution_clock clock;

    try{

        init_main_app_instance();
        App* app = get_main_app_instance();

        app->load_content();

        clock::time_point time_point = clock::now();
        clock::time_point sec_time_point = clock::now();
        size_t frame_count=0;
        double deltatime = 0;
        while( app->running ){
            {
                clock::time_point temp_time = clock::now();
                deltatime = std::chrono::duration<double>( temp_time - time_point ).count();

                if( std::chrono::duration<double>( temp_time - sec_time_point ).count() >= 1 ){
                    sec_time_point = temp_time;
                    std::cout << "fps: " << frame_count << "\n";
                    frame_count = 0;
                }

                time_point = temp_time;
            }

            app->update(deltatime);
            app->display();

            frame_count ++;
        }

        app->terminate();
    }catch(const char* err){
        std::cerr << "[ERROR] " << err << "\n";
    }catch( std::string err ){
        std::cerr << "[ERROR] " << err << "\n";
    }catch( std::exception err ){
        std::cerr << "[std ERROR] " << err.what() << "\n";
    }

    return 0;
}

#endif