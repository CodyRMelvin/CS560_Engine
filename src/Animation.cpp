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
    , m_name() 
{ }

Animation::Animation( const Animation& other )
    : m_channels( other.m_channels.begin(), other.m_channels.end() )
    , m_name(other.m_name)
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
    //dt *= 10;
    //dt = .01;

    std::for_each( m_channels.begin(), m_channels.end(), 
    [dt]( Channel& channel )
    {
        if ( channel.frames.empty() )
            return;

        float start = channel.frames.front().time;
        float end = channel.frames.back().time;
        float duration = end - start;

        // Update with dt
        channel.elapsedTime += dt;
        // Loop elapsed time, taking into account the starting and ending times
        if ( duration > 0 )
            channel.elapsedTime = start + std::fmod( channel.elapsedTime - start, duration );
        else
            channel.elapsedTime = start;

        // Get frame with greatest time vlaue under elapsed time 
        auto frameIt = std::upper_bound( channel.frames.begin(), channel.frames.end(), channel.elapsedTime,
        []( float dt, Frame& frame )
        {
            return dt < frame.time;
        });

        Frame nextFrame = *frameIt;
        Frame prevFrame = nextFrame;;

        if ( frameIt != channel.frames.begin() && frameIt != channel.frames.end() )
            prevFrame = *( frameIt - 1 );

        float alpha = ( channel.elapsedTime - nextFrame.time ) / ( nextFrame.time - prevFrame.time );


        // Get node target of channel
        Node3D* node = dynamic_cast<Node3D*>( Node::NodeLibrary[channel.target] );
        if (!node)
            return;

        switch (channel.path)
        {
            case pathTranslation:
                //node->LocalTransform().Translation() = std::get<Path::pathTranslation>(frameIt->output);
                node->LocalTransform().Translation() = Lerp( std::get<Path::pathTranslation>(prevFrame.output),
                                                             std::get<Path::pathTranslation>(nextFrame.output), 
                                                             alpha );
                break;

            case pathRotation:
                //node->LocalTransform().Rotation() =  std::get<Path::pathRotation>(frameIt->output);
                node->LocalTransform().Rotation() = SphericalLerp( std::get<Path::pathRotation>(prevFrame.output), 
                                                                   std::get<Path::pathRotation>(nextFrame.output),
                                                                   alpha );
                break;

            case pathScale:
                //node->LocalTransform().Scale() =  std::get<Path::pathScale>(frameIt->output);
                node->LocalTransform().Scale() = Lerp( std::get<Path::pathScale>(prevFrame.output), 
                                                       std::get<Path::pathScale>(nextFrame.output),
                                                       alpha );
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

