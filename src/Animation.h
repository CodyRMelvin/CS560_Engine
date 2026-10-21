//**************************************************************************************************
// File   : Animation.h
// Author : Cody Melvin
// Date   : 2026/10/17
// Brief  : 
//**************************************************************************************************

#pragma once
#ifndef ANIMATION_H
#define ANIMATION_H

#include "Common.h"

class Animation
{
    public:
        static std::vector<Animation> AnimationLibrary;

        enum Path : int
        {
            pathInvalid = -1,
            pathTranslation,
            pathRotation,
            pathScale,
            pathWeights,
        };

        enum Interpolation
        {
            interpolationInvalid = -1,
            interpolationLinear,
            interpolationStep,
            interpolationCubicSpline,
        };

        struct Frame
        {
            float time;
            std::variant< glm::vec3, Quat, glm::vec3, glm::vec2 > output;
        };

        struct Channel
        {
            uint64_t target;
            Path path;
            Interpolation interpolation;
            std::vector<Frame> frames;
            float elapsedTime;
        };

        Animation() = default;
        Animation( uint64_t id, uint64_t parent );
        Animation( const Animation& other );
        ~Animation() = default;

        void Initialize();
        void Animate();

        Channel GetChannel( size_t index ) const;
        void AddChannel( const Channel& channel );

    private:
        std::vector<Channel> m_channels;
};

#endif
