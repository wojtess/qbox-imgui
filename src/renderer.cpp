#include "renderer.h"
#include "gl.h"
#include <string>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

const char textShaderVS[] = R"(
#version 330 core
layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>
out vec2 TexCoords;

uniform mat4 projection;

void main()
{
    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
    TexCoords = vertex.zw;
}
)";

const char textShaderFS[] = R"(
#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D text;
uniform vec3 textColor;

void main()
{    
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);
    color = vec4(textColor, 1.0) * sampled;
}
)";

Renderer::Renderer(std::shared_ptr<renderer_data> renderer_data): internal_data(renderer_data) {
    
}

void Renderer::initGL() {
    int err = glGetError();
    if(err != 0) {
        std::cout << "renderer pre errrorGL: " << std::hex << err << std::endl;
    }

    this->textShader = Shader(textShaderVS, textShaderFS);
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(internal_data->width), 0.0f, static_cast<float>(internal_data->height));
    (*this->textShader).use();
    glUniformMatrix4fv(glGetUniformLocation((*this->textShader).ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    err = glGetError();
    if(err != 0) {
        std::cout << "renderer post errrorGL: " << std::hex << err << std::endl;
    }
}

void Renderer::preRender() {
    if(this->textShader) {
        glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(internal_data->width), 0.0f, static_cast<float>(internal_data->height));
        (*this->textShader).use();
        glUniformMatrix4fv(glGetUniformLocation((*this->textShader).ID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    }
    glGetIntegerv(GL_VIEWPORT, prevViewport);
    glViewport(0, 0, internal_data->width, internal_data->height);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(internal_data->bgColor.x, internal_data->bgColor.y, internal_data->bgColor.z, internal_data->bgColor.w);
    glClear(GL_COLOR_BUFFER_BIT);
}

void Renderer::postRenderer() {
    glViewport(prevViewport[0], prevViewport[1], prevViewport[2], prevViewport[3]);
    glDisable(GL_BLEND);
}

void Renderer::renderText(std::string text, float x, float y, float scale, glm::vec3 color) {
    if(textShader) {
        (*this->textShader).use();
        glUniform3f(glGetUniformLocation((*this->textShader).ID, "textColor"), color.x, color.y, color.z);
        glActiveTexture(GL_TEXTURE0);
        glBindVertexArray(VAO);

        for(char& c:text) {
            if(c == ' ') {
                x += internal_data->spacing_x;
            } else if(this->characters.count(c) != 0) {
                auto ch = this->characters[c];

                float xpos = x + ch.Bearing.x * scale;
                float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

                float w = ch.Size.x * scale;
                float h = ch.Size.y * scale;
                // update VBO for each character
                float vertices[6][4] = {
                    { xpos,     ypos + h,   0.0f, 0.0f },            
                    { xpos,     ypos,       0.0f, 1.0f },
                    { xpos + w, ypos,       1.0f, 1.0f },

                    { xpos,     ypos + h,   0.0f, 0.0f },
                    { xpos + w, ypos,       1.0f, 1.0f },
                    { xpos + w, ypos + h,   1.0f, 0.0f }           
                };
                // render glyph texture over quad
                glBindTexture(GL_TEXTURE_2D, ch.TextureID);
                // update content of VBO memory
                glBindBuffer(GL_ARRAY_BUFFER, VBO);
                glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); 
                glBindBuffer(GL_ARRAY_BUFFER, 0);
                // render quad
                glDrawArrays(GL_TRIANGLES, 0, 6);
                // now advance cursors for next glyph (note that advance is number of 1/64 pixels)

                x += (ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64)
            }
        }
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

void Renderer::renderTextWrappedCentered(std::string text, float scale, float spacing, glm::vec3 color) {
    float x = 0;
    std::vector<std::string> lines;
    std::string currentLine;
    float y = 0;
    auto splited = split(text, ' ');
    for(const auto& word:splited) {
        int width = calculateWidth(word);
        if(width + x <= getWidth() - spacing) {
            x += width;
            if(!currentLine.empty()) {
                currentLine += " ";
            }
            currentLine += word;
            //all good
        } else {
            //new line
            lines.push_back(currentLine);
            y += calculateHeight(currentLine);
            y += internal_data->spacing_y;

            currentLine = word;
            x = 0;
        }
    }
    lines.push_back(currentLine);
    y += calculateHeight(currentLine);
    
    int centeredY = getHeight() / 2 + y / 2;

    for(const auto& line:lines) {
        int line_width = calculateWidth(line);
        int line_height = calculateHeight(line);
        int centerX = getWidth() / 2;
        
        centeredY -= line_height;

        renderText(line, centerX - line_width / 2, centeredY, 1.0f, color);
        centeredY -= internal_data->spacing_y;
    }
}

void Renderer::setSize(int w, int h) {
    internal_data->width = w;
    internal_data->height = h;
}

void Renderer::initCharactersGL() {
    //remove all remaning characters
    for(auto it=this->characters.cbegin(); it != characters.cend();) {
        auto texID = (*it).second.TextureID;
        glDeleteTextures(1, &texID);
        it = characters.erase(it);
    }

    if(textShader) {
        (*this->textShader).use();

        auto face = internal_data->face;

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        for(unsigned char c = 0; c < 128; c++) {
            if(FT_Load_Char(*face, c, FT_LOAD_RENDER)) {
                std::cout << "err while loading char (int)" << (int)c << " skipping" << std::endl;
                continue;
            }
            unsigned int texture;
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RED,
                (*face)->glyph->bitmap.width,
                (*face)->glyph->bitmap.rows,
                0,
                GL_RED,
                GL_UNSIGNED_BYTE,
                (*face)->glyph->bitmap.buffer
            );
            // set texture options
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            // now store character for later use
            Character character = {
                texture,
                glm::ivec2((*face)->glyph->bitmap.width, (*face)->glyph->bitmap.rows),
                glm::ivec2((*face)->glyph->bitmap_left, (*face)->glyph->bitmap_top),
                static_cast<unsigned int>((*face)->glyph->advance.x)
            };
            this->characters.insert(std::pair<char, Character>(c, character));
        }
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

int Renderer::getHeight() {
    return internal_data->height;
}

int Renderer::getWidth() {
    return internal_data->width;
}

int Renderer::calculateWidth(std::string text) {
    int all = 0;
    for(char& c:text) {
        if(characters.count(c) != 0) {
            auto character = characters[c];
            all += (character.Advance >> 6);
        } else if(c == ' ') {
            all += internal_data->spacing_x;
        }
    }
    return all;
}

int Renderer::calculateHeight(std::string text) {
    int max = 0;
    for(char& c:text) {
        if(characters.count(c) != 0) {
            auto character = characters[c];
            if(character.Size.y > max) {
                max = character.Size.y;
            }
        }
    }
    return max;
}