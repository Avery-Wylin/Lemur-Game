#ifndef FBO_H
#define FBO_H

#include "definitions.h"
#include <stdlib.h>


class FBO {
public:
    FBO();
    virtual ~FBO();
    
    static void bindDefault(unsigned int xres, unsigned int yres);
    
    void allocate(unsigned int width, unsigned int height);
    bool isAllocated();
    void free();
    void bind();
    void createDepthAttachment();
    void createColorAttachment(unsigned int slot);
    const GLuint& getFboid(){return fboid;};
    GLuint getColorRenderBuffer(unsigned int slot);
    unsigned int getWidth(){return xResolution;};
    unsigned int getHeight(){return yResolution;};
    void drawToDefault(unsigned int slot, unsigned int w, unsigned int h);
    void copyTo(FBO &dest, uint32_t scaleType);

private:
    unsigned int xResolution, yResolution;
    GLuint fboid = 0;
    GLuint colorRenderBuffers[FBO_MAX_COLOR_ATTACHMENTS] = {0,0,0,0,0,0,0,0};
    GLuint depthAttachment = 0;
    
    // Forbid copy
    FBO(const FBO&);
    FBO operator=(const FBO&);
    
};

#endif /* FBO_H */

