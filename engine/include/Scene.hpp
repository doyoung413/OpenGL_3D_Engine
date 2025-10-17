#pragma once

class Camera;
class Scene
{
public:
	virtual ~Scene() {}

	virtual void Init() = 0;
	virtual void Update(float dt) = 0;
	virtual void Restart() = 0;
	virtual void End() = 0;
	virtual void PostRender(Camera* camera) { (void)camera; }
	virtual void RenderImGui() {}

protected:
};