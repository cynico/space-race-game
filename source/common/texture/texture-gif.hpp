#pragma once

#include "texture/texture2d.hpp"
#include <glad/gl.h>
#include <vector>

namespace our {

    // This class is for a gif texture.
    // It simply is a wrapper of a vector of 2d textures.
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