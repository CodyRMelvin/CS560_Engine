//**************************************************************************************************
// File   : Animation.h
// Author : Cody Melvin
// Date   : 2026/10/17
// Brief  : Interface for class containing and updating animation data
//**************************************************************************************************

#pragma once
#ifndef ANIMATION_H
#define ANIMATION_H

#include "Common.h"

class Animation
{
    public:
        static std::vector<Animation> AnimationLibrary;

        // Enum determining what the animation channel modifies
        // Weight path in not implemented, but can be read in
        enum Path : int
        {
            pathInvalid = -1,
            pathTranslation,
            pathRotation,
            pathScale,
            pathWeights,
        };

        // Enum describing the form of interpolation being used
        // Only inrepolationLinear is implemented, despite other options being deserializable
        enum Interpolation
        {
            interpolationInvalid = -1,
            interpolationLinear,
            interpolationStep,
            interpolationCubicSpline,
        };

        // Struct detailing keframe data
        struct Frame
        {
            float time;
            // Variant for the various data that can be modified by a channel
            // Can be accessed via std::get passing in the Path enum value as the template
            std::variant< glm::vec3, Quat, glm::vec3, glm::vec2 > output;
        };

        // Struct containing the needed data for an animation channel
        struct Channel
        {
            // Node being modified
            uint64_t target;
            // Path of channel
            Path path;
            // Interpolation method of channel
            Interpolation interpolation;
            // Keyframes of channel
            std::vector<Frame> frames;
            // The total time that has passed in the channel, loops back to 0
            float elapsedTime;
        };

        // Standard cosntrutors and destructors
        Animation() = default;
        Animation( uint64_t id, uint64_t parent );
        Animation( const Animation& other );
        ~Animation() = default;

        // Performs needed initialization on the animation
        void Initialize();
        // Performs single update to effected nodes based on dt
        void Animate();

        // Getters and setters for name of animation
        std::string_view Name() const { return m_name; }
        void Name( std::string name ) { m_name = name; }
        void Name( const char* name ) { m_name = name; }

        // Getter and adder for channels
        Channel GetChannel( size_t index ) const;
        void AddChannel( const Channel& channel );

    private:
        // Channels managed by this animation
        std::vector<Channel> m_channels;
        // Name of animation
        std::string m_name;
};

#endif
