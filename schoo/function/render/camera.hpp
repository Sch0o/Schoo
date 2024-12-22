#include"external/glm/glm.hpp"
namespace schoo{
    class Camera{
    public:
        Camera(glm::vec3 position,glm::vec3 target,glm::vec3 worldUp);
        Camera(glm::vec3 position, float pitch, float yaw, glm::vec3 worldup);

        glm::mat4 GetViewMatrix();
        void ProcessMouseMovement(float deltaX,float deltaY);
        void MoveForward(float delatTime);
        void MoveRight(float delatTime);
        void MoveUp(float deltaTime);
        glm::vec3 GetPosition();
    private:
        glm::vec3 position_;
        glm::vec3 worldUp_;
        glm::vec3 forward_;
        glm::vec3 up_;
        glm::vec3 right_;

        float fov=90;

        float mouseSensitivity_=0.005f;
        float pitch_;
        float yaw_;
        float moveSpeed=10.0f;

        void updateCameraVectors();



    };
}