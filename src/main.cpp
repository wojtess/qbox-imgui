#include <iostream>
#include "gl.h"


#include "window.h"
#include <thread>
#include "utils.h"
#include "controller.h"

void GLAPIENTRY
MessageCallback( GLenum source,
                 GLenum type,
                 GLuint id,
                 GLenum severity,
                 GLsizei length,
                 const GLchar* message,
                 const void* userParam )
{
  fprintf( stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
           ( type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "" ),
            type, severity, message );
}

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

        glEnable              ( GL_DEBUG_OUTPUT );
        glDebugMessageCallback( MessageCallback, 0 );

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