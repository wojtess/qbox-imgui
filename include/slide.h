#pragma once
#include "renderer.h"
#include <string>
#include <vector>

class Slide {
public:
    virtual void render(Renderer) = 0;
    virtual std::string getText() = 0;
};

class TextSlide: public Slide {
public:
    TextSlide(std::string);
    void render(Renderer);
    std::string getText();
private:
    std::string text;
};

std::vector<std::shared_ptr<Slide>> readSlidesCSV(std::string);