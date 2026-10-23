//**************************************************************************************************
// File   : Animation.h
// Author : Cody Melvin
// Date   : 2026/10/17
// Brief  : 
//**************************************************************************************************

#include "Animation.h"

#include "Common.h"
#include "Application.h"

std::vector<Animation> Animation::AnimationLibrary;

Animation::Animation( uint64_t id, uint64_t parent )
    : m_channels()
{ }

Animation::Animation( const Animation& other )
    : m_channels( other.m_channels.begin(), other.m_channels.end() )
{ }

void Animation::Initialize()
{ 
    // Sort the frames in each channel
    std::for_each( m_channels.begin(), m_channels.end(), 
    []( Channel& channel )
    {
        std::sort( channel.frames.begin(), channel.frames.end(),
        []( Frame& frame1, Frame& frame2 )
        {
            return frame1.time < frame2.time;
        });
    });
}

void Animation::Animate()
{
    //return;
    float dt = Application::Get().GetDeltaTime();
    dt = .01;

    std::for_each( m_channels.begin(), m_channels.end(), 
    [dt]( Channel& channel )
    {
        // Loop elapsed time
        channel.elapsedTime = std::fmod( channel.elapsedTime, channel.frames.back().time );
        // Update with dt
        channel.elapsedTime += dt;

        // Get frame with greatest time vlaue under elapsed time 
        auto frameIt = std::lower_bound( channel.frames.begin(), channel.frames.end(), channel.elapsedTime,
        []( Frame& frame, float dt )
        {
            return frame.time < dt;
        });

        if ( frameIt == channel.frames.end() )
            return;

        // Get node target of channel
        Node3D* node = static_cast<Node3D*>( Node::NodeLibrary[channel.target] );

        switch (channel.path)
        {
            case pathTranslation:
                node->LocalTransform().Translation() = std::get<Path::pathTranslation>(frameIt->output);
                break;

            case pathRotation:
                node->LocalTransform().Rotation() = std::get<Path::pathRotation>(frameIt->output);
                break;

            case pathScale:
                node->LocalTransform().Scale() = std::get<Path::pathScale>(frameIt->output);
                break;

            default:
                break;
        }
    });
}

Animation::Channel Animation::GetChannel( size_t index ) const
{
    if ( index < m_channels.size() )
        return m_channels[index];

    Channel c = {0};
    return c;
}

void Animation::AddChannel( const Channel& channel )
{
    m_channels.push_back(channel);
}

