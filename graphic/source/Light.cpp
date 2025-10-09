#include "Light.hpp"
#include "Engine.hpp"
#include "RenderManager.hpp"

void Light::Init()
{
    Engine::GetInstance().GetRenderManager()->RegisterLight(this);
}

void Light::End()
{
    Engine::GetInstance().GetRenderManager()->UnregisterLight(this);
}