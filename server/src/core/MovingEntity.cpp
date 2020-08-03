#include <core/MovingEntity.hpp>
#include <cassert>
MovingEntity::MovingEntity(float x, float y, ObjMap* map): MovingEntity(x,y,0,0,map){}
MovingEntity::MovingEntity(float x, float y, int w, int h, ObjMap* map):
	m_position(x, y), m_width(w), m_height(h), m_lastPosition(x, y), m_map(map)
{
	this->hitbox = {m_position.x - m_width/2.f, m_position.y - m_height/2.f, static_cast<float>(m_width), static_cast<float>(m_height)};
}

void MovingEntity::warpto(float x, float y){
	m_lastPosition = glm::vec2(x,y);
	m_position = glm::vec2(x,y);
	m_speed = glm::vec2(0.0f,0.0f);
	isOnGround = false;
	wasOnGround = false;
}
void MovingEntity::Update(float dt) {
// Save current touching info
	assert(m_map);
	ObjMap& map_ref = *m_map; 
	this->m_lastPosition = this->m_position;
	this->m_lastSpeed = this->m_speed;
	this->pushedLeftWall = this->pushesLeftWall;
	this->pushedRightWall = this->pushesRightWall;
	this->wasOnGround = this->isOnGround;
	this->wasAtCeiling = this->isAtCeiling;
	this->m_position += (this->m_speed * dt);
	this->pushesRightWall=false;
	this->pushesLeftWall=false;
	this->isAtCeiling=false;
	this->isOnGround=false;
	this->onOneWayPlatform = false;
	this->lastHitbox = this->hitbox;
	this->hitbox = {m_position.x - m_width/2.f, m_position.y - m_height/2.f, static_cast<float>(m_width), static_cast<float>(m_height)};
//	Rect<float> lastHitbox = {m_lastPosition.x - m_width/2.f, m_lastPosition.y - m_height/2.f, static_cast<float>(m_width), static_cast<float>(m_height)}; 
	int lowX = std::min(lastHitbox.left, hitbox.left);
	int highX = std::max(lastHitbox.left + lastHitbox.width, hitbox.left + hitbox.width);
	int lowY = std::min(lastHitbox.top, hitbox.top);
	int highY = std::max(lastHitbox.top + lastHitbox.height, hitbox.top + hitbox.height);
	for (auto& surf : map_ref.surfaces){
		auto& i = surf;
		if ((highY >= i.hitbox.top) && (lowY <= i.hitbox.top + i.hitbox.height)){
			if ((i.flags & WallType::RWALL)&&(lastHitbox.left + lastHitbox.width <= i.hitbox.left)&&(hitbox.left + hitbox.width >= i.hitbox.left)){
				m_lastPosition.x = i.hitbox.left - (m_width/2);
				m_speed.x = 0.0f;
				this->pushesRightWall=true;
			}
			if ((i.flags & WallType::LWALL)&&(lastHitbox.left >= i.hitbox.left + i.hitbox.width)&&(hitbox.left <= i.hitbox.left + i.hitbox.width)){
				m_lastPosition.x = i.hitbox.left + i.hitbox.width + (m_width/2);
				m_speed.x = 0.0f;
				this->pushesLeftWall=true;
			}
		}
		if ((highX >= i.hitbox.left) && (lowX <= i.hitbox.left + i.hitbox.width)){
			if ((i.flags & WallType::CEIL) && (lastHitbox.top >= i.hitbox.top + i.hitbox.height)&&(hitbox.top <= i.hitbox.top + i.hitbox.height)){
				m_lastPosition.y = i.hitbox.top + i.hitbox.height + (m_height/2);
				this->isAtCeiling=true;
				m_speed.y = 0;
			}
			if ((i.flags & WallType::FLOOR)){
				if ((lastHitbox.top + lastHitbox.height <= i.hitbox.top) && (hitbox.top + hitbox.height >= i.hitbox.top)){
					m_lastPosition.y = i.hitbox.top-(m_height/2);
					this->isOnGround=true;
					this->onOneWayPlatform = (i.flags & WallType::ONEWAY);
					m_speed.y = 0;
				}
			}
		}
	}
	m_position = m_lastPosition + (m_speed*dt);
	this->hitbox = {m_position.x - m_width/2.f, m_position.y - m_height/2.f, static_cast<float>(m_width), static_cast<float>(m_height)};
	if (m_position.y > (map_ref.bbox.top + map_ref.bbox.height)){
		dead = true;
	}
}
