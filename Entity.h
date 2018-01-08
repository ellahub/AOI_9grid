/*
author: atao 1628025718@qq.com
*/
#pragma once
#include <cstdint>
namespace TT
{
	//场景实体
	class Entity
	{
	public:
		Entity(uint64_t id) :id_(id), x_(0), y_(0) {}
		Entity(uint64_t id, int x, int y) :id_(id), x_(x), y_(y) {}
		Entity() = delete;
		Entity& operator=(const Entity& rhs) = delete;
		~Entity() {};

	public:
		//API
		uint64_t id() const { return id_; }
		uint16_t x() const { return x_; }
		void set_x(uint16_t x) { x_ = x; }
		uint16_t y() const { return y_; }
		void set_y(uint16_t y) { y_ = y; }

	private:
		uint64_t id_; //ID
		uint16_t x_;  //坐标
		uint16_t y_;
	};
}//namespace TT

