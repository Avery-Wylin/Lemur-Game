#ifndef TEXTURE_H
#define TEXTURE_H

#include <string>
#include "library/glad_common.h"
#include "FBO.h"



class Texture {
    public:

    static void unbind();

    Texture();
    ~Texture();

    void bind ( int active_texture );
    void load_png ( std::string filename, uint32_t scale_type = GL_NEAREST, uint32_t extention_type = GL_REPEAT, uint32_t format = GL_RGB );
    void from_FBO( FBO &fbo, unsigned int slot, uint32_t scale_type = GL_NEAREST );
    void allocate();
    bool is_allocated();
    void free();

    private:
    GLuint tid;

    // Forbid Copy
    Texture ( Texture const& );
    Texture& operator= ( Texture const& );

};

#endif /* TEXTURE_H */

