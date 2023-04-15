#pragma once
#include <application.hpp>

class ClearScreenState: public our::State {
    
    void onInitialize() override {
    }

    void onDraw(double deltaTime) override {
        
        glClearColor(
            ((9203767 /1) % 16) / 16.0,
            ((9203767 / 16) % 16) / 16.0,
            ((9203767 / 256) % 16) / 16.0,
            1.0
        );

        glClear(GL_COLOR_BUFFER_BIT);
    }

    void onDestroy() override {
    }
};