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
    std::unique_ptr<MetatextureObject> _emission;

protected:
    virtual void update_tile(
        const Metatexture &tex,
        MetatextureObject &obj,
        GameObject &object,
        PyEngine::TimeFloat t);

public:
    void update(GameObject &object,
                PyEngine::TimeFloat deltaT) override;

};

class PhiOffsetMetatextureView: public MetatextureView
{
public:
    PhiOffsetMetatextureView(TileMaterial &mat,
                             const float phi_offset);

private:
    const float _phi_offset;

protected:
    void update_tile(
        const Metatexture &tex,
        MetatextureObject &obj,
        GameObject &object,
        PyEngine::TimeFloat t) override;

};

#endif
