#pragma once

#include "texture/texture2d.hpp"
#include <glad/gl.h>
#include <vector>

namespace our {

    class GIFTexture {
    public:
        std::vector<Texture2D*> textures;
        GIFTexture() = default;

        ~GIFTexture() { 
            for (auto it = textures.begin(); it != textures.end(); it++) delete *it;
        }

        GIFTexture(const GIFTexture&) = delete;
        GIFTexture& operator=(const GIFTexture&) = delete;
    };
    
}