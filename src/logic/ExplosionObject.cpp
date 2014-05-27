#include "ExplosionObject.hpp"

static const CellStamp explosion_object_stamp(
    {
        false, false, false, false, false,
        false, false, false, false, false,
        false, false, false, false, false,
        false, false, false, false, false,
        false, false, false, false, false,
    }
);

static ObjectInfo explosion_object_info(
    true,
    false,
    false,
    false,
    false,
    false,
    false,
    0.5,
    1.0,
    explosion_object_stamp);


/* ExplosionView */

ExplosionView::ExplosionView():
    ObjectView()
{

}

/* ExplosionObject */

ExplosionObject::ExplosionObject(Level *level):
    GameObject(explosion_object_info, level)
{
    die_at = level->get_ticks() + EXPLOSION_BLOCK_LIFETIME;
}

void ExplosionObject::update()
{
    GameObject::update();
    if (ticks >= die_at) {
        destruct_self();
    }
}

void ExplosionObject::setup_view()
{
    view = std::unique_ptr<ObjectView>(new ObjectView());
}

void ExplosionObject::setup_view(TileMaterialManager &matman)
{
    setup_view();
}
