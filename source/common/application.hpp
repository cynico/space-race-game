#pragma once

#include <glm/vec2.hpp>
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <imgui.h>

#include <string>
#include <unordered_map>
#include <type_traits>
#include <json/json.hpp>

#include "input/keyboard.hpp"
#include "input/mouse.hpp"

#include <iostream>
#include <map>

// This is for the sound library.
#include <irrKlang.h>
#include <conio.h>

#include "../states/playstate-type.hpp"
#include "./text-utils.hpp"

namespace our {

    // This struct handles window attributes: (title, size, isFullscreen).
    struct WindowConfiguration {
        std::string title;
        glm::i16vec2 size;
        bool isFullscreen;
    };

    // This struct holds the stats related to a player.
    struct Player {
        bool finished = false;
        float time;
        float collectedRatio;
    };

    class Application; // Forward declaration

    // This is the base class for all states
    // The application will be responsible for managing all scene functionality by calling the "on*" functions.
    class State {
        // Each scene will have a pointer to the application that owns it
        Application* application;
        friend Application;
    protected:
        int type;
    public:
        virtual void onInitialize(){}                   // Called once before the game loop.
        virtual void onImmediateGui(){}                 // Called every frame to draw the Immediate GUI (if any).
        virtual void onDraw(double deltaTime){}         // Called every frame in the game loop passing the time taken to draw the frame "Delta time".
        virtual void onDestroy(){}                      // Called once after the game loop ends for house cleaning.


        // Override these functions to get mouse and keyboard event.
        virtual void onKeyEvent(int key, int scancode, int action, int mods){}      
        virtual void onCursorMoveEvent(double x, double y){}
        virtual void onCursorEnterEvent(int entered){}
        virtual void onMouseButtonEvent(int button, int action, int mods){}
        virtual void onScrollEvent(double x_offset, double y_offset){}

        // Return type of state.
        // This could be useful where there are two play states (for two players), two menu states (one pause, and one on initialization, etc).
        int returnType() {
            return type;
        };

        //Returns a pointer
        Application* getApp() { return application; }
    };

    // This class act as base class for all the Applications covered in the examples.
    // It offers the functionalities needed by all the examples.
    class Application {
    protected:
        GLFWwindow * window = nullptr;      // Pointer to the window created by GLFW using "glfwCreateWindow()".
        
        Keyboard keyboard;                  // Instance of "our" keyboard class that handles keyboard functionalities.
        Mouse mouse;                        // Instance of "our" mouse class that handles mouse functionalities.

        nlohmann::json app_config;           // A Json file that contains all application configuration

        std::unordered_map<std::string, State*> states;   // This will store all the states that the application can run
        State * currentState = nullptr;         // This will store the current scene that is being run
        State * nextState = nullptr;            // If it is requested to go to another scene, this will contain a pointer to that scene

        // Sound Engine.
        irrklang::ISoundEngine* engine;

        // An enum variable that is after decision of the winner.
        our::PlaystateType winner;

        // Two players.
        our::Player US, USSR;

        // Map containing all character maps.
        std::map<char, Character> *Characters = NULL;

        // Virtual functions to be overrode and change the default behaviour of the application
        // according to the example needs.
        virtual void configureOpenGL();                             // This function sets OpenGL Window Hints in GLFW.
        virtual WindowConfiguration getWindowConfiguration();       // Returns the WindowConfiguration current struct instance.
        virtual void setupCallbacks();                             // Sets-up the window callback functions from GLFW to our (Mouse/Keyboard) classes.

    public:

        // Create an application with following configuration
        Application(const nlohmann::json& app_config) : app_config(app_config) {
            engine = irrklang::createIrrKlangDevice();
            if (!engine)
                std::cerr << "ERROR: creating a sound engine failed." << std::endl;
        }
        // On destruction, delete all the states
        ~Application() { 
            for (auto &it : states) 
                delete it.second; 
            
            engine->drop();
        }

        // This is the main class function that run the whole application (Initialize, Game loop, House cleaning).
        int run(int run_for_frames = 0);

        // Register a state for use by the application
        // The state is uniquely identified by its name
        // If the name is already used, the old name owner is deleted and the new state takes its place
        template<typename T>
        void registerState(std::string name){
            static_assert(std::is_base_of<State, T>::value, "T must derive from our::State");
            auto it = states.find(name);
            if(it != states.end()){
                delete it->second;
            }
            State* scene = new T();
            scene->application = this;
            states[name] = scene;

            // If this is a play state, set its type to the corresponding type.
            if (name == "play-USSR") {
                scene->type = (int)our::USSR;
            } else if (name == "play-US") {
                scene->type = (int)our::US;
            }
        }

        // Tells the application to change its current state
        // The change will not be applied until the current frame ends
        void changeState(std::string name){
            auto it = states.find(name);
            if(it != states.end()){
                nextState = it->second;
            }
        }

        // This is an overloaded function of the above function, used when a search has already been conducted,
        // and the desired state was found.
        void changeState(our::State* nextState) {
            this->nextState = nextState;
        }

        // Closes the Application
        void close(){
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }

        // Class Getters.
        GLFWwindow* getWindow(){ return window; }
        [[nodiscard]] const GLFWwindow* getWindow() const { return window; }
        Keyboard& getKeyboard() { return keyboard; }
        [[nodiscard]] const Keyboard& getKeyboard() const { return keyboard; }
        Mouse& getMouse() { return mouse; }
        [[nodiscard]] const Mouse& getMouse() const { return mouse; }

        [[nodiscard]] const nlohmann::json& getConfig() const { return app_config; }

        // Get the size of the frame buffer of the window in pixels.
        glm::ivec2 getFrameBufferSize() {
            glm::ivec2 size;
            glfwGetFramebufferSize(window, &(size.x), &(size.y));
            return size;
        }

        // Get the window size. In most cases, it is equal to the frame buffer size.
        // But on some platforms, the framebuffer size may be different from the window size.
        glm::ivec2 getWindowSize() {
            glm::ivec2 size;
            glfwGetWindowSize(window, &(size.x), &(size.y));
            return size;
        }

        irrklang::ISoundEngine* getSoundEngine() {
            return engine;
        }

        // This function changes states after a player has finished their turn.
        // If both have finished, it changes the state to the winner state, which displays who won.
        void takeTurns() {
            
            // If both player have finished.
            if (US.finished && USSR.finished) {

                // Find the winner state.
                State* winnerState;
                auto it = states.find("winner");
                if(it == states.end()){
                    std::cerr << "ERROR: CAN'T FIND THE WINNER STATE." << std::endl;
                    return;
                }
                winnerState = it->second;

                // Decide the winner of the race.
                this->decideWinner();

                // Change the state to the winner state.
                this->changeState(winnerState);
                
            } 
            else if (US.finished)
                this->changeState("play-USSR");
            else
                this->changeState("play-US");
        }
        
        // A function that returns the winner, used in the Winnerstate.
        our::PlaystateType getWinner() {
            return winner;
        }

        // This function decides who have won the race, depending on different variables.
        void decideWinner() {     
            winner = (US.time < USSR.time) ? our::US : our::USSR;
        }

        // This function is called from the player states to set the stats related to a player, later used in deciding the winner.
        // This sets the time the player took for the race, the ratio of the collected parts, etc.
        void setPlayerStats(our::PlaystateType who, float time, float collectedRatio) {
            if (who == our::US) {
                US.time = time;
                US.collectedRatio = collectedRatio;
                US.finished = true;
            } else {
                USSR.time = time;
                USSR.collectedRatio = collectedRatio;
                USSR.finished = true;
            }
        }

        // This function returns the characters' map of textures, used by the text component.
        std::map<char, Character>* getCharacterMap() {
            
            // If we haven't already generated the characters' textures/bitmaps before.
            if (!Characters) {
                Characters = our::GenerateCharacterBitmaps();
            }

            return Characters;
        }
    };
}
