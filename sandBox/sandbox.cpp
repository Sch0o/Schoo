#include"schoo/engine.hpp"

int main() {
    schoo::SchooEngine::Init();
    auto &app = schoo::SchooEngine::GetInstance();
    app.TickMain();
    schoo::SchooEngine::Quit();
    return 1;
}