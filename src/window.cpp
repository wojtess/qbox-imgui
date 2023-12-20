#include "window.h"
#include <iostream>

#include "ImGuiFileDialog/ImGuiFileDialog.h"

int Window::initGL(int width, int height, std::string name, GLFWmonitor* monitor, GLFWwindow* share) {
    if(!glfwInit()) {
        std::cout << "Error initiazling glfw" << std::endl;
        return -1;
    }

    this->window = glfwCreateWindow(width, height, name.c_str(), monitor, share);
    glfwMakeContextCurrent(this->window);

    int version = gladLoadGL(glfwGetProcAddress);
    printf("loaded GL %d.%d\n", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));
    if(version == 0) {
        return -2;
    }

    this->width = width;
    this->height = height;
    this->onSizeChange();

    glfwSetWindowUserPointer(this->window, this);

    glfwSetFramebufferSizeCallback(this->window, [] (GLFWwindow* window, int width, int height) {
        Window* windowInstance = static_cast<Window*>(glfwGetWindowUserPointer(window));
        if(windowInstance != 0) {
            windowInstance->width = width;
            windowInstance->height = height;
            windowInstance->onSizeChange();
            glfwMakeContextCurrent(window);
            glViewport(0, 0, width, height);
        }
    });

    //post init
    this->postInit();
    return 0;
}

void Window::onSizeChange() {}//empty

bool Window::render() {
    if(glfwWindowShouldClose(window)) {
        return true;
    }

    glfwMakeContextCurrent(window);

    this->pollEvents();

    this->renderGL();

    glfwSwapBuffers(window);    

    return false;
}

Window::~Window() {
    glfwSetWindowUserPointer(this->window, 0);
    glfwTerminate();
}


std::atomic<bool> MainWindow::created(false); 

MainWindow::MainWindow(std::shared_ptr<Queue<MainWindow::input_event>> input_channel, std::shared_ptr<Renderer::renderer_data> renderer_data):input_channel(input_channel), renderer(renderer_data) {
    if(this->created) {
        //throw excpetion
        std::cout << "main window created twice" << std::endl;
        return;
    }
    this->output_channel = std::make_unique<Queue<MainWindow::output_event>>();

    created = true;
}

void MainWindow::initGL(int width, int height, std::string name) {
    Window::initGL(width, height, name, 0, 0);
    renderer.initGL();
}

MainWindow::~MainWindow() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    created = false;
}

void MainWindow::pollEvents() {
    //handle events
    if(std::optional<input_event> e = this->input_channel->get()) {
        // std::cout << "e->type: " << e->type << std::endl;
        switch (e->type) {
            case input_event::FONTS_RESPONSE: {
                this->fonts = std::get<std::vector<std::string>>(e->data);
                break;
            }
            case input_event::RENDERER_SIZE_CHANGE: {
                auto size = std::get<glm::vec2>(e->data);
                if(FBO == nullptr) {
                    FBO = std::make_shared<GL::FrameBuffer>(size.x, size.y);
                    rendererFBO = std::make_unique<GL::FrameBufferRenderer>(FBO);
                } else {
                    FBO->setSize(size.x, size.y);
                }
                break;
            }
            case input_event::SLIDE: {
                this->currentSlide = std::get<int>(e->data);
                break;
            }
            case input_event::SET_SLIDES: {
                this->slides = std::get<std::vector<std::shared_ptr<Slide>>>(e->data);
                break;
            }
            case input_event::SET_FONT_RENDERER: {
                this->renderer.initCharactersGL();
                break;
            }
        }
    }
}

void MainWindow::renderGL() {
    if(FBO != nullptr) {
        FBO->bind();

            renderer.preRender();
            if(currentSlide) {
                slides[*currentSlide]->render(renderer);
            }
            renderer.postRenderer();

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::ShowDemoWindow();

    if(ImGui::Begin("Output")) {
        float ratio = (float)renderer.getWidth() / (float)renderer.getHeight();

        auto windowSize = ImGui::GetContentRegionAvail();
        float y;
        float x;
        float calculated_y = windowSize.y * ratio;
        float calculated_x = windowSize.x * 1.0/ratio;
        
        if(std::max(calculated_x, windowSize.x) < std::max(calculated_y, windowSize.y)) {
            y = windowSize.x * 1.0/ratio;
            x = windowSize.x;
        } else if(std::max(calculated_x, windowSize.x) > std::max(calculated_y, windowSize.y)) {
            y = windowSize.y;
            x = windowSize.y * ratio;
        } else {
            x = windowSize.x;
            y = windowSize.y;
        }

        if(FBO != nullptr)
            ImGui::Image((void*)(intptr_t)FBO->getTex(), ImVec2(x,y), {0, 1}, {1, 0});
    }
    ImGui::End();

    if(ImGui::Begin("Slides")) {
        if(ImGui::Button("Select file")) {
            ImGuiFileDialog::Instance()->OpenDialog("FileSlidesChoose", "Choose File", ".*", ".");
        }
        if(ImGuiFileDialog::Instance()->Display("FileSlidesChoose", ImGuiWindowFlags_NoCollapse, ImVec2(100, 100))) {
            if (ImGuiFileDialog::Instance()->IsOk()) {
                std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
                
                output_channel->put(output_event{
                    output_event::LOAD_SLIDES,
                    filePathName
                });
            }
            ImGuiFileDialog::Instance()->Close();
        }
        if(ImGui::Button("prev")) {
            output_channel->put(output_event{
                output_event::CHANGE_SLIDE,
                -1
            });
        }
        ImGui::SameLine();
        if(ImGui::Button("next")) {
            output_channel->put(output_event{
                output_event::CHANGE_SLIDE,
                +1
            });
        }
    }
    ImGui::End();

    if(ImGui::Begin("Settings")) {
        if(ImGui::CollapsingHeader("Font")) {
            int size = this->size;
            ImGui::InputInt("Font size", &size);
            if(this->size != size && !this->current_font.empty()) {
                this->output_channel->put(output_event{
                    output_event::SET_FONT,
                    output_event::set_font_data{
                        this->current_font,
                        size
                    }
                });
            }
            this->size = size;

            //render fonts
            if(ImGui::BeginCombo("Font selector", this->current_font.c_str())) {   
                //request for fonts
                this->output_channel->put(output_event{
                    output_event::GET_FONTS
                });

                for(const auto& font:this->fonts) {
                    bool selected = false;
                    if(font.compare(this->current_font) == 0) {
                        selected = true;
                    }
                    if(ImGui::Selectable(font.c_str(), selected)) {
                        this->current_font = font;
                        //send event
                        this->output_channel->put(output_event{
                            output_event::SET_FONT,
                            output_event::set_font_data{
                                this->current_font,
                                size
                            }
                        });
                    }
                    if(selected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }

            float spacing[2] = {
                renderer.internal_data->spacing_x,
                renderer.internal_data->spacing_y
            };
            ImGui::InputFloat2("Spacing", spacing, "%.1f");


            if(spacing[0] != renderer.internal_data->spacing_x || spacing[1] != renderer.internal_data->spacing_y) {
                output_channel->put(output_event{
                    output_event::SET_SPACING,
                    glm::vec2(spacing[0], spacing[1])
                });
            }

        }
    }
    ImGui::End();

    glClearColor(0.7f, 0.9f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

}

void MainWindow::postInit() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // IF using Docking Branch

    ImGui_ImplGlfw_InitForOpenGL(window, true);          // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
    ImGui_ImplOpenGL3_Init();
}


RendererWindow::RendererWindow(std::shared_ptr<Queue<RendererWindow::input_event>> input_channel, std::shared_ptr<Renderer::renderer_data> renderer_data): input_channel(input_channel), renderer(renderer_data) {
    output_channel = std::make_shared<Queue<RendererWindow::output_event>>();
}

void RendererWindow::initGL(std::string name, GLFWmonitor* monitor) {
    Window::initGL(1280, 720, name, monitor, 0);
    renderer.initGL();
}

RendererWindow::~RendererWindow() {
    
}

void RendererWindow::pollEvents() {
    // handle events
    if(std::optional<input_event> e = this->input_channel->get()) {
        // std::cout << "e->type: " << e->type << std::endl;
        switch (e->type) {
            case input_event::FONT_CHANGE: {
                renderer.initCharactersGL();
                break;
            }
            case input_event::SLIDE: {
                this->slide = std::get<std::shared_ptr<Slide>>(e->data);
                break;
            }
        }
    }
}

void RendererWindow::renderGL() {
    int err = glGetError();
    if(err != 0) {
        std::cout << "pre errrorGL: " << std::hex << err << std::endl;
    }
     
    renderer.preRender();

    if(slide) {
        (*slide)->render(renderer);
    }

    renderer.postRenderer();

    err = glGetError();
    if(err != 0) {
        std::cout << "post errrorGL: " << std::hex << err << std::endl;
    }
}

void RendererWindow::postInit() {
    
}

void RendererWindow::onSizeChange() {
    this->renderer.setSize(this->width, this->height);

    output_channel->put(output_event{
        output_event::SIZE_CHANGE,
        glm::vec2(this->width, this->height)
    });
}