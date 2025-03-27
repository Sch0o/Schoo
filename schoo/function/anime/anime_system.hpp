#include <cstdint>

namespace schoo{
    class AnimationSystem{
    public:

        AnimationSystem(const AnimationSystem&)=delete;
        AnimationSystem&operator=(const AnimationSystem&)=delete;

        static void Init();
        static void Quit();
        void updateAnimation(float deltaTime);
        static AnimationSystem&instance(){
            static AnimationSystem instance;
            return instance;
        }
    private:
        uint32_t activeAnimation{0};
        AnimationSystem()=default;
    };
}
