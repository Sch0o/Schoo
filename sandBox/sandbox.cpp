
#include"schoo/window.hpp"
#include"schoo/schoo.hpp"



int main(){
    schoo::Init();
    schoo::Window &window=schoo::Window::GetInstance();

    while(!window.ShouldClose()){
        window.PollEvents();
        window.SwapBuffer();
        schoo::Render();
    }
    schoo::Quit();
}