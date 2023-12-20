#include "slide.h"
#include "csv.hpp"

TextSlide::TextSlide(std::string text): text(text) {
}

void TextSlide::render(Renderer renderer) {
    // int width = renderer.calculateWidth(text);
    // int height = renderer.calculateHeight(text);
    // int centerX = renderer.getWidth() / 2;
    // int centerY = renderer.getHeight() / 2;
    // renderer.renderText(text, centerX - (width / 2), centerY - (height / 2), 1.0f, glm::vec3(1.0f, 1.0f, 1.0f));
    renderer.renderTextWrappedCentered(text, 1.0f, 20.0f, glm::vec3(1.0f, 1.0f, 1.0f));
}

std::string TextSlide::getText() {
    return this->text;
}

std::vector<std::shared_ptr<Slide>> readSlidesCSV(std::string file) {
    csv::CSVReader reader(file);
    std::vector<std::shared_ptr<Slide>> slides;
    for (csv::CSVRow& row: reader) {
        auto t = row["text"].get<std::string>();
        std::shared_ptr<TextSlide> s = std::make_shared<TextSlide>(t);
        slides.push_back(s);
    }
    return slides;
}