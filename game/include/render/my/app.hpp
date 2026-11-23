#ifndef APP_HPP
#define APP_HPP



class App{
public:

    bool running = true; // only update on the update function and its main thread

    App() = default;
    
    virtual void load_content(){};

    // update loop in this order
    virtual void update( double deltatime ){};
    virtual void display(  ){};
    // update loop end

    virtual void terminate(){};

    virtual ~App() = default;

};

void init_main_app_instance();
App* get_main_app_instance();


#endif