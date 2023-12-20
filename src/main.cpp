#include <iostream>
#include "gl.h"

#include "window.h"
#include <thread>
#include "utils.h"
#include "controller.h"

int main() {
    std::shared_ptr<Queue<RendererWindow::input_event>> input_channel_renderer = std::make_shared<Queue<RendererWindow::input_event>>();
    std::shared_ptr<Queue<MainWindow::input_event>> input_channel_main = std::make_shared<Queue<MainWindow::input_event>>();

    std::shared_ptr<Renderer::renderer_data> renderer_data = std::make_shared<Renderer::renderer_data>();
    renderer_data->bgColor = glm::vec4(184.0f / 255.0f, 201.0f / 255.0f, 230.0f / 255.0f, 1.0f);

    MainWindow w1(input_channel_main, renderer_data);
    RendererWindow w2(input_channel_renderer, renderer_data);

    Controller controller(w1.output_channel, w1.input_channel, w2.input_channel, w2.output_channel, renderer_data);

    std::thread renderer_thread([&w2, &w1, &controller] {
        w2.initGL("renderer", 0);
        w1.initGL(1280, 970, "window");

        while(true) {
            int err;
            glfwPollEvents();
            bool w1_close = w1.render();
            if(w1_close) {
                break;
            }

            err = glGetError();
            if(err != 0) {
                std::cout << "w1errrorGL: " << std::hex << err << std::endl;
            }

            w2.render();

            err = glGetError();
            if(err != 0) {
                std::cout << "w2errrorGL: " << std::hex << err << std::endl;
            }
        }
        controller.stop();
    });

    controller.startLoopInCurrenThread();
    renderer_thread.join();
    
    return 0;
}