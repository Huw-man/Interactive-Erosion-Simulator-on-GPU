#include <GL/glew.h>
#include <string>

#include "common/texture.hpp"
#ifndef IMAGE_TEXTURE_HPP
#define IMAGE_TEXTURE_HPP


// Load a texture
struct ImageTexture {
    GLuint image, normals;
    ImageTexture(std::string name) {
        image = loadDDS((name+".dds").c_str());
        normals = loadDDS((name+"_norms.dds").c_str());
    }
};


#endif // IMAGE_TEXTURE_HPP