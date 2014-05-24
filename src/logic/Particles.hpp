#ifndef _ML_PARTICLES_H
#define _ML_PARTICLES_H

#include <forward_list>
#include <list>
#include <vector>
#include <array>

#include <CEngine/IO/Time.hpp>

template <typename _Particle, size_t _chunk_size>
class GenericParticleSystem
{
public:
    static constexpr size_t chunk_size = _chunk_size;
    typedef _Particle Particle;

    typedef std::list<Particle*> container_type;
    typedef typename container_type::iterator iterator;
    typedef typename container_type::const_iterator const_iterator;


public:
    GenericParticleSystem() = default;
    GenericParticleSystem(const GenericParticleSystem &ref) = delete;
    GenericParticleSystem &operator=(const GenericParticleSystem &ref) = delete;
    GenericParticleSystem(GenericParticleSystem &&ref) = default;
    GenericParticleSystem &operator=(GenericParticleSystem &&ref) = default;

private:
    typedef std::array<Particle, chunk_size> Chunk;

    std::vector<std::unique_ptr<Chunk>> _storage;

protected:
    std::forward_list<Particle*> _available;
    container_type _active;

protected:
    inline Particle *allocate()
    {
        if (_available.empty()) {
            grow();
        }
        Particle *result = _available.front();
        _available.pop_front();
        result->alive = true;
        result->age = 0;
        return result;
    }

    void grow()
    {
        Chunk *new_chunk = new Chunk();
        for (auto &item: *new_chunk) {
            _available.push_front(&item);
        }
        _storage.push_back(std::unique_ptr<Chunk>(new_chunk));
    }

    inline void release(Particle *part)
    {
        _available.push_front(part);
    }

public:
    inline typename container_type::size_type active_size()
    {
        return _active.size();
    }

    inline iterator begin()
    {
        return _active.begin();
    }

    inline const_iterator cbegin()
    {
        return _active.cbegin();
    }

    void clear()
    {
        _active.clear();
        _available.clear();
        for (auto &chunk: _storage) {
            for (auto &item: *chunk) {
                _available.push_front(&item);
            }
        }
    }

    inline const_iterator cend()
    {
        return _active.cend();
    }

    inline iterator end()
    {
        return _active.end();
    }

};

enum class ParticleType {
    PARTICLE_FIRE
};

struct PhysicsParticle
{
    bool alive;
    float age;
    float lifetime;
    float x, y;
    float vx, vy;
    float ax, ay;

    ParticleType type;
};

class ParticleSystem: public GenericParticleSystem<PhysicsParticle, 1024>
{
public:
    typedef std::function<void(Particle*)> Generator;

public:
    Particle *spawn();
    void spawn_generator(size_t n, const Generator &generator);
    void update(PyEngine::TimeFloat deltaT);

};

#endif
