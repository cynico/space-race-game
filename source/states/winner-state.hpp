#pragma once

#include "play-state.hpp"
#include "extra-definitions.hpp"
#include <application.hpp>
#include <cstddef>
#include <material/material.hpp>
#include <texture/texture-utils.hpp>

class Winnerstate : public our::State {

    // Two textured materials for the two flags.
    our::TexturedMaterial* ussrMaterial, *usMaterial;

    // A rectangle mesh on which the winner material will be drawn
    our::Mesh* rectangle;

    // A variable to record the time since the state is entered (it will be used for the fading effect).
    float time;

    // A variable to record players' info to decide who the winner is.
    float timeUS, timeUSSR; // The time each took to finish the race.
    float collectedRatioUS, collectedRatioUSSR; // The ratio of the collected parts to the total number of parts for each.

    // A string holding the path of the song corresponding to the winner;
    std::string winnerSong;

    // This indicates whether the song has started playing or not, so that we don't start it all over every frame.
    bool songPlayed = false;

    void onInitialize() override {

        // Creating a mterial for both: USSR and US.
        ussrMaterial = new our::TexturedMaterial();
        usMaterial = new our::TexturedMaterial();

        // Here, we load the shader that will be used to draw the background
        ussrMaterial->shader = new our::ShaderProgram();
        ussrMaterial->shader->attach("assets/shaders/textured.vert", GL_VERTEX_SHADER);
        ussrMaterial->shader->attach("assets/shaders/textured.frag", GL_FRAGMENT_SHADER);
        ussrMaterial->shader->link();

        usMaterial->shader = new our::ShaderProgram();
        usMaterial->shader->attach("assets/shaders/textured.vert", GL_VERTEX_SHADER);
        usMaterial->shader->attach("assets/shaders/textured.frag", GL_FRAGMENT_SHADER);
        usMaterial->shader->link();

        // Then we load the textures
        ussrMaterial->texture = our::texture_utils::loadImage("assets/textures/ussr.jpg");
        usMaterial->texture = our::texture_utils::loadImage("assets/textures/us.jpg");

        // Initially, the material will be black, then it will fade in
        ussrMaterial->tint = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
        usMaterial->tint = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);


        // Then we create a rectangle whose top-left corner is at the origin and its size is 1x1.
        // Note that the texture coordinates at the origin is (0.0, 1.0) since we will use the 
        // projection matrix to make the origin at the the top-left corner of the screen.
        rectangle = new our::Mesh({
            {{0.0f, 0.0f, 0.0f}, {255, 255, 255, 255}, {0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
            {{1.0f, 0.0f, 0.0f}, {255, 255, 255, 255}, {1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
            {{1.0f, 1.0f, 0.0f}, {255, 255, 255, 255}, {1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
            {{0.0f, 1.0f, 0.0f}, {255, 255, 255, 255}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
        },{
            0, 1, 2, 2, 3, 0,
        });

        // Reset the time elapsed since the state is entered.
        time = 0;


    }

    void onDraw(double deltaTime) override {

        // Decide who won the race.
        our::TexturedMaterial *winnerMaterial = NULL;
        if (getApp()->getWinner() == our::USSR) {
            winnerMaterial = ussrMaterial;
            winnerSong = "assets/sounds/le-internationale.mp3";
        }
        else {
            winnerMaterial = usMaterial;
            winnerSong = "assets/sounds/never-gonna.mp3";
        }
        
        // Get the framebuffer size to set the viewport and the create the projection matrix.
        glm::ivec2 size = getApp()->getFrameBufferSize();
        // Make sure the viewport covers the whole size of the framebuffer.
        glViewport(0, 0, size.x, size.y);

        // The view matrix is an identity (there is no camera that moves around).
        // The projection matrix applys an orthographic projection whose size is the framebuffer size in pixels
        // so that the we can define our object locations and sizes in pixels.
        // Note that the top is at 0.0 and the bottom is at the framebuffer height. This allows us to consider the top-left
        // corner of the window to be the origin which makes dealing with the mouse input easier. 
        glm::mat4 VP = glm::ortho(0.0f, (float)size.x, (float)size.y, 0.0f, 1.0f, -1.0f);
        // The local to world (model) matrix of the background which is just a scaling matrix to make the menu cover the whole
        // window. Note that we defind the scale in pixels.
        glm::mat4 M = glm::scale(glm::mat4(1.0f), glm::vec3(size.x, size.y, 1.0f));

        // First, we apply the fading effect.
        time += (float)deltaTime;
        winnerMaterial->tint = glm::vec4(glm::smoothstep(0.00f, 2.00f, time));

        winnerMaterial->setup();
        winnerMaterial->shader->set("transform", VP*M);
        rectangle->draw();

        // Play the corresponding song.
        if (!songPlayed) {
            irrklang::ISound *music = getApp()->getSoundEngine()->play3D(winnerSong.c_str(), irrklang::vec3df(0,0,0), false, false, true);
            if (music)
                music->setMinDistance(5.0f);
            songPlayed = true;
        }

        // Check keyboard input
        auto& keyboard = getApp()->getKeyboard();
        if (keyboard.justPressed(GLFW_KEY_ESCAPE)) {
            getApp()->close();
        }

    }

    // Delete all the allocated resources
    void onDestroy() override {
        delete rectangle;
        
        delete ussrMaterial->texture;
        delete ussrMaterial->shader;
        delete ussrMaterial;

        delete usMaterial->texture;
        delete usMaterial->shader;
        delete usMaterial;
    }

};