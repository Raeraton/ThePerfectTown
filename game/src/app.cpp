#include <iostream>
#include <vector>
#include <cmath>

#include <render/glad/gl.h>
#include <GLFW/glfw3.h>

#include <render/my/app.hpp>


struct {

    bool move_north=false, move_south=false, move_west=false, move_east=false;
    bool zoom_in=false, zoom_out=false;
    
    void update(GLFWwindow* window){
        move_north = glfwGetKey( window, GLFW_KEY_W ) == GLFW_PRESS;
        move_south = glfwGetKey( window, GLFW_KEY_S ) == GLFW_PRESS;
        move_west  = glfwGetKey( window, GLFW_KEY_A ) == GLFW_PRESS;
        move_east  = glfwGetKey( window, GLFW_KEY_D ) == GLFW_PRESS;
        zoom_in    = glfwGetKey( window, GLFW_KEY_Q ) == GLFW_PRESS;
        zoom_out   = glfwGetKey( window, GLFW_KEY_E ) == GLFW_PRESS;
    }

} Inputs;




class GameApp: public App{


    int window_width = 1920;
    int window_height = 1080;

    GLFWwindow* window = nullptr;


public:

    GameApp() : App() {

        // Initialize GLFW
        if( ! glfwInit() ){
            throw "glfwInit";
        }


        // Request OpenGL 3.3 Core
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


        // Create window
        window = glfwCreateWindow(
            window_width,
            window_height,
            "My OpenGL App",
            nullptr,
            nullptr
        );

        if (!window)
        {
            glfwTerminate();
            throw "glfwCreateWindow";
        }

        // Make OpenGL context current
        glfwMakeContextCurrent(window);


        // 4. Load OpenGL functions using GLAD
        int version = gladLoadGL(glfwGetProcAddress);

        if (version == 0)
        {
            throw "Failed to initialize GLAD";
        }

        std::cout << "OpenGL version: "
                << glGetString(GL_VERSION)
                << '\n';

        // Set viewport
        glViewport(0, 0, window_width, window_height);


        glEnable(GL_DEPTH_TEST);



    }

    void load_content() override {
      

    }

    void update( double deltatime )override {



    }

    void display(  ) override {

        // Clear screen
        glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        // TODO



        // Display rendered frame
        glfwSwapBuffers(window);

        // Process keyboard/mouse/window events
        glfwPollEvents();
        Inputs.update(window);

    }

    void terminate() override {
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    ~GameApp() override {
    }

};


App* main_app_instance = nullptr;
void init_main_app_instance(){
    if( main_app_instance != nullptr ){
        throw "whatafack  main_app_istance already initialised";
    }
    main_app_instance = new GameApp{};
}
App* get_main_app_instance(){
    if( main_app_instance == nullptr ){
        throw "whatafack  main_app_istance isnt initialised";
    }
    return main_app_instance;
}
