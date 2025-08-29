#pragma once
#include <string>

// rows, cols = grid layout (use 1,1 for single photo)
// tileAspect = width/height (3.0/4.0 gives a 3:4 look)
// addWhiteBorder = draw a small white border around the tile
bool make_frame_opencv(const std::string& inputJpg,
                       const std::string& outputPng,
                       const std::string& logoPng,
                       int rows = 1, int cols = 1,
                       double tileAspect = 3.0/4.0,
                       bool addWhiteBorder = true);
