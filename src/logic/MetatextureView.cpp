#include "MetatextureView.hpp"

MetatextureView::MetatextureView(TileMaterial &material):
    ObjectView(),
    _material(material),
    _diffuse(_material.diffuse
             ? _material.diffuse->create_tile(0, 0, 0, 0)
             : nullptr),
    _emmission(_material.emmission
               ? _material.emmission->create_tile(0, 0, 0, 0)
               : nullptr)
{

}

void MetatextureView::update(
    GameObject &object,
    PyEngine::TimeFloat deltaT)
{
    if (_diffuse) {
        _material.diffuse->update_tile(
            *_diffuse,
            object.x + 0.5, object.y + 0.5,
            object.phi,
            deltaT);

        _diffuse->stream();
    }

    if (_emmission) {
        _material.emmission->update_tile(
            *_emmission,
            object.x + 0.5, object.y + 0.5,
            object.phi,
            deltaT);

        _emmission->stream();
    }

}
