#pragma once
#include "gl.h"
#include <string>
#include <atomic>
#include <memory>
#include "queue.h"
#include <variant>
#include "utils.h"
#include "renderer.h"
#include "slide.h"

extern "C" {
#include <ft2build.h>
#include FT_FREETYPE_H
}

class Window {
    public:
        virtual ~Window();
        bool render();
    public:
        int initGL(int width, int height, std::string name, GLFWmonitor* monitor, GLFWwindow* share);
        virtual void renderGL() = 0;
        virtual void postInit() = 0;
        virtual void onSizeChange();
        virtual void pollEvents() = 0;

        GLFWwindow* window;
        int width;
        int height;
};

class MainWindow: public Window {
    public:
        struct output_event {
            struct set_font_data {
                std::string name;
                int size;
            };
            enum {
                SET_FONT,
                GET_FONTS,
                LOAD_SLIDES,
                CHANGE_SLIDE, //relative value 1 means next, -1 means prev
                SET_SPACING
            } type;
            std::variant<
                set_font_data, //SET_FONT
                std::monostate, //GET_FONTS
                std::string, //LOAD_SLIDES
                int, //CHANGE_SLIDE
                glm::vec2 //SET_SPACING
            > data;
        };
        struct input_event {
            enum {
                FONTS_RESPONSE,
                RENDERER_SIZE_CHANGE,
                SLIDE,
                SET_SLIDES,
                SET_FONT_RENDERER
            } type;
            std::variant<
                std::vector<std::string>, //FONTS_RESPONSE
                glm::vec2, //RENDERER_SIZE_CHANGE
                int, //SLIDE
                std::vector<std::shared_ptr<Slide>>, //SET_SLIDES
                std::monostate //SET_FONT_RENDERER
            > data;
        };

        MainWindow(std::shared_ptr<Queue<MainWindow::input_event>> input_channel, std::shared_ptr<Renderer::renderer_data> renderer_data);
        ~MainWindow();
        void initGL(int width, int height, std::string name);
        
        std::shared_ptr<Queue<MainWindow::output_event>> output_channel;
        std::shared_ptr<Queue<MainWindow::input_event>> input_channel;

    protected:
        void renderGL();
        void postInit();
        void pollEvents();
    private:
        static std::atomic<bool> created;

        std::optional<int> currentSlide;
        std::vector<std::shared_ptr<Slide>> slides;

        std::string current_font;
        std::vector<std::string> fonts;
        int size = 200;

        std::shared_ptr<GL::FrameBuffer> FBO;
        std::unique_ptr<GL::FrameBufferRenderer> rendererFBO;
        Renderer renderer;
};

class RendererWindow: public Window {
    public:
        struct input_event {
            enum {
                FONT_CHANGE,
                SLIDE
            } type;
            std::variant<
                std::monostate, //FONT_CHANGE
                std::shared_ptr<Slide> //SLIDE
                > data;
        };
        struct output_event {
            enum {
                SIZE_CHANGE
            } type;
            std::variant<
                glm::vec2
                > data;
        };

        RendererWindow(std::shared_ptr<Queue<RendererWindow::input_event>> input_channel, std::shared_ptr<Renderer::renderer_data> renderer_data);
        ~RendererWindow();
        void initGL(std::string name, GLFWmonitor* monitor);

        std::shared_ptr<Queue<RendererWindow::input_event>> input_channel;
        std::shared_ptr<Queue<RendererWindow::output_event>> output_channel;
    public:
        void renderGL();
        void postInit();
        void onSizeChange();
        void pollEvents();

    private:
        Renderer renderer;

        std::optional<std::shared_ptr<Slide>> slide;
};

