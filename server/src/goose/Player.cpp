#include <goose/Player.hpp>
#include <goose/JSONWriter.hpp>
Rect<float> Player::getHitbox(){
	const std::lock_guard<std::mutex> lock(*(this->mtx));
	Rect<float> hitbox = this->entity.hitbox;
	return hitbox;
}
Rect<float> Player::getAtkHitbox(){
	Rect<float> hitbox = this->getHitbox();
	shift(hitbox, direction, hitbox.width, hitbox.height);
	if (this->type == 5){
		hitbox.width += 64;
		hitbox.height += 64;
		hitbox.left -= 64;
		hitbox.top -= 64;
	}
	return hitbox;
}
void Player::Render(JSONWriter& writer){
	const std::lock_guard<std::mutex> lock(*(this->mtx));
	writer.Key("x");
	writer.Int(this->entity.m_position.x - 16);
	writer.Key("y");
	writer.Int(this->entity.m_position.y - 16);
	writer.Key("h");
	writer.Int(this->health);
	writer.Key("d");
	writer.Int(this->direction);
	writer.Key("t");
	writer.Int(this->type*this->state);
	writer.Key("n");
	writer.Int(this->entity.m_speed.x);
	writer.Key("m");
	writer.Int(this->entity.m_speed.y);
	writer.Key("p");
	writer.Int(this->ap);
	writer.Key("a");
	writer.Int(this->totalattack);
	writer.Key("k");
	writer.Int(this->totalkill);
	writer.Key("u");
	writer.StartArray();
    for (int i : this->unlocked){
		writer.Int(i);
	}
	writer.EndArray();
}
