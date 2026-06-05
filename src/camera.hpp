// camera.hpp
#pragma once
#include <Common/interface/BasicMath.hpp>
#include <cmath>
#include <SDL3/SDL.h>

class Camera {
public:
    Camera(const float3& pos = {0,0.1,-3.0}, float yaw = 0.0f, float pitch = 0.0f) : m_position(pos), m_yaw(yaw), m_pitch(pitch) {}

    void processKeys(const bool* keystate, float deltatime) {
        float speed = m_speed * deltatime;

        if(keystate[SDL_SCANCODE_LCTRL]) speed *= 2.0f;

        float3 forward = getForward();
        float3 right = normalize(cross(forward,float3(0,1,0)));

        if (keystate[SDL_SCANCODE_W]) m_position += forward * speed;
        if (keystate[SDL_SCANCODE_S]) m_position -= forward * speed;
        if (keystate[SDL_SCANCODE_A]) m_position -= right   * speed;
        if (keystate[SDL_SCANCODE_D]) m_position += right   * speed;
        if (keystate[SDL_SCANCODE_Q] || keystate[SDL_SCANCODE_SPACE]) m_position.y += speed;
        if (keystate[SDL_SCANCODE_E] || keystate[SDL_SCANCODE_LSHIFT]) m_position.y -= speed;

        if (keystate[SDL_SCANCODE_MINUS]) m_speed *= 0.75f;
        if (keystate[SDL_SCANCODE_EQUALS]) m_speed *= 1.25f;
    }

    void processMouse(float xrel, float yrel) {
        const float sensitivity = 0.1f;
        m_yaw -= xrel * sensitivity;
        m_pitch += yrel * sensitivity;

        if(m_pitch > 89.0f) m_pitch = 89.0f;
        if(m_pitch < -89.0f) m_pitch = -89.0f;
    }

    float4x4 getViewMat() const {
        float3 forward = getForward(true);
        float3 right = normalize(cross(forward,float3(0,1,0)));
        float3 up = cross(right, forward);

        float4x4 mat = {
            right.x, right.y, right.z, 0.0f,
            up.x, up.y, up.z, 0.0f,
            forward.x, forward.y, forward.z, 0.0f,
            m_position.x, m_position.y, m_position.z, 1.0f
        };
        return mat;
    }

    float3 pos() {return m_position;}

    void reset() {
        m_position = {0.0,0.1,-3.0};
        m_yaw = 0.0f;
        m_pitch = 0.0f;
    }

private:
    float3 getForward(bool straight = false) const {
        float yawRad = m_yaw * (M_PI / 180.0f);
        float pitchRad = m_pitch * (M_PI / 180.0f);
        if(!straight) {
            return float3(
                /*cos(pitchRad) * */sin(yawRad),
                /*-sin(pitchRad)*/0.0f,
                /*cos(pitchRad) **/ cos(yawRad)
            );
        } else {
            return float3(
                cos(pitchRad) * sin(yawRad),
                -sin(pitchRad),
                cos(pitchRad) * cos(yawRad)
            );
        }
    }

    float3 m_position;
    float m_yaw;
    float m_pitch;
    float m_speed = 5.0f;
};
