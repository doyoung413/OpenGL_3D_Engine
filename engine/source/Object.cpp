#include "Object.hpp"

Object::~Object()
{
    for (const auto& component : components)
    {
        component->End();
    }
}

void Object::Update(float dt)
{
	for (auto& comp : components)
	{
		comp->Update(dt);
	}
}
