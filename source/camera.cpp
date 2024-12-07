#include"schoo/render/camera.hpp"
#include "external/glm/gtc/matrix_transform.hpp"
namespace schoo{
    Camera::Camera(glm::vec3 position, glm::vec3 target, glm::vec3 worldUp):
    position_(position),worldUp_(worldUp){
        position_=position;
        worldUp_=worldUp;
        forward_=glm::normalize(target-position);
        right_=glm::normalize(glm::cross(forward_,worldUp));
        up_=glm::normalize(glm::cross(right_,forward_));

    }
    Camera::Camera(glm::vec3 position, float pitch, float yaw, glm::vec3 worldup):
    position_(position),pitch_(pitch),yaw_(yaw),worldUp_(worldup) {
        forward_.x = glm::cos(pitch_) * glm::cos(yaw_);
        forward_.y = glm::sin(pitch_);
        forward_.z = glm::cos(pitch_) * glm::sin(yaw_);
        forward_ = glm::normalize(forward_);
        right_ = glm::normalize(glm::cross(forward_, worldUp_));
        up_= glm::normalize(glm::cross(right_, forward_));
    }
    glm::mat4 Camera::GetViewMatrix(){
        return glm::lookAt(position_,position_+forward_,worldUp_);
    }
    void Camera::MoveForward(float deltaTime){
        position_+=deltaTime*moveSpeed*forward_;
    }
    void Camera::MoveRight(float deltaTime){
        position_+=deltaTime*moveSpeed*right_;
    }
    void Camera::MoveUp(float deltaTime) {
        position_+=deltaTime*moveSpeed*up_;
    }
    void Camera::ProcessMouseMovement(float deltaX,float deltaY){
        pitch_+=deltaY*mouseSensitivity_;
        yaw_+=deltaX*mouseSensitivity_;

        if(pitch_>89.0f){
            pitch_=89.0f;
        }
        if(pitch_<-89.0f){
            pitch_=-89.0f;
        }

        updateCameraVectors();
    }
    void Camera::updateCameraVectors() {
        forward_.x = glm::cos(pitch_) * glm::cos(yaw_);
        forward_.y = glm::sin(pitch_);
        forward_.z = glm::cos(pitch_) * glm::sin(yaw_);
        forward_ = glm::normalize(forward_);
        right_ = glm::normalize(glm::cross(forward_, worldUp_));
        up_= glm::normalize(glm::cross(right_, forward_));
    }

    glm::vec3 Camera::GetPosition() {
        return position_;
    }

}