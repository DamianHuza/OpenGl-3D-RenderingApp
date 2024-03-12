#include "Camera.hpp"

namespace gps {

    //Camera constructor
    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp) {
        //TODO
        this->cameraPosition = cameraPosition;
        this->cameraTarget = cameraTarget;
        this->cameraUpDirection = cameraUp;

        this->cameraFrontDirection = glm::normalize(cameraTarget - cameraPosition);

    }

    //return the view matrix, using the glm::lookAt() function
    glm::mat4 Camera::getViewMatrix() {
        //TODO
        return  glm::lookAt(this->cameraPosition, this->cameraPosition + this->cameraFrontDirection, this->cameraUpDirection);

    }

    glm::vec3 Camera::getCameraPosition() {
        //TODO
        return this->cameraPosition;

    }

    //update the camera internal parameters following a camera move event
    void Camera::move(MOVE_DIRECTION direction, float speed) {
        //TODO
        if (direction == MOVE_FORWARD)
            this->cameraPosition += speed * this->cameraFrontDirection;
        if (direction == MOVE_BACKWARD)
            this->cameraPosition -= speed * this->cameraFrontDirection;
        if (direction == MOVE_LEFT)
            this->cameraPosition -= glm::normalize(glm::cross(cameraFrontDirection, cameraUpDirection)) * speed;
        if (direction == MOVE_RIGHT)
            this->cameraPosition += glm::normalize(glm::cross(cameraFrontDirection, cameraUpDirection)) * speed;
        if (direction == MOVE_UP)
            this->cameraPosition += -1 * speed * this->cameraUpDirection;
        if (direction == MOVE_DOWN)
            this->cameraPosition += speed * this->cameraUpDirection;
    }

    //update the camera internal parameters following a camera rotate event
    //yaw - camera rotation around the y axis
    //pitch - camera rotation around the x axis
    void Camera::rotate(float pitch, float yaw) {
        //TODO
        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;

        glm::vec3 front;
        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        this->cameraFrontDirection = glm::normalize(front);
    }
    void Camera::animation(float x , float angle) {
        this->cameraPosition = glm::vec3(39.0f , 25.0f,110.0f);
        this->cameraTarget = glm::vec3(39.0f, 25.0f, -50.0f);

        glm::mat4 camera = glm::mat4(1.0f);

        camera = glm::rotate(camera, glm::radians(angle), glm::vec3(0, 1, 0));

        this->cameraPosition = glm::vec4(camera * glm::vec4(this->cameraPosition, 1));
        this->cameraFrontDirection = glm::normalize(cameraTarget - cameraPosition);
        cameraRightDirection = glm::normalize(glm::cross(cameraFrontDirection, glm::vec3(0.0f, 1.0f, 0.0f)));
        
        camera = glm::translate(camera, glm::vec3(10.0f+x, 5.0f + x, 6.67f + x));

        this->cameraPosition = glm::vec4(camera * glm::vec4(this->cameraPosition, 1));
        this->cameraFrontDirection = glm::normalize(cameraTarget - cameraPosition);
        cameraRightDirection = glm::normalize(glm::cross(cameraFrontDirection, glm::vec3(0.0f, 1.0f, 0.0f)));

        

    }
}