#ifndef TEXT_H
#define TEXT_H

#include <cglm/cglm.h>
#include "FontInfo.h"
#include <memory>
#include "../graphics/VAO.h"


class Text {
    public:

        Text();
        virtual ~Text();
        // Text(Text &a);

        uint32_t spacing = 20;
        bool needsGenerated = false;
        vec3 pos;
        float scale = 1;
        bool visible = true;

        void set_text(std::string &text);
        void set_text(const char *text, uint32_t length);
        std::string const& getText();
        void append(char c);
        void overwrite(char c);
        void insert(char c);
        void pop();
        void erase();
        void setCursor(int32_t position);
        void moveCursor(int32_t amount);
        void selectMore(int32_t amount);
        void selectAll();
        void deselectAll();
        bool isEmpty();
        void bind(FontInfo &font);
        inline void unbind(){vao->unbind();};
        inline uint32_t getVertexCount(){return vertexCount;};
        inline float getWidth(){return width;};
        inline float getHeight(){return height;};
        inline uint32_t getSelectionCount(){return selStop-selStart;};

    private:
        string text;
        int32_t selStart = 0;
        int32_t selStop = 0;
        void generate(FontInfo &font);

        // Use a shared pointer to allow copy/deletion of text
        // copies of text will share the same VAO, the VAO will be deleted once no copies exist
        std::shared_ptr<VAO> vao;
        uint32_t vertexCount = 0;
        float width, height;
};

#endif /* TEXT_H */

