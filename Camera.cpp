#include "Camera.h"

namespace vk {
    Camera::Camera()
    {
        position = glm::vec3(0.0f, 0.5f, 2.0f);
        Orientation = glm::vec3(0.f, -0.5f, -2.0f);
        test_Orientation = glm::quat();
        Up = glm::vec3(0.f, 1.f, 0.f);

        view = glm::mat4(1.0f);
        proj = glm::mat4(1.0f);
    }

    void Camera::update(float FOVdeg, float nearPlane, float farPlane)
    {	// Initializes matrices since otherwise they will be the null matrix
        if (glfwJoystickPresent(GLFW_JOYSTICK_1)) {
            std::jthread t_buttons([this] { Controller_Input(); });
        }
        std::jthread t_mouse([this] { Mouse_Input(); });
        std::jthread t_keyboard([this] { Keyboard_Input(); });
        if (!noClip) {
            gravity(position, velocity);
        }
        //view = glm::lookAt(position, position + Orientation, Up);
        glm::mat4 rotate = glm::mat4_cast(test_Orientation);
        glm::mat4 translate = glm::translate(glm::mat4(1.f), -position);
        view = rotate * translate;

        proj = glm::perspective(glm::radians(FOVdeg), (float)GPU::Extent.width / GPU::Extent.height, nearPlane, farPlane);
        proj[1][1] *= -1;
    }

    void Camera::Keyboard_Input() {
        if (glfwGetKey(Window::handle, GLFW_KEY_W))
        {
            position += velocity * Orientation;
        }

        if (glfwGetKey(Window::handle, GLFW_KEY_A))
        {
            position -= velocity * glm::normalize(glm::cross(Orientation, Up));
        }

        if (glfwGetKey(Window::handle, GLFW_KEY_S))
        {
            position -= velocity * Orientation;
        }

        if (glfwGetKey(Window::handle, GLFW_KEY_D))
        {
            position += velocity * glm::normalize(glm::cross(Orientation, Up));
        }

        if (glfwGetKey(Window::handle, GLFW_KEY_SPACE))
        {
            position += velocity * Up;
        }

        if (glfwGetKey(Window::handle, GLFW_KEY_LEFT_CONTROL))
        {
            position -= velocity * Up;
        }

        if (glfwGetKey(Window::handle, GLFW_KEY_LEFT_SHIFT))
        {
            velocity = 0.01f;
        }

        else if (!glfwGetKey(Window::handle, GLFW_KEY_LEFT_SHIFT))
        {
            velocity = 0.005f;
        }
        if (glfwGetKey(Window::handle, GLFW_KEY_V))
        {
            noClip = !noClip;
        }
    }

    void Camera::Controller_Input() {
        glm::mat4 inverted = glm::inverse(glm::toMat4(test_Orientation));
        // Button Mapping
        int buttonCount;
        const unsigned char* buttons = glfwGetJoystickButtons(GLFW_JOYSTICK_1, &buttonCount);
        if (buttons[1])
        {// X button
            glm::vec3 up = normalize(glm::vec3(inverted[1]));
            position += 2 * velocity * up;
        }
        if (buttons[2])
        {// O button
            glm::vec3 down = normalize(glm::vec3(inverted[1]));
            position -= 2 * velocity * down;
        }
        if (buttons[10])
        {// left stick in
            velocity *= 10.f;
        }
        if (buttons[11])
        {// right stick in
            velocity *= 10.f;
        }
        if (buttons[14])
        {// dpad up
            velocity *= 100.0f;
        }
        if (buttons[15])// dpad down
        {
            velocity /= 2.f;
        }
        if (buttons[5])// right bumper
        {
            float rotZ = sensitivity;
            glm::mat4 roll_mat = glm::rotate(glm::mat4(1.0f), glm::radians(rotZ), Orientation);
            Up = glm::mat3(roll_mat) * Up;
        }
        if (buttons[4])
        {
            float rotZ = -sensitivity;
            glm::mat4 roll_mat = glm::rotate(glm::mat4(1.0f), glm::radians(rotZ), Orientation);
            Up = glm::mat3(roll_mat) * Up;
        }

        // Controller Joystick Settings
        int axesCount;
        const float* axes = glfwGetJoystickAxes(GLFW_JOYSTICK_1, &axesCount);
        if ((axes[2] > RS_deadzone_X) || (axes[2] < -RS_deadzone_X))
        {
            rotX += sensitivity * axes[2];
            qPitch = glm::angleAxis(rotX, glm::vec3(0, 1, 0));
        }
        if ((axes[5] > RS_deadzone_Y) || (axes[5] < -RS_deadzone_Y))
        {
            rotY += sensitivity * axes[5];
            if (rotY >= 0.875f) {
                rotY = 0.875f;
            }
            else if (rotY <= -0.75f) {
                rotY = -0.75f;
            }
            qYaw = glm::angleAxis(rotY, glm::vec3(1, 0, 0));
        }
        test_Orientation = glm::normalize(qYaw * qPitch);

        // Left and Right
        if ((axes[0] > 0.1) || (axes[0] < -0.1))
        {
            glm::vec3 sideways = normalize(glm::vec3(inverted[0]));
            position += sideways * (velocity * axes[0]);
        }
        //Forward and Backwards
        if ((axes[1] > 0.1) || (axes[1] < -0.1))
        {
            glm::vec3 forward = normalize(glm::vec3(inverted[2]));
            position += forward * (velocity * axes[1]);
        }
    }

    float Camera::rotation(const float input) {
        return glm::radians(sensitivity * input);
    }

    void Camera::Mouse_Input()
    {
        // Handles mouse inputs
        if (glfwGetMouseButton(Window::handle, GLFW_MOUSE_BUTTON_RIGHT))
        {	// Hides mouse cursor
            glfwSetInputMode(Window::handle, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

            if (firstClick)
            {	// Prevents camera from jumping on the first click
                glfwSetCursorPos(Window::handle, (SwapChain::Extent.width / 2.0), (SwapChain::Extent.height / 2.0));
                firstClick = false;
            }
            // Stores the coordinates of the cursor
            double mouseX;
            double mouseY;

            // Fetches the coordinates of the cursor
            glfwGetCursorPos(Window::handle, &mouseX, &mouseY);

            // Normalizes and shifts the coordinates of the cursor such that they begin in the middle of the screen
            // and then "transforms" them into degrees 
            float rotX = sensitivity * (float)(mouseY - (SwapChain::Extent.height / 2.0)) / SwapChain::Extent.height;
            float rotY = sensitivity * (float)(mouseX - (SwapChain::Extent.width / 2.0)) / SwapChain::Extent.width;

            // Calculates upcoming vertical change in the Orientation
            glm::vec3 newOrientation = glm::rotate(Orientation, glm::radians(-rotX), glm::normalize(glm::cross(Orientation, Up)));

            if (abs(glm::angle(newOrientation, Up) - glm::radians(90.0f)) <= glm::radians(85.0f))
            {	// Decides whether or not the next vertical Orientation is legal or not
                //Orientation = newOrientation;
            }

            // Rotates the Orientation left and right
            //Orientation = glm::rotate(Orientation, glm::radians(-rotY), Up);

            // Sets mouse cursor to the middle of the screen so that it doesn't end up roaming around
            glfwSetCursorPos(Window::handle, (SwapChain::Extent.width / 2.0), (SwapChain::Extent.height / 2.0));
        }
        else if (!glfwGetMouseButton(Window::handle, GLFW_MOUSE_BUTTON_RIGHT))
        {	// Unhides cursor since camera is not looking around anymore
            glfwSetInputMode(Window::handle, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            // Makes sure the next time the camera looks around it doesn't jump
            firstClick = true;
        }
    }

}

