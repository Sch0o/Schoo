#include"schoo/schoo.hpp"

int main() {
    schoo::Schoo::Init();
    auto &app = schoo::Schoo::GetInstance();
    app.TickMain();
    schoo::Schoo::Quit();

    return 1;
}