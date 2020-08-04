#include <core/Map.hpp>
#include <goose/Player.hpp>
#include <goose/Food.hpp>
#include <goose/GameState.hpp>
#include <goose/JSONWriter.hpp>
#include <thread>
#include <mutex>
#include <chrono>
#include <string>
#include <fstream>
#include <algorithm>

template<typename Container>
void split(const std::string& total, const char delim, Container& c){
	std::string temp;
	for (auto& it : total){
		if (it == delim){
			c.emplace_back(temp);
			temp = "";
		} else {
			temp += it;
		}
	}
	if (!temp.empty()){
		c.emplace_back(temp);
	}
}

void logErrorCode(const SimpleWeb::error_code& ec){
	if (ec){
		std::cout << "Error: " << ec.message() << std::endl;
	}
}

void writePlayers(std::shared_ptr<GameState> state){
	JSONWriter writer;
	writer.StartObject();
	writer.Key("p");
	writer.StartObject();	
	const std::lock_guard<std::mutex> lock(state->mtx);
	std::vector<std::shared_ptr<WsServer::Connection>> todelete;
	
	for (auto& p : state->players){
		auto& player = p.second;
		const Type& type = state->types[player.type];
		std::string name;
		{
			std::lock_guard<std::mutex> lock(*(player.mtx));
			player.entity.m_width = type.width;
			player.entity.m_height = type.height;
			player.entity.Update(15.f/1000.f);
			player.attackTimer();
			name = player.name;
		}
		if (!name.empty()){
			if (player.health <= 0.f){
				p.first->send_close(1000);
				todelete.emplace_back(p.first);
			} else {
				writer.Key(player.name.c_str());
				writer.StartObject();
				Rect<float> atkbox = player.getAtkHitbox();
				Rect<float> hitbox = player.getHitbox();
				if ((player.state == -1) && (!player.attacked)){
					Rect<float> spawn(361*5, 260*5, 79*5, 80*5);
					Rect<float> shop1(0, 0, 120*5, 71*5);
					Rect<float> shop2(681*5, 530*5, 120*5, 71*5);
					if (hitbox.Intersects(spawn)){
						writer.Key("q");
						writer.Int(1);
					}else if (hitbox.Intersects(shop1) and player.type != 21){
						writer.Key("s");
						writer.StartArray();
						std::vector<int> shophas = {2, 3, 4, 5, 6};
						{
							std::lock_guard<std::mutex> lock(*(player.mtx));
							for (int i : shophas){
								if (std::find(player.unlocked.begin(), player.unlocked.end(), i) == player.unlocked.end())
									writer.Int(i);
							}
						}
						writer.EndArray();						
					} else if (hitbox.Intersects(shop2) and player.type != 21) {
					} else {
						for (auto& p2 : state->players){
							auto& otherPlayer = p2.second;
							std::lock_guard<std::mutex> lock(*(otherPlayer.mtx));
							const auto& ptype = state->types[otherPlayer.type];
							if (otherPlayer.entity.hitbox.Intersects(atkbox) && otherPlayer.type != 21 && player.type != 21){
								player.totalattack++;
								player.ap++;
								float tochange = type.atk/5.f-ptype.def/10.f;
								otherPlayer.health -= std::max(tochange, 1.f);
								if (otherPlayer.health <= 0.f)
									player.totalkill++;
							}
						}
						player.attacked = true;
					}
				}				
				int addcount = 0;
				for (auto f = state->food.begin(); f != state->food.end();){
					if (f->hitbox.Intersects(hitbox) and player.type != 21){
						{
							std::lock_guard<std::mutex> lock(*(player.mtx));
							player.health = std::min(player.maxHealth,player.health + f->replenish);
						}
						f = state->food.erase(f);
						addcount++;
					}else{
						f++;
					}
				}
				for (int i = 0; i < addcount; i++){
					state->food.emplace_back(state->xdist(state->rng), state->ydist(state->rng));
				}
				player.Render(writer);
				writer.EndObject();
			}
		}
	}
	
	const auto& conns = state->server.get_connections();
	for (auto& conn : state->players){
		if (conns.count(conn.first) == 0){
			todelete.emplace_back(conn.first);
		}
	}
	for (auto& conn : todelete){
		if (state->players.count(conn)){
			state->players.erase(conn);
		}
	}
	writer.EndObject();
	writer.Key("f");
	writer.StartArray();
	for (auto& f : state->food) {
		writer.StartObject();
		writer.Key("r");
		writer.Int(f.replenish);
		writer.Key("x");
		writer.Int(f.hitbox.left);
		writer.Key("y");
		writer.Int(f.hitbox.top);
		writer.EndObject();
	}
	writer.EndArray();
	writer.EndObject();
	state->wsdata = writer.get();
}

void physicsThread(std::shared_ptr<GameState> state){
	std::this_thread::sleep_for(std::chrono::milliseconds(15));
	while (true){
		const auto& start = std::chrono::high_resolution_clock::now();
		writePlayers(state);
		std::this_thread::sleep_until(start + std::chrono::milliseconds(15));
	}
}
int main(int argc, char** argv){
	if (argc < 2){
		std::cout << "Usage: ./Birb IP PORT" << std::endl;
		return 1;
	}
	uint32_t xspawn = 2000;
	uint32_t yspawn = 1500;
	std::shared_ptr<GameState> state = std::make_shared<GameState>("resources/types.json", "resources/map.json");
	state->server.config.address = std::string(argv[1]);
	state->server.config.port = std::stoul(argv[2]);
	auto& goosegame = state->server.endpoint["^/.*/?$"];
	for (int i = 0; i < 50; i++)
		state->food.emplace_back(state->xdist(state->rng), state->ydist(state->rng));
	std::thread phyThread(physicsThread, state);
	phyThread.detach();
	goosegame.on_open = [=](std::shared_ptr<WsServer::Connection> connection){
		std::lock_guard<std::mutex> lock(state->mtx);
		state->players[connection] = Player(xspawn, yspawn, &(state->internalMap));
		state->players[connection].entity.m_speed = {0.f, 0.f};
	};
	goosegame.on_close = [=](std::shared_ptr<WsServer::Connection> connection, int status, const std::string& reason){
		std::lock_guard<std::mutex> lock(state->mtx);
		if (state->players.count(connection)){
			state->players.erase(connection);
		}
	};
	goosegame.on_message = [=](std::shared_ptr<WsServer::Connection> connection, std::shared_ptr<WsServer::InMessage> in_message){
		std::vector<std::string> splitvec;
		split(in_message->string(),',', splitvec);
		{
			std::lock_guard<std::mutex> guard(state->mtx);
			if ((!state->players.count(connection))||(state->players.at(connection).health <= 0.f)){
				connection->send_close(1000);
				return;
			}
		}
		if (splitvec[0] == "n") {
			std::lock_guard<std::mutex> guard(*(state->players.at(connection).mtx));
			if (state->players.at(connection).name == ""){
				for (auto& conn : state->players){
					if (conn.second.name == splitvec[1]){
						connection->send("no", logErrorCode);
						return;
					}
				}
				state->players.at(connection).name = splitvec[1];
				state->players.at(connection).entity.m_position = {xspawn, yspawn};
				state->players.at(connection).entity.m_speed = {0.f, 0.f};
				connection->send("ok", logErrorCode);
			}else{
				
				int typex = std::stoul(splitvec[2]);
				if (std::find(state->players.at(connection).unlocked.begin(), state->players.at(connection).unlocked.end(), typex) == state->players.at(connection).unlocked.end()){
					const Type& type = state->types[typex];
					int cost = type.cost;
					if (cost > state->players.at(connection).ap){
						connection->send("no", logErrorCode);
						return;
					}
					state->players.at(connection).ap -= cost;
					state->players.at(connection).unlocked.push_back(typex);
				}else{
					const Type& type = state->types[typex];
					state->players.at(connection).entity.m_width = type.width;
					state->players.at(connection).entity.m_height = type.height;
					state->players.at(connection).entity.m_position = {xspawn, yspawn};
					state->players.at(connection).type = typex;
					state->players.at(connection).entity.m_speed = {0.f, 0.f};
					state->players.at(connection).entity.Update(15.f/1000.f);
				}
				connection->send("ok", logErrorCode);
			}
		} else if (splitvec[0] == "m") {
			std::vector<int> keystates;
			for (int i = 1; i < splitvec.size(); i++){
				keystates.emplace_back(std::stoul(splitvec[i]));
			}
			std::lock_guard<std::mutex> guard(*(state->players.at(connection).mtx));
			
			const Type& type = state->types[state->players.at(connection).type];
			float pspeed = type.spd * 30.f;
			state->players.at(connection).entity.m_speed = { (keystates[3] - keystates[2]) * pspeed, (keystates[0] - keystates[1]) * pspeed};
			if (!state->players.at(connection).attackTimer.getTime()){
				if ((keystates[4]) && (!state->players.at(connection).attackTimer.getDelay())){
					state->players.at(connection).state = -1;
					state->players.at(connection).attacked = false;
					state->players.at(connection).attackTimer.setTime(6);
					state->players.at(connection).attackTimer.setDelay(24);
				} else {
					state->players.at(connection).state = 1;
				}
			}
			if (std::fabs(state->players.at(connection).entity.m_speed.y) > std::fabs(state->players.at(connection).entity.m_speed.x)){
				switch (keystates[0] - keystates[1]){
					case 1: // Down
						state->players.at(connection).direction = 2;
						break;
					case -1: // Up
						state->players.at(connection).direction = 0;
						break;
					default:break;
				}
				switch (keystates[3] - keystates[2]){
					case 1: // right
						state->players.at(connection).direction = 1;
						break;
					case -1: // left
						state->players.at(connection).direction = 3; 
						break;
					default:break;
				}
			} else {
				switch (keystates[3] - keystates[2]){
					case 1: // right
						state->players.at(connection).direction = 1;
						break;
					case -1: // left
						state->players.at(connection).direction = 3;
						break;
					default:break;
				}
				switch (keystates[0] - keystates[1]){
					case 1: // Down
						state->players.at(connection).direction = 2;
						break;
					case -1: // Up
						state->players.at(connection).direction = 0;
						break;
					default:break;
				}
			}
			connection->send(state->wsdata, logErrorCode);
		}
	};
	state->server.start();
	return 0;
}
