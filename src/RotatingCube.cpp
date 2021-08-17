#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <chrono>
#include <iostream>
#include "RendererCore.hpp"

class MyRenderer : public stlr::RendererCore {
public:
    MyRenderer( stlr::Window& w ) : stlr::RendererCore( w ) {}
protected:
    void update() {}
    void render() {}
};

float _windowWidth = 1080.0f, _windowHeight = 720.0f;
float _windowAspectRatio = static_cast<float>(_windowWidth) / _windowHeight;
glm::vec3 cameraPosition = glm::vec3(0, 3, -5);
glm::vec3 lookAtPosition = glm::vec3(0, 0, 0);
glm::vec3 upDirection = glm::vec3(0, -1, 0);

int main(int argc, char** argv) {
    stlr::Window w;
    MyRenderer r(w);
    return 0;
}
