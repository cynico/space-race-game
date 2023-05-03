#include <iostream>
#include <glm/glm.hpp>
#include "glad/gl.h"
#include "shader/shader.hpp"
#include <map>

#include "./text-utils.hpp"

// This is for the FreeType font library.
#include <ft2build.h>
#include FT_FREETYPE_H  

namespace our {

    // Generate the bitmaps/textures of the characters.
    std::map<char, Character>* GenerateCharacterBitmaps() {

        // Creating the characters' map that will hold all the generated characters.
        std::map<char, Character>* Characters = new std::map<char, Character>;

        // First, we create and initialize the freetype library.
        FT_Library ft;
        if (FT_Init_FreeType(&ft)) {
            std::cerr << "ERROR: INITIALZATION OF THE FREETYPE LIBRARY FAILED." << std::endl;
            return NULL;
        }

        // Second, we load the font (called face) we desire to use.
        FT_Face face;
        // This takes the initialized library, the path to the TTF font, and a reference to the face. 
        if (FT_New_Face(ft, "assets/fonts/notoserif-regular.ttf", 0, &face)) {
            std::cerr << "ERROR: FAILED TO LOAD THE FONT FILE assets/fonts/notserif-regular.ttf" << std::endl;
            return NULL;
        }

        // Here, we set the desired size of the character.
        // We can set the height of the character, and let it choose a width corresponding to the height
        // by putting 0 in the width parameter. 
        FT_Set_Pixel_Sizes(face ,0, 48);

        // Here, we specify the alignment requirement for the start of each pixel in memory.
        // We want it to be byte-aligned, since each character in the texture will only have
        // one component of size 1 byte (which is the alpha component used later in the fragment shader).
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        
        // Generating the first 128 ASCII characters.
        for (unsigned char c = 0; c < 128; c++) {
            
            // First, we load the character glyph.
            // By setting FT_LOAD_RENDER as one of the loading flags, we tell FreeType
            // to create an 8-bit grayscale bitmap image for us that we can access through
            // face->glyph->bitmap
            if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
                std::cout << "ERROR::FREETYPE: Failed to load Glyph" << std::endl;
                continue;
            }

            // Generating the Texture of the current character, using the bitmap just generated
            // above.
            unsigned int texture;
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);

            // Here, we set the data of the generated texture. Note that we choose the format and 
            // the internal format to be GL_RED, because the texture will only contain 1 8-bit component
            // (GL_UNSIGNED_BYTE) for every pixel, which is the alpha or the opacity of that pixel.
            // If the pixel has an alpha=1, then it's part of the character, otherwise, it's part
            // of the background.

            // We can use bitmap.width and bitmap.rows as the width and height of the texture.
            // Finally, we supply bitmap.buffer as the data of the texture.
            glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RED,
                face->glyph->bitmap.width,
                face->glyph->bitmap.rows,
                0,
                GL_RED,
                GL_UNSIGNED_BYTE,
                face->glyph->bitmap.buffer
            );

            // Setting parameters of the texture.
            // TODO: For now, for the sake of simplicity, we won't use a sampler.
            // For the wrap around s and t axes, we choose to clamp to edge, that is,
            // if the texture doesn't fit the object, it's stretched to the edge across both
            // axes.

            // We also set the minimizing and magnifying filters to be GL_LINEAR.
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            // We encapsulate all of the related info into a character structure:
            // the generated texture, the size (width and height), the bearing and the advance.
            Character character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            static_cast<unsigned int>(face->glyph->advance.x)
            };

            // We insert the character into the map of characters.
            Characters->insert(std::pair<char, Character>(c, character));

        }

        // Unbinding any bound textures.
        glBindTexture(GL_TEXTURE_2D, 0);

        // Destroying the create face and library from FreeType, since we don't need 
        // them anymore after we've created all the desired characters textures/bitmaps.
        FT_Done_Face(face);
        FT_Done_FreeType(ft);

        return Characters;
    }

    // This function creates a text struct, and initializes/creates all of its data members.
    Text* CreateText(std::string displayedText, float scale, XPositioning xPosition, glm::vec3 color) {

        Text *generatedText = new Text();

        generatedText->displayedText = displayedText;
        generatedText->scale = scale;
        generatedText->color = color;
        generatedText->xPosition = xPosition;

        // Generating the vertex array, and the vertex buffer used for the text,
        // then tying/binding them to each other.         
        glGenVertexArrays(1, &generatedText->VAO);
        glGenBuffers(1, &generatedText->VBO);
        glBindVertexArray(generatedText->VAO);
        glBindBuffer(GL_ARRAY_BUFFER, generatedText->VBO);


        // First, we specify the size of the buffer to be 6*4 the size of float. The reasoning is as follows:
        // Each character is drawn on a quad. To draw a quad with OpenGL we need 6 vertices (two triangles).
        // Each vertex will have 4 floats: x and y coordinates (where it will be located on the screen), x and y text coordinates,
        // (where its pixel opacity will be taken from the texture).  

        // We specify the data of the buffer to be NULL for now, as this buffer will be used for multiple characters.
        // That is, we'll fill it with each character's data before rendering. 
        // That's also why we use GL_DYNAMIC_DRAW because the data of the buffer will be modified repeatedly.
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);

        // Enabling one vertex attribute pointer at (location = 0), which will have the 4 needed components
        // described above for each pixel (stride = size = 4*sizeof(float)).
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
        
        // Unbinding everything.
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        
        // Creating the shader program that will be used with the text.
        ShaderProgram *shader = new ShaderProgram();

        // Attaching the vertex shader and the fragment shader to the program.
        shader->attach("assets/shaders/text.vert", GL_VERTEX_SHADER);
        shader->attach("assets/shaders/text.frag", GL_FRAGMENT_SHADER);

        // Linking the program.
        if (!shader->link()) {
            std::cerr << "ERROR: FAILED TO LINK THE TEXT'S SHADER PROGRAM." << std::endl;
        }

        // Setting the shader of the text.
        generatedText->shader = shader;

        // Setting the pipeline state of the text: enabling blending. Every other blending
        // parameter is okay left as default.
        generatedText->textPipelineState.blending.enabled = true;

        return generatedText;
    }

    void RenderText(Text* text, glm::ivec2 windowSize, std::map<char, Character>*characters) {
        
        // First, we activate/use the shader program associated with the text component.
        text->shader->use();

        // Setting the text color uniform. This is the color that will be used to display text.
        text->shader->set("texColor", text->color);

        // Setting the projection matrix used within this program's vertex shader.
        // We'll use an orthographic projection matrix, as the text doesn't need a perspective
        // one. And then we can sepcify the text coordinates as screen coordinates.
        glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(windowSize.x), 0.0f, static_cast<float>(windowSize.y));
        text->shader->set("projection", projection);

        // Setting the texture used to be zero.
        // This is actually redundant, given that the default value of sampler2D in glsl is zero.
        // But we do it just in case.
        text->shader->set("text", 0);

        // Setting up the pipeline state (enabling blending).
        text->textPipelineState.setup();

        // Activating Texture 0, and binding to the vertex array.
        glActiveTexture(GL_TEXTURE0);
        glBindVertexArray(text->VAO);

        // Initialize the initial x and y position with the supplied values.
        float x;
        float y = windowSize.y - (80.0/1080.0)*windowSize.y;
        float scale = (text->scale / (1920.0*1080.0)) * (windowSize.x * windowSize.y);

        // If the text is desired to be centered (x-axis),
        // calculate total width of text, subtract it from the windows width,
        // divide by two to get the x position of the initial character.

        // If the text is desired to be right-aligned, calculate the total width
        // of the text, subtract it from the window's width, then subtract additional
        // padding.
        if (text->xPosition == our::CENTER || text->xPosition == our::RIGHT) {
            
            float totalTextWidth = 0;
            for (std::string::const_iterator c = text->displayedText.begin(); c != text->displayedText.end(); c++) {
                totalTextWidth += (*characters)[*c].Size.x * text->scale;
            }

            if (text->xPosition == our::CENTER) {
                x = (windowSize.x - totalTextWidth)/2.0;
            } else {
                x = (windowSize.x - totalTextWidth) - (50.0/1920) * (float)windowSize.x;
            }

        } else if (text->xPosition == our::LEFT) {
            x = (20.0/1920) * (float)windowSize.x;
        }

        // Looping over all characters of the string to render them.
        for (std::string::const_iterator c = text->displayedText.begin(); c != text->displayedText.end(); c++) {
            
            // Obtaining the first character's data of the displayed text.
            Character ch = (*characters)[*c];

            // Calculating the width and height of the character.
            // This is done by multiplying the size with the desired scale of the text. 
            float w = ch.Size.x * scale;
            float h = ch.Size.y * scale;

            // Calculating the positon of the origin of the character.
            // For better illustration, take a look at: https://learnopengl.com/img/in-practice/glyph.png 
            float xpos = x + ch.Bearing.x * scale;
            float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

            // The data of 6 vertices.
            // The position of every vertex is indicated with a comment.
            // Remember: coordinates in the textue start from the top left.
            // The coordinates we specify for the screen start from the bottom left.  
            float vertices[6][4] = {
                // Top left
                { xpos,     ypos + h,   0.0f, 0.0f },  

                // Bottom left          
                { xpos,     ypos,       0.0f, 1.0f },

                // Bottom right
                { xpos + w, ypos,       1.0f, 1.0f },
                
                // Top left
                { xpos,     ypos + h,   0.0f, 0.0f },

                // Botto right
                { xpos + w, ypos,       1.0f, 1.0f },

                // Top right
                { xpos + w, ypos + h,   1.0f, 0.0f } 
            };

            // Bind the texture of the character.
            glBindTexture(GL_TEXTURE_2D, ch.TextureID);

            // Bind to the VBO of the text to fill the data.
            // We fill the buffer's data with glBufferSubData() instead of glBufferData,
            // this is preferred when replacing the entire data store, than using glBufferData,
            // which re-allocates the data store.
            // Check this for more: https://registry.khronos.org/OpenGL-Refpages/gl4/html/glBufferSubData.xhtml 
            glBindBuffer(GL_ARRAY_BUFFER, text->VBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

            // After filling the buffer's data, we unbind it.
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            // Draw the vertices in the vertex array bound above (6 vertices for a quad).
            glDrawArrays(GL_TRIANGLES, 0, 6);

            x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
        }

        // Unbinding everything.
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);

    }
}