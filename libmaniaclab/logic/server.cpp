#include "server.hpp"


typedef std::chrono::steady_clock WorldClock;

Server::Server():
    m_level(level_width, level_height),
    m_terminated(false),
    m_game_thread(std::bind(&Server::game_thread, this))
{

}

Server::~Server()
{
    m_terminated = true;
    m_game_thread.join();
}

void Server::game_frame()
{
    m_level.physics().wait_for_frame();

    std::lock_guard<std::shared_timed_mutex> lock(m_interframe_mutex);
    {
        std::lock_guard<std::mutex> lock(m_op_queue_mutex);
        m_op_queue.swap(m_op_buffer);
    }
    m_level.step_singlebuffered_simulation();

    for (LevelOperatorPtr &op: m_op_buffer) {
        (*op)(m_level);
    }
    m_op_buffer.clear();

    m_level.physics().start_frame();
}

void Server::game_thread()
{
    static const std::chrono::microseconds game_frame_duration(
                static_cast<unsigned int>(std::round(Level::time_slice * 1e6f)));
    /* the idea with return_early is that sleep_for will most likely overshoot
     * a little; we set the sleep a bit short so that we're more likely to be
     * on time.
     * as we don't actually subtract this time from the frame duration (but only
     * from the sleep interval), this doesn't change the actual frame rate --
     * it just makes the frames a bit earlier than they should be.*/
    static const std::chrono::microseconds return_early(100);

    // tnext_frame is always in the future when we are on time
    WorldClock::time_point tnext_frame = WorldClock::now();
    while (!m_terminated)
    {
        WorldClock::time_point tnow = WorldClock::now();
        if (tnext_frame > tnow)
        {
            std::chrono::nanoseconds time_to_sleep = tnext_frame - tnow - return_early;
            if (time_to_sleep.count() > 0) {
                std::this_thread::sleep_for(time_to_sleep);
            }
            continue;
        }

        game_frame();

        tnext_frame += game_frame_duration;
    }
}

Server::SyncSafeLock Server::sync_safe_point()
{
    return SyncSafeLock(m_interframe_mutex);
}
