#pragma once
#include "glad/gl.h"
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "shader.h"
#include <memory>

namespace GL {

class FrameBuffer {
public:
    FrameBuffer(int w, int h);
    ~FrameBuffer();
    void setSize(int w, int h);
    void bind();
    unsigned int getTex();
private:

    unsigned int id;
    unsigned int rbo;
    unsigned int texture;

    int width, height;
};

class FrameBufferRenderer {
public:
    FrameBufferRenderer(std::shared_ptr<FrameBuffer> framebuffer);
    ~FrameBufferRenderer();
    void render();
private:
    Shader shader;

    std::shared_ptr<FrameBuffer> framebuffer;

    unsigned int VAO, VBO;
};

}