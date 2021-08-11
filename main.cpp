#include <QGuiApplication>
#include <QtX11Extras/QX11Info>
#include <QWindow>
#include <QObject>
#include <iostream>
#define VK_USE_PLATFORM_XLIB_KHR
#include <QWindow>
#include <spirv_cross.hpp>
#include <fstream>
#include <functional>
#include "renderer.hpp"

using namespace std;

std::vector<uint32_t> gsd(std::string spvFilePath) {
    std::ifstream spvFile(spvFilePath.data(), std::ios::ate | std::ios::binary);
    if(!spvFile.is_open())
        return {};
    size_t size{ static_cast<size_t>(spvFile.tellg()) };
    std::vector<uint32_t> data(size/sizeof(uint32_t));
    spvFile.seekg(0);
    spvFile.read(reinterpret_cast<char*>(data.data()), size);
    spvFile.close();
    return data;
}

int main(int argc, char** argv)
{
//    auto assimpVS = gsd("/home/shinobu/Downloads/Compressed/shaders/AssimpVS.spv");
//    spirv_cross::Compiler comp(assimpVS);
//    auto s = comp.get_shader_resources();
    QGuiApplication app(argc, argv);


    QWindow win;
//    win.show();
//    auto rend = stellar::renderer(win);
    return app.exec();
}
