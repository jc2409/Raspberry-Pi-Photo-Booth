#include "save_picture.h"
#include "make_frame_opencv.h"
#include <filesystem>
#include <ctime>
#include <cstdlib>
#include <iostream>
#include <string>
#include <unistd.h>

namespace fs = std::filesystem;

static std::string sanitize(const std::string& s) {
    std::string out = s;
    for (char& c : out) {
        if (c == ' ') c = '_';
        else if (!(std::isalnum(static_cast<unsigned char>(c)) || c=='-' || c=='_')) c = '_';
    }
    return out;
}

std::string username() {
    std::string name;
    std::cout << "Please enter your name: ";
    std::getline(std::cin >> std::ws, name);
    return sanitize(name);
}

void save_pic() {
    // ensure output dirs (so rpicam/opencv writes don’t fail)
    fs::create_directories("./out");
    fs::create_directories("./out/framed");

    std::cout << "************ Welcome to the Arm Photo Booth! ************" << std::endl;

    char status;
    std::string name = username();

    std::cout << "Please enter 'y' to take a picture: ";
    std::cin >> status;

    if (status == 'y' || status == 'Y') {
        std::cout << "Taking picture..." << std::endl;
        for (int i = 0; i < 3; ++i) {
            std::cout << "Picture will be taken in " << (3 - i) << " seconds." << std::endl;
            sleep(1);
        }

        std::time_t ts = std::time(nullptr);
        std::string filename = "./out/" + name + "_" + std::to_string(ts) + ".jpg";
        std::string framed   = "./out/framed/" + name + "_" + std::to_string(ts) + "_framed.png";

        int rc = std::system(("rpicam-still -n -o \"" + filename + "\" > /dev/null 2>&1").c_str());
        if (rc != 0) {
            std::cout << "Capture failed (code " << rc << ").\n";
            return;
        }
        std::cout << "Picture taken and saved as " << filename << std::endl;

        bool ok = make_frame_opencv(
            filename, framed, "./logo/arm_logo.png",
            /*rows=*/1, /*cols=*/1, /*tileAspect=*/3.0/4.0, /*addWhiteBorder=*/true
        );
        std::cout << (ok ? "Framed photo created: " + framed : "Failed to create framed photo.") << std::endl;

        std::string email;
        std::cout << "Please enter your email address (Press q to quit): ";
        std::getline(std::cin >> std::ws, email);

        if (email == "q") {
            std::cout << "Exiting PhotoBooth.\n";
            return;
        }

        std::string cmd =
            "python3 ./scripts/send_email.py "
            "--to \"" + email + "\" "
            "--name \"" + name + "\" "
            "--file \"" + framed + "\"";
        int mrc = std::system(cmd.c_str());
        std::cout << (mrc == 0 ? "Email sent to " + email : "Email failed (code " + std::to_string(mrc) + ").") << std::endl;
    } else {
        std::cout << "Exiting PhotoBooth.\n";
    }
}
