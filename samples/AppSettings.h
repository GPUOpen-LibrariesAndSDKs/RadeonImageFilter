#pragma once
#include <string>
#include "Utils/cmd_parser.h"

using namespace utils;

struct AppSettings
{
    AppSettings();

    //model load settings
    std::string path = "./images/";
    std::string colorImg = "cam_12_color_spp_8.bin" ;
    std::string albedoImg = "cam_12_albedo_spp_8.bin";
    std::string depthImg = "cam_12_view_shading_depth.bin";
    std::string normalImg = "cam_12_view_shading_normal.bin";

    bool UseColorOnly = false;
    bool UseToneMapper = false;
};


class AppCliParser
{
public:

    AppCliParser(int argc, char * argv[]);
    AppSettings Parse();

private:

    void ShowHelp();

    CmdParser m_cmd_parser;
};