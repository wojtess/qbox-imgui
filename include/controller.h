#pragma once
#include <iostream>
#include <window.h>
#include <renderer.h>

extern "C" {
#include <ft2build.h>
#include FT_FREETYPE_H
}

class Controller {
public:
    Controller(
        std::shared_ptr<Queue<MainWindow::output_event>> output_channel_main,
        std::shared_ptr<Queue<MainWindow::input_event>> main_input_channel,
        std::shared_ptr<Queue<RendererWindow::input_event>> input_channel_renderer,
        std::shared_ptr<Queue<RendererWindow::output_event>> output_channel_renderer,
        std::shared_ptr<Renderer::renderer_data> renderer_data
    );
    void stop();
    void startLoopInCurrenThread();
private:
    FT_Library ft;

    std::shared_ptr<Queue<MainWindow::output_event>> output_channel_main;
    std::shared_ptr<Queue<MainWindow::input_event>> input_channel_main;
    std::shared_ptr<Queue<RendererWindow::input_event>> input_channel_renderer;
    std::shared_ptr<Queue<RendererWindow::output_event>> output_channel_renderer;
    
    bool working = true;

    std::vector<std::shared_ptr<Slide>> slides;
    int currentSlide;

    std::shared_ptr<Renderer::renderer_data> renderer_data;
};