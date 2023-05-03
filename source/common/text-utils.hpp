#pragma once

#include <map>
#include <glm/glm.hpp>
#include "./shader/shader.hpp"
#include "material/pipeline-state.hpp"

namespace our {

    // This enum dictates how the text is placed on the x-axis (horizontally on the screen).
    // It can be to the right, the center, or the left.
    // The handling and the calculations are in the RenderText()
    enum XPositioning {
        RIGHT,
        CENTER,
        LEFT
    };

    // This struct holds all character-related information.
    struct Character {
        
        unsigned int TextureID;     // OpenGL's name of the character's texture.
        
        glm::ivec2   Size;          // Size of glyph
        
        glm::ivec2   Bearing;       // Offset from baseline to left/top of glyph
        
        unsigned int Advance;       // Offset to advance to next glyph
    };

    // This struct holds the necessary data members needed to render text.
    struct Text {
        
        // The vertex array object used in rendering the text's characters.
        unsigned int VAO;
        
        // The vertext buffer object used in storing the text's chracters while rendering.
        unsigned int VBO;
        
        // The shader program, to which assets/shaders/text.vert and assets/shaders/text.frag are attached.
        ShaderProgram* shader;

        // The string of the displayed text.
        std::string displayedText;
        
        // The color of the displayed text.
        glm::vec3 color = glm::vec3(1.0);
        
        // The scale/size of the displayed text.
        float scale;

        // Pipeline state defining the needed state (mainly so that blending is enabled).
        PipelineState textPipelineState;

        // This dictates whether the text should be right, center, or left.
        XPositioning xPosition;
    };

    // Generate the bitmaps/textures of the characters.
    std::map<char, Character>* GenerateCharacterBitmaps();

    // This function creates a text struct, and initializes/creates all of its data members.
    Text* CreateText(std::string displayedText, float scale, XPositioning xPosition, glm::vec3 color = glm::vec3(1.0f));

    void RenderText(Text* text, glm::ivec2 windowSize, std::map<char, Character>*characters);
}