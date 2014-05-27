#include "MetatextureView.hpp"

/* MetatextureView */

MetatextureView::MetatextureView(TileMaterial &material):
    ObjectView(),
    _material(material),
    _diffuse(_material.diffuse
             ? _material.diffuse->create_tile(0, 0, 0, 0)
             : nullptr),
    _emission(_material.emission
               ? _material.emission->create_tile(0, 0, 0, 0)
               : nullptr)
{

}

void MetatextureView::update_tile(
    const Metatexture &tex,
    MetatextureObject &tile,
    GameObject &object,
    PyEngine::TimeFloat t)
{
    tex.update_tile(
        tile,
        object.x + 0.5, object.y + 0.5,
        object.phi,
        t);
}

void MetatextureView::update(
    GameObject &object,
    PyEngine::TimeFloat deltaT)
{
    if (_diffuse) {
        update_tile(
            *_material.diffuse,
            *_diffuse,
            object,
            0);
        _diffuse->stream();
    }

    if (_emission) {
        update_tile(
            *_material.emission,
            *_emission,
            object,
            0);
        _emission->stream();
    }

}

/* PhiOffsetMetatextureView */

PhiOffsetMetatextureView::PhiOffsetMetatextureView(
        TileMaterial &mat,
        const float phi_offset):
    MetatextureView(mat),
    _phi_offset(phi_offset)
{

}

void PhiOffsetMetatextureView::update_tile(
    const Metatexture &tex,
    MetatextureObject &tile,
    GameObject &object,
    PyEngine::TimeFloat t)
{
    tex.update_tile(
        tile,
        object.x + 0.5, object.y + 0.5,
        object.phi + _phi_offset,
        t);
}
