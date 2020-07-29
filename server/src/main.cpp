#include <simplews/server_ws.hpp>
#include <core/Map.hpp>
#include <core/MovingEntity.hpp>
#include <goose/Player.hpp>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <thread>
#include <mutex>
#include <chrono>
#include <util/PRNG.hpp>
#include <string>
#include <fstream>
#include <algorithm>

std::mt19937 rng = SeedRNG();
std::uniform_int_distribution<int> xdist(0, 3992);
std::uniform_int_distribution<int> ydist(0, 2992);

using WsServer = SimpleWeb::SocketServer<SimpleWeb::WS>;

using namespace rapidjson;
Document document;

std::mutex mtx;
struct Food{
	Food(float x, float y) : hitbox(x, y, 8.f, 8.f){}
	Rect<float> hitbox;
	float replenish = 1.f;
};

using PlayerMap = std::unordered_map<std::shared_ptr<WsServer::Connection>, Player>;
using FoodList = std::vector<Food>;

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

void writePlayers(std::shared_ptr<PlayerMap> players, std::shared_ptr<FoodList> food, std::shared_ptr<std::string> data, std::shared_ptr<WsServer> server){
	rapidjson::StringBuffer s;
	rapidjson::Writer<rapidjson::StringBuffer> writer(s);
	writer.StartObject();
	writer.Key("p");
	writer.StartObject();	
	const std::lock_guard<std::mutex> lock(mtx);
	std::vector<std::shared_ptr<WsServer::Connection>> todelete;
	const Value& types = document["types"];
	for (auto& p : *players){
		auto& player = p.second;
		const Value& type = types[player.type];
		{
			std::lock_guard<std::mutex> lock(*(player.mtx));
			player.entity.m_width = type["w"].GetInt();
			player.entity.m_height = type["h"].GetInt();
			player.entity.Update(15.f/1000.f);
			player.attackTimer();
		}
		if (!player.name.empty()){
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
						for (auto& p2 : *players){
							auto& otherPlayer = p2.second;
							std::lock_guard<std::mutex> lock(*(otherPlayer.mtx));
							const auto& ptype = types[otherPlayer.type];
							if (otherPlayer.entity.hitbox.Intersects(atkbox) and otherPlayer.type != 21 and player.type != 21){
								player.totalattack++;
								player.ap++;
								float tochange = ((float)type["a"].GetInt())/5.f-((float)ptype["d"].GetInt())/10.f;
								otherPlayer.health -= std::max(tochange, 1.f);
								if (otherPlayer.health <= 0.f)
									player.totalkill++;
							}
						}
						player.attacked = true;
					}
				}				
				int addcount = 0;
				for (auto f = food->begin(); f != food->end();){
					if (f->hitbox.Intersects(hitbox) and player.type != 21){
						{
							std::lock_guard<std::mutex> lock(*(player.mtx));
							player.health = std::min(player.maxHealth,player.health + f->replenish);
						}
						f = food->erase(f);
						addcount++;
					}else{
						f++;
					}
				}
				for (int i = 0; i < addcount; i++){
					food->emplace_back(xdist(rng), ydist(rng));
				}
				player.Render(writer);
				writer.EndObject();
			}
		}
	}
	
	const auto& conns = server->get_connections();
	for (auto& conn : *players){
		if (conns.count(conn.first) == 0){
			todelete.emplace_back(conn.first);
		}
	}
	for (auto& conn : todelete){
		if (players->count(conn)){
			players->erase(conn);
		}
	}
	writer.EndObject();
	writer.Key("f");
	writer.StartArray();
	for (auto& f : *food) {
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
	*data = s.GetString();
}

void physicsThread(std::shared_ptr<PlayerMap> players, std::shared_ptr<FoodList> food, std::shared_ptr<std::string> data, std::shared_ptr<WsServer> server){
	std::this_thread::sleep_for(std::chrono::milliseconds(15));
	while (true){
		const auto& start = std::chrono::high_resolution_clock::now();
		writePlayers(players, food, data, server);
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
	std::shared_ptr<WsServer> server = std::make_shared<WsServer>();
	server->config.address = std::string(argv[1]);
	server->config.port = std::stoul(argv[2]);
	auto& goosegame = server->endpoint["^/.*/?$"];
	std::shared_ptr<PlayerMap> players = std::make_shared<PlayerMap>();
	std::shared_ptr<FoodList> food = std::make_shared<FoodList>();
	for (int i = 0; i < 50; i++)
		food->emplace_back(xdist(rng), ydist(rng));
	std::shared_ptr<ObjMap> smap = std::make_shared<ObjMap>("resources/map.json");
	std::ifstream ifs("resources/types.json");
	IStreamWrapper isw(ifs);
	document.ParseStream(isw);
	std::shared_ptr<std::string> data = std::make_shared<std::string>();
	std::thread phyThread(physicsThread, players, food, data, server);
	phyThread.detach();
	goosegame.on_open = [&](std::shared_ptr<WsServer::Connection> connection){
		std::lock_guard<std::mutex> lock(mtx);
		(*players)[connection] = Player(xspawn, yspawn, smap);
		(*players)[connection].entity.m_speed = {0.f, 0.f};
	};
	goosegame.on_close = [&players, &smap](std::shared_ptr<WsServer::Connection> connection, int status, const std::string& reason){
		std::lock_guard<std::mutex> lock(mtx);
		if (players->count(connection)){
			players->erase(connection);
		}
	};
	goosegame.on_message = [&](std::shared_ptr<WsServer::Connection> connection, std::shared_ptr<WsServer::InMessage> in_message){
		std::vector<std::string> splitvec;
		split(in_message->string(),',', splitvec);
		{
			std::lock_guard<std::mutex> guard(mtx);
			if ((!players->count(connection))||(players->at(connection).health <= 0.f)){
				connection->send_close(1000);
				return;
			}
		}
		if (splitvec[0] == "n") {
			std::lock_guard<std::mutex> guard(*(players->at(connection).mtx));
			if (players->at(connection).name == ""){
				for (auto& conn : *players){
					if (conn.second.name == splitvec[1]){
						connection->send("no", logErrorCode);
						return;
					}
				}
				players->at(connection).name = splitvec[1];
				players->at(connection).entity.m_position = {xspawn, yspawn};
				players->at(connection).entity.m_speed = {0.f, 0.f};
				connection->send("ok", logErrorCode);
			}else{
				const Value& types = document["types"];
				int typex = std::stoul(splitvec[2]);
				if (std::find(players->at(connection).unlocked.begin(), players->at(connection).unlocked.end(), typex) == players->at(connection).unlocked.end()){
					const Value& type = types[typex];
					int cost = type["c"].GetInt();
					if (cost > players->at(connection).ap){
						connection->send("no", logErrorCode);
						return;
					}
					players->at(connection).ap -= cost;
					players->at(connection).unlocked.push_back(typex);
				}else{
					const Value& type = types[typex];
					players->at(connection).entity.m_width = type["w"].GetInt();
					players->at(connection).entity.m_height = type["h"].GetInt();
					players->at(connection).entity.m_position = {xspawn, yspawn};
					players->at(connection).type = typex;
					players->at(connection).entity.m_speed = {0.f, 0.f};
					players->at(connection).entity.Update(15.f/1000.f);
				}
				connection->send("ok", logErrorCode);
			}
		} else if (splitvec[0] == "m") {
			std::vector<int> keystates;
			for (int i = 1; i < splitvec.size(); i++){
				keystates.emplace_back(std::stoul(splitvec[i]));
			}
			std::lock_guard<std::mutex> guard(*(players->at(connection).mtx));
			const Value& types = document["types"];
			const Value& type = types[players->at(connection).type];
			float pspeed = ((float)type["s"].GetInt()) * 30.f;
			players->at(connection).entity.m_speed = { (keystates[3] - keystates[2]) * pspeed, (keystates[0] - keystates[1]) * pspeed};
			if (!players->at(connection).attackTimer.getTime()){
				if ((keystates[4]) && (!players->at(connection).attackTimer.getDelay())){
					players->at(connection).state = -1;
					players->at(connection).attacked = false;
					players->at(connection).attackTimer.setTime(6);
					players->at(connection).attackTimer.setDelay(24);
				} else {
					players->at(connection).state = 1;
				}
			}
			if (std::fabs(players->at(connection).entity.m_speed.y) > std::fabs(players->at(connection).entity.m_speed.x)){
				switch (keystates[0] - keystates[1]){
					case 1: // Down
						players->at(connection).direction = 2;
						break;
					case -1: // Up
						players->at(connection).direction = 0;
						break;
					default:break;
				}
				switch (keystates[3] - keystates[2]){
					case 1: // right
						players->at(connection).direction = 1;
						break;
					case -1: // left
						players->at(connection).direction = 3; 
						break;
					default:break;
				}
			} else {
				switch (keystates[3] - keystates[2]){
					case 1: // right
						players->at(connection).direction = 1;
						break;
					case -1: // left
						players->at(connection).direction = 3;
						break;
					default:break;
				}
				switch (keystates[0] - keystates[1]){
					case 1: // Down
						players->at(connection).direction = 2;
						break;
					case -1: // Up
						players->at(connection).direction = 0;
						break;
					default:break;
				}
			}
			connection->send(*data, logErrorCode);
		}
	};
	server->start();
	return 0;
}
