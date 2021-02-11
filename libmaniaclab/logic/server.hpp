#ifndef ML_SERVER_H
#define ML_SERVER_H

#include <thread>
#include <mutex>
#include <shared_mutex>
#include <atomic>

#include "logic/level.hpp"


typedef std::chrono::steady_clock WorldClock;


class Server
{
public:
    typedef std::shared_lock<std::shared_timed_mutex> SyncSafeLock;

public:
    Server();
    ~Server();

private:
    Level m_level;

    /* guarded by m_op_queue_mutex */
    std::mutex m_op_queue_mutex;
    std::vector<LevelOperatorPtr> m_op_queue;

    /* unguarded */
    std::atomic_bool m_terminated;
    std::thread m_game_thread;

    /* owned by m_game_thread */
    std::vector<LevelOperatorPtr> m_op_buffer;

    std::shared_timed_mutex m_interframe_mutex;

protected:
    void game_frame();
    void game_thread();

public:
    inline Level &state()
    {
        return m_level;
    }

    inline const Level &state() const
    {
        return m_level;
    }

    inline void enqueue_op(LevelOperatorPtr &&op) {
        std::lock_guard<std::mutex> lock(m_op_queue_mutex);
        m_op_queue.push_back(std::move(op));
    }

    SyncSafeLock sync_safe_point();
};

#endif
