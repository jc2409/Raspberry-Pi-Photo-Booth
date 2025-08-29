#include "make_frame_opencv.h"
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <filesystem>
#include <iostream>
#include <cmath>

namespace fs = std::filesystem;

static cv::Mat resizeCrop(const cv::Mat& src, int w, int h) {
    if (src.empty()) return {};
    const double sx = w / static_cast<double>(src.cols);
    const double sy = h / static_cast<double>(src.rows);
    const double s = std::max(sx, sy);
    cv::Mat r; cv::resize(src, r, cv::Size(), s, s, cv::INTER_AREA);
    int x = std::max(0, (r.cols - w) / 2);
    int y = std::max(0, (r.rows - h) / 2);
    return r(cv::Rect(x, y, w, h)).clone();
}

static void alphaBlend(const cv::Mat& fgRGBA, cv::Mat& bgBGR, cv::Point pos) {
    CV_Assert(fgRGBA.type() == CV_8UC4 && bgBGR.type() == CV_8UC3);
    for (int y = 0; y < fgRGBA.rows; ++y) {
        int by = pos.y + y; if (by < 0 || by >= bgBGR.rows) continue;
        const cv::Vec4b* fr = fgRGBA.ptr<cv::Vec4b>(y);
        cv::Vec3b* br = bgBGR.ptr<cv::Vec3b>(by);
        for (int x = 0; x < fgRGBA.cols; ++x) {
            int bx = pos.x + x; if (bx < 0 || bx >= bgBGR.cols) continue;
            const cv::Vec4b& p = fr[x];
            double a = p[3] / 255.0;
            br[bx][0] = cv::saturate_cast<uchar>((1-a)*br[bx][0] + a*p[0]);
            br[bx][1] = cv::saturate_cast<uchar>((1-a)*br[bx][1] + a*p[1]);
            br[bx][2] = cv::saturate_cast<uchar>((1-a)*br[bx][2] + a*p[2]);
        }
    }
}

bool make_frame_opencv(const std::string& inputJpg,
                       const std::string& outputPng,
                       const std::string& logoPng,
                       int rows, int cols,
                       double tileAspect,
                       bool addWhiteBorder)
{
    try {
        fs::create_directories(fs::path(outputPng).parent_path());

        // Canvas & brand bars
        const int W = 1200, H = 1800;
        const int barH = 160;
        const int margin = 40;
        const int gap = 40;
        const cv::Scalar brandBGR(0x27, 0x16, 0x00); // #001627 in B,G,R

        cv::Mat canvas(H, W, CV_8UC3, cv::Scalar(255,255,255));
        cv::rectangle(canvas, {0,0,W,barH}, brandBGR, cv::FILLED);
        cv::rectangle(canvas, {0,H-barH,W,barH}, brandBGR, cv::FILLED);

        // Load source
        cv::Mat src = cv::imread(inputJpg, cv::IMREAD_COLOR);
        if (src.empty()) {
            std::cerr << "make_frame_opencv: failed to read input: " << inputJpg << "\n";
            return false;
        }

        // Safe area (between bars, with margins)
        const int safeX = margin;
        const int safeY = barH + margin;
        const int safeW = W - 2*margin;
        const int safeH = H - 2*barH - 2*margin;

        // Per-tile max (from width and height budgets)
        const double maxTileW_fromW = (safeW - (cols-1)*gap) / static_cast<double>(cols);
        const double maxTileH_fromH = (safeH - (rows-1)*gap) / static_cast<double>(rows);

        // Keep tileAspect = width/height; fit by width first, else by height
        int tileW = static_cast<int>(std::floor(maxTileW_fromW));
        int tileH = static_cast<int>(std::floor(tileW / tileAspect));
        if (tileH > maxTileH_fromH) {
            tileH = static_cast<int>(std::floor(maxTileH_fromH));
            tileW = static_cast<int>(std::floor(tileH * tileAspect));
        }

        // Center the grid
        const int gridW = cols*tileW + (cols-1)*gap;
        const int gridH = rows*tileH + (rows-1)*gap;
        const int startX = safeX + (safeW - gridW)/2;
        const int startY = safeY + (safeH - gridH)/2;

        // Prepare one normalized tile
        cv::Mat tile = resizeCrop(src, tileW, tileH);

        // Optional white border
        int border = addWhiteBorder ? 14 : 0;
        cv::Mat tileWithBorder(tileH + 2*border, tileW + 2*border, CV_8UC3, cv::Scalar(255,255,255));
        tile.copyTo(tileWithBorder(cv::Rect(border, border, tileW, tileH)));

        // Paste tiles
        for (int r = 0; r < rows; ++r) {
            for (int c = 0; c < cols; ++c) {
                int x = startX + c*(tileW + gap) - border;
                int y = startY + r*(tileH + gap) - border;
                tileWithBorder.copyTo(canvas(cv::Rect(x, y, tileWithBorder.cols, tileWithBorder.rows)));
            }
        }

        // Logo on bottom bar (center, ~110px high)
        cv::Mat logo = cv::imread(logoPng, cv::IMREAD_UNCHANGED);
        if (!logo.empty()) {
            double scale = 110.0 / logo.rows;
            cv::Mat lr; cv::resize(logo, lr, cv::Size(), scale, scale, cv::INTER_AREA);
            int lx = (W - lr.cols)/2;
            int ly = H - barH + (barH - lr.rows)/2;
            if (lr.channels() == 4) alphaBlend(lr, canvas, {lx, ly});
            else lr.copyTo(canvas(cv::Rect(lx, ly, lr.cols, lr.rows)));
        } else {
            cv::putText(canvas, "arm", {W/2-120, H-55}, cv::FONT_HERSHEY_SIMPLEX, 2.5,
                        cv::Scalar(255,255,255), 5, cv::LINE_AA);
        }

        if (!cv::imwrite(outputPng, canvas)) {
            std::cerr << "make_frame_opencv: failed to write: " << outputPng << "\n";
            return false;
        }
        return true;
    } catch (const std::exception& e) {
        std::cerr << "make_frame_opencv: exception: " << e.what() << "\n";
        return false;
    }
}
