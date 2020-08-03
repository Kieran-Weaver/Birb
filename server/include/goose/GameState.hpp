#ifndef GAMESTATE_HPP
#define GAMESTATE_HPP
#include <simplews/server_ws.hpp>
#include <goose/Player.hpp>
#include <goose/Food.hpp>
#include <mutex>
#include <random>
#include <unordered_map>
#include <memory>
using WsServer = SimpleWeb::SocketServer<SimpleWeb::WS>;
using PlayerMap = std::unordered_map<std::shared_ptr<WsServer::Connection>, Player>;
struct Type{
	int atk;
	int def;
	int spd;
	int cost;
	int width;
	int height;
};
struct GameState{
	GameState(const std::string& filename, const std::string& mapname);
	std::mutex mtx;
	std::mt19937 rng;
	std::uniform_int_distribution<int> xdist;
	std::uniform_int_distribution<int> ydist;
	PlayerMap players;
	ObjMap internalMap;
	std::vector<Food> food;
	std::vector<Type> types;
	std::string wsdata;
	WsServer server;
};
#endif
