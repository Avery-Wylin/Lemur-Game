#include "library/glad.h"
#include "FBO.h"
#include <iostream>

static FBO* active;

FBO::FBO(){
    xResolution = 0;
    yResolution = 0;
    fboid = 0;
}

FBO::~FBO() {
    free();
}

/*
 * Creates the FBO on the GPU.
 * Nothing is performed if the FBO is already allcoated.
 */
void FBO::allocate(unsigned int width, unsigned int height){
    if(isAllocated())
        return;
    // Generate the framebuffer, bind it, and set the resoluton
    glGenFramebuffers(1, &fboid);
    glBindFramebuffer(GL_FRAMEBUFFER,fboid);
    xResolution = width;
    yResolution = height;
}

/*
 * Deletes the FBO on the GPU, including all attachments.
 */
void FBO::free(){
    if(!isAllocated())
        return;
    
    // Delete all color attachments
    for(unsigned int i = 0; i < FBO_MAX_COLOR_ATTACHMENTS; i++){
        if( colorRenderBuffers[i] == 0)
            continue;
        glDeleteRenderbuffers(1,&colorRenderBuffers[i]);
        colorRenderBuffers[i] = 0;
    }
    
    // Delete depth attachment
    if(depthAttachment != 0){
        glDeleteRenderbuffers(1,&depthAttachment);
        depthAttachment = 0;
    }
    
    // Delete the FBO
    glDeleteFramebuffers(1,&fboid);
    
}

/*
 * Returns whether or not the FBO is allocated on the GPU.
 */
bool FBO::isAllocated(){
    return fboid != 0;
}

/*
 * Binds the FBO, all drawing will be performed on this FBO.
 * Nothing will be performed if the FBO is unallocated.
 */
void FBO::bind(){
    if(!isAllocated())
        return;
    
    glBindFramebuffer(GL_FRAMEBUFFER, fboid);
    // Set the size of the viewport to the FBO's resolution
    glViewport(0, 0, xResolution, yResolution);
}

/*
 * Sets the FBO to the default one that renders to the screen.
 */
void FBO::bindDefault(unsigned int xres, unsigned int yres){
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, xres, yres);
}

/*
 * Creates a depth attachment for the FBO.
 * Nothing will be performed if the attachment exist or the FBO is unallocated.
 */
void FBO::createDepthAttachment(){
    if(depthAttachment != 0 || !isAllocated())
        return;
    bind();
    glGenRenderbuffers(1,&depthAttachment);
    glBindRenderbuffer(GL_RENDERBUFFER, depthAttachment);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, xResolution, yResolution);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthAttachment);
}

/*
 * Create a color attachment for the FBO.
 * Color attachments allow for rendering to multiple targets using:
 * layout (location = <slot#>) out <uniform>
 * A slot can be chosen up to MAX_COLOR_ATTACHMENTS.
 * Nothing will be performed if the attachment exist, the slot number is invalid, or the FBO is unallocated.
 */
void FBO::createColorAttachment(unsigned int slot){
    if(!(slot < FBO_MAX_COLOR_ATTACHMENTS && isAllocated() && colorRenderBuffers[slot] != 0))
        return;
    
    bind();
    glGenRenderbuffers(1,&colorRenderBuffers[slot]);
    glBindRenderbuffer(GL_RENDERBUFFER, colorRenderBuffers[slot]);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, xResolution, yResolution);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + slot, GL_RENDERBUFFER, colorRenderBuffers[slot]);
    
    // Update the attachments that the shader will draw to
    GLenum drawableBuffers[FBO_MAX_COLOR_ATTACHMENTS];

    for(unsigned int i = 0; i < FBO_MAX_COLOR_ATTACHMENTS; i++){
        if( colorRenderBuffers[i] == 0)
            drawableBuffers[i] = GL_NONE;
        else
            drawableBuffers[i] = GL_COLOR_ATTACHMENT0+i;
    }

    glDrawBuffers( FBO_MAX_COLOR_ATTACHMENTS, drawableBuffers);

}

GLuint FBO::getColorRenderBuffer (unsigned int slot){
    if(slot > FBO_MAX_COLOR_ATTACHMENTS )
        return 0;
    return colorRenderBuffers[slot];
}

void FBO::drawToDefault(unsigned int slot, unsigned int w, unsigned int h){
    if(!isAllocated() || slot >= FBO_MAX_COLOR_ATTACHMENTS )
        return;
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fboid);
    glReadBuffer(GL_COLOR_ATTACHMENT0+slot);
    glBlitFramebuffer(0,0,xResolution,yResolution,0,0,w,h,GL_COLOR_BUFFER_BIT,GL_LINEAR);
}

void FBO::copyTo(FBO &dest, uint32_t scaleType){
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dest.fboid);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fboid);
    glBlitFramebuffer(0, 0, xResolution, yResolution, 0, 0, dest.xResolution, dest.yResolution, GL_COLOR_BUFFER_BIT, scaleType);
}
