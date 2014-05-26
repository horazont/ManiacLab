#ifndef _ML_METATEXTURE_VIEW_H
#define _ML_METATEXTURE_VIEW_H

#include "GameObject.hpp"
#include "TileMaterial.hpp"

struct MetatextureView: public ObjectView
{
public:
    MetatextureView(TileMaterial &material);

private:
    TileMaterial &_material;
    std::unique_ptr<MetatextureObject> _diffuse;
    std::unique_ptr<MetatextureObject> _emmission;

public:
    void update(GameObject &object,
                PyEngine::TimeFloat deltaT) override;

};

#endif
