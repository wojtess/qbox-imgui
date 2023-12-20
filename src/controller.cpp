#include "controller.h"
#include <filesystem>
#include "utils.h"

Controller::Controller(
        std::shared_ptr<Queue<MainWindow::output_event>> output_channel_main,
        std::shared_ptr<Queue<MainWindow::input_event>> input_channel_main,
        std::shared_ptr<Queue<RendererWindow::input_event>> input_channel_renderer,
        std::shared_ptr<Queue<RendererWindow::output_event>> output_channel_renderer,
        std::shared_ptr<Renderer::renderer_data> renderer_data
    ): output_channel_main(output_channel_main), input_channel_main(input_channel_main), input_channel_renderer(input_channel_renderer), output_channel_renderer(output_channel_renderer), renderer_data(renderer_data) {

    if(FT_Init_FreeType(&ft)) {
        std::cout << "error while initializing freetype" << std::endl;
    }
}

void Controller::stop() {
    this->working = false;
}

void Controller::startLoopInCurrenThread() {
    while(working) {
        if(std::optional<MainWindow::output_event> event_output_main = output_channel_main->get()) {
            switch(event_output_main->type) {
                case MainWindow::output_event::SET_FONT: {
                    MainWindow::output_event::set_font_data font_data = std::get<MainWindow::output_event::set_font_data>(event_output_main->data);
                    std::string path = std::string("fonts/").append(font_data.name);

                    FT_Face* face = new FT_Face;
                    int result = FT_New_Face(ft, path.c_str(), 0, face);
                    if(result != 0) {
                        std::cout << "error while loading face" << std::endl;
                        continue;
                    }
                    FT_Set_Pixel_Sizes(*face, 0, font_data.size);

                    std::shared_ptr<FT_Face> font(face, [] (FT_Face* face) {
                        FT_Done_Face(*face);
                        delete face;
                    });
                    
                    renderer_data->face = font;

                    input_channel_renderer->put(RendererWindow::input_event{
                        RendererWindow::input_event::FONT_CHANGE
                    });
                    input_channel_main->put(MainWindow::input_event{
                        MainWindow::input_event::SET_FONT_RENDERER
                    });

                    renderer_data->font_size = font_data.size;

                    break;
                }
                case MainWindow::output_event::GET_FONTS: {
                    std::vector<std::string> fonts;
                    try {
                        for(const auto& file:std::filesystem::directory_iterator("fonts")) {
                            if(file.is_regular_file()) {
                                fonts.push_back(file.path().filename().string());
                            }
                        }
                    } catch(std::filesystem::filesystem_error const& ex) {
                        //todo add event for errors
                    }
                    this->input_channel_main->put(MainWindow::input_event{
                        MainWindow::input_event::FONTS_RESPONSE,
                        fonts
                    });
                    break;
                }
                case MainWindow::output_event::LOAD_SLIDES: {
                    auto file_path = std::get<std::string>(event_output_main->data);
                    this->slides = readSlidesCSV(file_path);
                    this->currentSlide = 0;

                    if(slides.size() >= currentSlide) {
                        input_channel_main->put(MainWindow::input_event{
                            MainWindow::input_event::SET_SLIDES,
                            this->slides
                        });
                        input_channel_main->put(MainWindow::input_event{
                            MainWindow::input_event::SLIDE,
                            0
                        });

                        input_channel_renderer->put(RendererWindow::input_event{
                            RendererWindow::input_event::SLIDE,
                            this->slides[currentSlide]
                        });
                    }
                    break;
                }
                case MainWindow::output_event::CHANGE_SLIDE: {
                    auto s = std::get<int>(event_output_main->data);
                    int newSlide = this->currentSlide + s;
                    if(newSlide >= 0 && newSlide <= slides.size() - 1) {
                        this->currentSlide = newSlide;
                    }
                    if(slides.size() >= currentSlide) {
                        input_channel_main->put(MainWindow::input_event{
                            MainWindow::input_event::SLIDE,
                            this->currentSlide
                        });

                        input_channel_renderer->put(RendererWindow::input_event{
                            RendererWindow::input_event::SLIDE,
                            this->slides[currentSlide]
                        });
                    }
                    break;
                }
                case MainWindow::output_event::SET_SPACING: {
                    auto d = std::get<glm::vec2>(event_output_main->data);
                    renderer_data->spacing_x = d.x;
                    renderer_data->spacing_y = d.y;
                    break;
                }
            }
        }
        if(auto event_output_renderer = output_channel_renderer->get()) {
            switch(event_output_renderer->type) {
                case RendererWindow::output_event::SIZE_CHANGE: {
                    auto size = std::get<glm::vec2>(event_output_renderer->data);
                    renderer_data->height = size.y;
                    renderer_data->width = size.x;
                    input_channel_main->put(MainWindow::input_event {
                        MainWindow::input_event::RENDERER_SIZE_CHANGE,
                        size
                    });
                    break;
                }
            }
        }
    }
}