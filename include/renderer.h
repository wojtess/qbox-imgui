#pragma once
#include <map>
#include <glm/glm.hpp>
#include <memory>
#include "shader.h"
#include <optional>
#include "utils.h"
#include "gl.h"

extern "C" {
#include <ft2build.h>
#include FT_FREETYPE_H 
}

//https://learnopengl.com/In-Practice/Text-Rendering
struct Character {
    unsigned int TextureID;  // ID handle of the glyph texture
    glm::ivec2   Size;       // Size of glyph
    glm::ivec2   Bearing;    // Offset from baseline to left/top of glyph
    signed long Advance;    // Offset to advance to next glyph
};

class Renderer {
public:
    struct renderer_data {
        int width;
        int height;
        std::shared_ptr<FT_Face> face;
        glm::vec4 bgColor;
        float spacing_x;
        float spacing_y;
        int font_size;
    };

    Renderer(std::shared_ptr<renderer_data>);

    void initGL();
    void initCharactersGL();
    void setSize(int w, int h);
    int getWidth();
    int getHeight();
    void preRender();
    void postRenderer();

    void renderText(std::string text, float x, float y, float scale, glm::vec3 color);
    void renderTextWrappedCentered(std::string text, float scale, float spacing, glm::vec3 color);
    int calculateWidth(std::string);
    int calculateHeight(std::string);

    std::shared_ptr<renderer_data> internal_data;
private:
    std::optional<Shader> textShader;
    std::map<char, Character> characters;

    unsigned int VAO, VBO;

    GLint prevViewport[4];
};