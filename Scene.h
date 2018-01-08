/*
author: atao 1628025718@qq.com
*/
#pragma once
#include <iostream>
#include <unordered_map>
#include <vector>
#include <unordered_set>
#include "Entity.h"


namespace TT
{
//将普通坐标转换为9宫格坐标的转换比率
//划分的基本原则：将一个视野（或者略大于）在宽和高上9等分，就可以得到GX,GY的实际大小。
#define GX 8
#define GY 6
//这里乘以100，完全可以将一个100*100的2D场景转换的9宫格地图中每个格子唯一标示了
//这里有个技巧，gy最大是几位数，比如100的最大y坐标，GY=6的时候，那么gy最大就是16，是2位数， gx就乘以10^2再加上gy的值就可以构造唯一gid了。
#define FACTOR 100

	// 场景
	class Scene
	{
	public:
		Scene(int width=100, int height=100) :
			width_(width), height_(height)
		{
			InitGrids();
		}
		Scene() = delete;
		~Scene() {};
	public:
		//aoi api
		//定义三个AOI相关的接口，用于实体进入、离开、在场景中移动
		void Enter(const Entity& entity);
		void Enter(const uint64_t id, const int gid);
		void Leave(const Entity& entity);
		void Leave(const uint64_t id, const int gid);
		void Move(const Entity& entity, const int x, const int y);
		// debug api
		int width() const { return width_; }
		int height() const { return height_; }
		void Stat() const;
	
	private:
		int width_;  //场景宽
		int height_; //场景高
		//9宫格
		//大格子采用{GX,GY}表示，为了存储方便使用,将GX，GY转换为一个Gid，这里演示使用：GX*100+GY = Gid，具体可以根据场景实际大小
		//采用相应的公式，当然也可以直接存储一个tuple。
		//每个gid对应一个视野中的一块区域，因为涉及到频繁的查找和删除操作，set是很适合的数据结构
		typedef int Gid;
		typedef std::unordered_map<Gid, std::unordered_set<uint64_t>> Grids;
		Grids grids_;
		//8个方向
		enum DIR {
			DIR_NULL = 0, //基本方向合法检查
			DIR_UP = 1,
			DIR_DOWN,
			DIR_LEFT,
			DIR_RIGHT,
			DIR_LEFT_UP,
			DIR_RIGHT_UP,
			DIR_LEFT_DOWN,
			DIR_RIGHT_DOWN = 8
		};

	private:
		//初始化9宫格数据
		void InitGrids();
		//对应的AOI广播
		void EnterBroadcast(const Entity& entity, const int x, const int y);
		void LeaveBroadcast(const Entity& entity, const int x, const int y);
		void MoveBroadcast(const Entity& entity, const int x, const int y);
		//返回实体对应的Gid
		int GetGid(const int x, const int y) const;
		//九宫格坐标到九宫格id的转换
		int GxGy2Gid(const int gx, const int gy) const;
		//视野内的实体
		void ViewEntities(std::unordered_set<uint64_t>& entities, const int x, const int y);
		//视野内的格子（大格子）
		void ViewGrids(std::unordered_set<int>& view_grids, const int x, const int y);
		//格子内的实体
		void GridEntities(std::unordered_set<uint64_t>& entities, int gid);
		void Notity(const Entity& entity, const std::unordered_set<int>& leave_grids, const std::unordered_set<int>& enter_grids);
		//计算移动的方向
		DIR GetDir(const int x1, const int y1, const int x2, const int y2);
		//8个方向上的移动
		void move_up(const Entity& entity, int x, int y);
		void move_down(const Entity& entity, int x, int y);
		void move_left(const Entity& entity, int x, int y);
		void move_right(const Entity& entity, int x, int y);
		void move_left_up(const Entity& entity, int x, int y);
		void move_right_up(const Entity& entity, int x, int y);
		void move_left_down(const Entity& entity, int x, int y);
		void move_right_down(const Entity& entity, int x, int y);

	};
}//namespace TT

