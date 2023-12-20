#pragma once
#include "gl.h"
#include <memory>
#include <string>
#include "shader.h"
#include <vector>

extern "C" {
#include <ft2build.h>
#include FT_FREETYPE_H
}

GLFWmonitor* getMonitor(int);

std::vector<std::string> split(const std::string& s, char seperator);