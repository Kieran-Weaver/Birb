#ifndef MovingEntity_hpp
#define MovingEntity_hpp
#include <core/Map.hpp>
#include <util/Rect.hpp>
#include <glm/glm.hpp>
#include <memory>
class MovingEntity {
public:
	glm::vec2 m_position = {0.f, 0.f};
	glm::vec2 m_speed = {0.f, 0.f};
	int m_width = 0;
	int m_height = 0;
	float maxFallSpeed = 600.0f;
	bool dead=false;
	Rect<float> hitbox = {};
public:
	MovingEntity() = default;
	MovingEntity(float x, float y, ObjMap* map);
	MovingEntity(float x, float y, int w, int h, ObjMap* map);
	void Update(float dt);
	void warpto(float x, float y);
protected:
	bool pushedRightWall=false, pushesRightWall=false;
	bool pushedLeftWall=false, pushesLeftWall=false;
	bool wasOnGround=false, isOnGround=false;
	bool wasAtCeiling=false, isAtCeiling=false;
	bool onOneWayPlatform = false;
	bool dropFromOneWay = false;
	glm::vec2 m_lastSpeed = {0.f, 0.f};
	glm::vec2 m_lastPosition = {0.f, 0.f};
	ObjMap* m_map = nullptr;
	Rect<float> lastHitbox = {};
};

#endif
