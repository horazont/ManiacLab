#include "GameObject.hpp"

#include "Errors.hpp"
#include "Physics.hpp"

void GameObject::setMovement(Movement *movement)
{
    if (_movement) {
        throw ProgrammingError("Attempt to replace movement of a moving game-object.");
    }
    _movement = movement;
}
