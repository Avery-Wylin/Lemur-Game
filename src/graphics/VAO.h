#ifndef VAO_H
#define VAO_H

#include "library/glad_common.h"
#include "Shader.h"
#include <string>

class VAO {
public:
    
    GLuint vaoid = 0;
    GLuint vboids[Attribute::NUM_ATTRBS];
    GLuint iboid = 0;
    
    VAO();
    virtual ~VAO();

    static void unbind();

    void allocate();
    void free();
    void loadAttributeFloat(int attrbid, int vertexOffset, int divisor, int vecSize, int size, void* data);
    void loadAttributeByte(int attrbid, int vertexOffset, int divisor, int vecSize, int size, bool convert_float, void* data);
    void loadIndex(int numIndices, GLuint* data);
    void bind();
    int getIndexCount();
    void loadPLY(std::string filename);


private:
    VAO(VAO const&);
    VAO& operator=(VAO const&);
    int indexCount = 0;
    int vboSizes[Attribute::NUM_ATTRBS];
};

#endif /* VAO_H */

