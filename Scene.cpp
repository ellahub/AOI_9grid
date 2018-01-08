/*
author: atao 1628025718@qq.com
*/
#include "Scene.h"
using namespace TT;

//初始化场景9宫格数据
void Scene::InitGrids()
{
	int width = width_;
	int height = height_;
	//将场景中的每个坐标都转换为9宫格坐标
	for (int w = 0; w <= width; ++w)
	{
		for (int h = 0; h <= height; ++h)
		{
			int gid = GetGid(w, h);
			auto it = grids_.find(gid);
			//没有初始化过就初始化一次
			if (it == grids_.end()) 
			{
				//std::vector<uint64_t> grid_entities = {};
				grids_.insert({ gid, {} });
				//printf("insert %d\n", gid);
			}
			
		}
	}
}

//debug
void Scene::Stat() const {
	std::cout << grids_.size() << std::endl;
}

//进入场景
void Scene::Enter(const Entity& entity)
{
	//实体进入场景，将自己进入的信息广播到对应的区域
	//同时将对应区域中其他实体的状态同步到实体
	int gid = GetGid(entity.x(), entity.y());
	Enter(entity.id(), gid);
	EnterBroadcast(entity, entity.x(), entity.y());
}

void Scene::Enter(const uint64_t id, const int gid)
{
	auto it = grids_.find(gid);
	if (it != grids_.end())
	{
		auto find_it = it->second.find(id);
		if (find_it == it->second.end())
		{
			it->second.insert(id);
		}
	}
}
// 离开场景
void Scene::Leave(const Entity& entity)
{
	//实体离开场景或者视野
	//发送离开通知到视野内的其他视野、更新该区域的实体集合数据
	int gid = GetGid(entity.x(), entity.y());
	Leave(entity.id(), gid);
	LeaveBroadcast(entity, entity.x(), entity.y());
}

void Scene::Leave(const uint64_t id, const int gid)
{
	auto it = grids_.find(gid);
	if (it != grids_.end())
	{
		auto find_it = it->second.find(id);
		if (find_it != it->second.end())
		{
			it->second.erase(id);
		}
	}
}

//正常情况下，实体的位置信息自己更新和维护，场景不负责更新实体的坐标
void Scene::Move(const Entity& entity, const int x, const int y)
{
	//先向所在区域发送移动消息
	MoveBroadcast(entity, x, y);
	//移动的时候要区分是否跨格子，如果没有跨格子，只需要向视野区域广播移动包就可以。
	//如果跨格子的话，需要向离开的区域广播离开消息，同时向进入的区域广播进入区域，同时在这三个区域都广播移动消息。
	//通常实现的时候，离开消息可以省略掉，因为实体移动走出了视野的话，客户端自己负责隐藏（销毁）实体对象。
	int old_gid = GetGid(entity.x(), entity.y());
	int new_gid = GetGid(x, y);
	if (old_gid == new_gid)
	{
		//printf("没有跨格子\n");
		return;
	}
	else
	{
		//移动后，离开了原来的大格子，视野变化
		int gx1 = entity.x() / GX;
		int gy1 = entity.y() / GY;
		int gx2 = x / GX;
		int gy2 = y / GY;
		
		Scene::DIR dir = GetDir(entity.x(), entity.y(), x, y);
		//根据不同的方向分别进行移动
		switch (dir)
		{
		case TT::Scene::DIR_UP:
			move_up(entity, x, y);
			break;
		case TT::Scene::DIR_DOWN:
			move_down(entity, x, y);
			break;
		case TT::Scene::DIR_LEFT:
			move_left(entity, x, y);
			break;
		case TT::Scene::DIR_RIGHT:
			move_right(entity, x, y);
			break;
		case TT::Scene::DIR_LEFT_UP:
			move_left_up(entity, x, y);
			break;
		case TT::Scene::DIR_RIGHT_UP:
			move_right_up(entity, x, y);
			break;
		case TT::Scene::DIR_LEFT_DOWN:
			move_left_down(entity, x, y);
			break;
		case TT::Scene::DIR_RIGHT_DOWN:
			move_right_down(entity, x, y);
			break;
		default:
			std::cerr << "错误的移动目的地" << std::endl;
			break;
		}
	}
}

//通知离开
void SendLeave(const Entity& entity, const std::unordered_set<uint64_t>& ids)
{
	for (auto id : ids)
	{
		//TODO 这里基本都是封一个离开包，发送给对应id的实体，或者直接发往id对应的网络连接
		std::cout << "notity: " << id << " " << entity.id() << " leave." << std::endl;
	}
}
//通知进入
void SendEnter(const Entity& entity, const std::unordered_set<uint64_t>& ids)
{
	for (auto id : ids)
	{
		//TODO 这里基本都是将entity打包然后发送给对应id的实体，或者直接发往id对应的网络连接
		//std::cout << "notity: " << id << " " << entity.id() << " enter." << std::endl;
	}
}
void Scene::EnterBroadcast(const Entity& entity, const int x, const int y)
{
	std::unordered_set<uint64_t> entities;
	ViewEntities(entities, x, y);
	//向视野内的实体通告entity进入
	SendEnter(entity, entities);
}

void Scene::LeaveBroadcast(const Entity& entity, const int x, const int y)
{
	std::unordered_set<uint64_t> entities;
	ViewEntities(entities, entity.x(), entity.y());
	//向视野内的实体通告entity离开
	SendLeave(entity, entities);
}

void Scene::MoveBroadcast(const Entity& entity, const int x, const int y)
{
	std::unordered_set<uint64_t> entities;
	ViewEntities(entities, x, y);
	//printf("通知移动消息的对象数量:%d\n", entities.size());
	//向视野内的实体通告entity进入
	for (auto id : entities)
	{
		//TODO 发送移动消息
		std::cout << "notity: " << id 
			<< " " << entity.id() 
			<< " move to " << x << " " << y 
			<< std::endl;
	}
}

int Scene::GetGid(const int x, const int y) const
{
	
	int gx = x / GX;
	int gy = y / GY;
	return GxGy2Gid(gx, gy);
}

int Scene::GxGy2Gid(const int gx, const int gy) const
{
	int gx1 = gx < 0 ? 0 : gx;
	int gy1 = gy < 0 ? 0 : gy;
	return gx1 * FACTOR + gy1;
}

//视野内的实体
void Scene::ViewEntities(std::unordered_set<uint64_t>& entities, const int x, const int y)
{
	std::unordered_set<int> view_grids;
	ViewGrids(view_grids, x, y);
	for (auto gid : view_grids)
	{
		auto grid_it = grids_.find(gid);
		if (grid_it != grids_.end())
		{
			for (auto grid_entity_id : grid_it->second)
			{
				entities.insert(grid_entity_id);
			}
		}
	}
}

//视野内的9宫格
//需要注意的是，其中格子因为加1之后可能已经非法的位置了，所以在使用结果的时候需要进行基本的判断
void Scene::ViewGrids(std::unordered_set<int>& view_grids, const int x, const int y)
{
	int gx = x / GX;
	int gy = y / GY;

	//这9个格子就是视野
	view_grids.insert({ 
		GxGy2Gid(gx, gy + 1)      //正上方
		,GxGy2Gid(gx - 1, gy + 1) //左上方
		,GxGy2Gid(gx + 1, gy + 1) //右上方
		,GxGy2Gid(gx, gy)         //中间
		,GxGy2Gid(gx - 1, gy)     //左边
		,GxGy2Gid(gx + 1, gy)     //右边
		,GxGy2Gid(gx, gy - 1)     //正下方
		,GxGy2Gid(gx - 1, gy - 1) //左下方
		,GxGy2Gid(gx + 1, gy - 1) //右下方
	});
}

void Scene::GridEntities(std::unordered_set<uint64_t>& entities, int gid)
{
	auto grid_it = grids_.find(gid);
	if (grid_it != grids_.end())
	{
		for (auto grid_entity_id : grid_it->second)
		{
			entities.insert(grid_entity_id);
		}
	}
}

//由{x1,y1}向{x2,y2}移动
Scene::DIR Scene::GetDir(const int x1, const int y1, const int x2, const int y2)
{
	//垂直方向移动
	if (x1 == x2)
	{
		if (y2 > y1)
			return Scene::DIR::DIR_UP;
		else
			return Scene::DIR::DIR_DOWN;
	}

	//水平移动
	if (y1 == y2)
	{
		if (x2 > x1)
			return Scene::DIR::DIR_RIGHT;
		else
			return Scene::DIR::DIR_LEFT;
	}

	//其他4个方向
	if (x2 > x1 && y2 > y1)
		return DIR_RIGHT_UP;
	if (x2 > x1 && y2 < y1)
		return Scene::DIR::DIR_RIGHT_DOWN;
	if (x1 > x2 && y2 > y1)
		return Scene::DIR::DIR_LEFT_UP;
	if (x1 > x2 && y2 < y1)
		return Scene::DIR::DIR_LEFT_DOWN;

	return Scene::DIR::DIR_NULL;
}

//进行相应的消息通知
void Scene::Notity(const Entity& entity,
	const std::unordered_set<int>& leave_grids, 
	const std::unordered_set<int>& enter_grids)
{
	std::unordered_set<uint64_t> leave_entities, enter_entities;
	for (auto it : leave_grids)
	{
		//Leave(entity.id(), it);
		GridEntities(leave_entities, it); //获取需要通知实体离开消息的其他实体
	}
	for (auto it : enter_grids)
	{
		//Enter(entity.id(), it);
		GridEntities(enter_entities, it); //获取需要通知实体进入消息的其他实体
	}
	SendLeave(entity, leave_entities);
	SendEnter(entity, enter_entities);
}

//向上移动，下面那3个格子会离开视野，上面会有3个新的格子进入视野
//下面几个方法都类似，需要注意的点和ViewGrids方法一样，在使用结果前进行基本的合法性判断
void Scene::move_up(const Entity& entity, int x, int y)
{
	//进入和离开的大格子
	std::unordered_set<int> leave_grids, enter_grids;
	int gx1 = entity.x() / GX;
	int gy1 = entity.y() / GY;
	int gx2 = x / GX;
	int gy2 = y / GY;
	leave_grids.insert({
		GxGy2Gid(gx1, gy1-1),    //下方
		GxGy2Gid(gx1-1,gy1-1), //左下方
		GxGy2Gid(gx1+1,gy1-1)  //右下方
	});
	enter_grids.insert({
		GxGy2Gid(gx2, gy2+1),    //上方
		GxGy2Gid(gx2 - 1,gy2 + 1), //左上方
		GxGy2Gid(gx2 + 1,gy2 + 1)  //右上方
	});
	Leave(entity.id(), GxGy2Gid(gx1, gy1));
	Enter(entity.id(), GxGy2Gid(gx2, gy2));
	Notity(entity, leave_grids, enter_grids);
}
void Scene::move_down(const Entity& entity, int x, int y)
{
	std::unordered_set<int> leave_grids, enter_grids;
	int gx1 = entity.x() / GX;
	int gy1 = entity.y() / GY;
	int gx2 = x / GX;
	int gy2 = y / GY;
	leave_grids.insert({
		GxGy2Gid(gx1, gy1 + 1),    //上方
		GxGy2Gid(gx1 - 1,gy1 + 1), //左上方
		GxGy2Gid(gx1 + 1,gy1 + 1)  //右上方
	});
	enter_grids.insert({
		GxGy2Gid(gx2, gy2 - 1),    //下方
		GxGy2Gid(gx2 - 1,gy2 - 1), //左下方
		GxGy2Gid(gx2 + 1,gy2 - 1)  //右下方
	});
	Leave(entity.id(), GxGy2Gid(gx1, gy1));
	Enter(entity.id(), GxGy2Gid(gx2, gy2));
	Notity(entity, leave_grids, enter_grids);
}
void Scene::move_left(const Entity& entity, int x, int y)
{
	std::unordered_set<int> leave_grids, enter_grids;
	int gx1 = entity.x() / GX;
	int gy1 = entity.y() / GY;
	int gx2 = x / GX;
	int gy2 = y / GY;
	leave_grids.insert({
		GxGy2Gid(gx1, gy1 + 1),    //右方
		GxGy2Gid(gx1 + 1,gy1 + 1), //右上方
		GxGy2Gid(gx1 + 1,gy1 - 1)  //右下方
		
	});
	enter_grids.insert({
		GxGy2Gid(gx2 - 1,gy2 + 1), //左上方
		GxGy2Gid(gx2 - 1,gy2 - 1), //左下方
		GxGy2Gid(gx2 - 1,gy1), //左方
	});
	Leave(entity.id(), GxGy2Gid(gx1, gy1));
	Enter(entity.id(), GxGy2Gid(gx2, gy2));
	Notity(entity, leave_grids, enter_grids);
}
void Scene::move_right(const Entity& entity, int x, int y)
{
	std::unordered_set<int> leave_grids, enter_grids;
	int gx1 = entity.x() / GX;
	int gy1 = entity.y() / GY;
	int gx2 = x / GX;
	int gy2 = y / GY;
	leave_grids.insert({
		GxGy2Gid(gx1 - 1,gy1 + 1), //左上方
		GxGy2Gid(gx1 - 1,gy1 - 1), //左下方
		GxGy2Gid(gx1 - 1,gy1), //左方
	});
	enter_grids.insert({
		GxGy2Gid(gx2, gy2 + 1),    //右方
		GxGy2Gid(gx2 + 1,gy2 + 1), //右上方
		GxGy2Gid(gx2 + 1,gy2 - 1)  //右下方
	});
	Leave(entity.id(), GxGy2Gid(gx1, gy1));
	Enter(entity.id(), GxGy2Gid(gx2, gy2));
	Notity(entity, leave_grids, enter_grids);
}
void Scene::move_left_up(const Entity& entity, int x, int y)
{
	std::unordered_set<int> leave_grids, enter_grids;
	int gx1 = entity.x() / GX;
	int gy1 = entity.y() / GY;
	int gx2 = x / GX;
	int gy2 = y / GY;
	leave_grids.insert({
		GxGy2Gid(gx1,gy1 - 1),    //下方
		GxGy2Gid(gx1 - 1,gy1 - 1),    //左下方
		GxGy2Gid(gx1 + 1,gy1 + 1), //右上方
		GxGy2Gid(gx1 + 1,gy1 - 1),  //右下方
		GxGy2Gid(gx1 + 1,gy1) //右边
	});
	enter_grids.insert({
		GxGy2Gid(gx2 - 1,gy2 + 1), //左上方
		GxGy2Gid(gx2 - 1,gy2 - 1), //左下方
		GxGy2Gid(gx2 - 1,gy2), //左方
		GxGy2Gid(gx2 + 1,gy2 + 1), //右上
		GxGy2Gid(gx2,gy2 + 1) //上
	});
	Leave(entity.id(), GxGy2Gid(gx1, gy1));
	Enter(entity.id(), GxGy2Gid(gx2, gy2));
	Notity(entity, leave_grids, enter_grids);
}
void Scene::move_right_up(const Entity& entity, int x, int y)
{
	std::unordered_set<int> leave_grids, enter_grids;
	int gx1 = entity.x() / GX;
	int gy1 = entity.y() / GY;
	int gx2 = x / GX;
	int gy2 = y / GY;
	leave_grids.insert({
		GxGy2Gid(gx1,gy1 - 1),    //下方
		GxGy2Gid(gx1 + 1,gy1 - 1),  //右下方
		GxGy2Gid(gx1 - 1,gy1 - 1),    //左下方
		GxGy2Gid(gx1 - 1,gy1 + 1), //左上方
		GxGy2Gid(gx1 - 1,gy1) //左边
	});
	enter_grids.insert({
		GxGy2Gid(gx2 - 1,gy2 + 1), //左上方
		GxGy2Gid(gx2,gy2 + 1), //上
		GxGy2Gid(gx2 + 1,gy2 + 1), //右上
		GxGy2Gid(gx2 + 1,gy2), //右方
		GxGy2Gid(gx2 + 1,gy2 - 1) //右下方
	});
	//printf("gx1 %d gy1 %d gx2 %d gy2 %d\n", gx1, gy1, gx2, gy2);
	Leave(entity.id(), GxGy2Gid(gx1, gy1));
	Enter(entity.id(), GxGy2Gid(gx2, gy2));
	Notity(entity, leave_grids, enter_grids);
}
void Scene::move_left_down(const Entity& entity, int x, int y)
{
	std::unordered_set<int> leave_grids, enter_grids;
	int gx1 = entity.x() / GX;
	int gy1 = entity.y() / GY;
	int gx2 = x / GX;
	int gy2 = y / GY;
	leave_grids.insert({
		GxGy2Gid(gx1 - 1,gy1 + 1), //左上方
		GxGy2Gid(gx1,gy1 + 1), //上
		GxGy2Gid(gx1 + 1,gy1 + 1), //右上
		GxGy2Gid(gx1 + 1,gy1), //右方
		GxGy2Gid(gx1 + 1,gy1 - 1) //右下方
	});
	enter_grids.insert({
		GxGy2Gid(gx2,gy2 - 1),    //下方
		GxGy2Gid(gx2 + 1,gy2 - 1),  //右下方
		GxGy2Gid(gx2 - 1,gy2 - 1),    //左下方
		GxGy2Gid(gx2 - 1,gy2 + 1), //左上方
		GxGy2Gid(gx2 - 1,gy2) //左边
	});
	Leave(entity.id(), GxGy2Gid(gx1, gy1));
	Enter(entity.id(), GxGy2Gid(gx2, gy2));
	Notity(entity, leave_grids, enter_grids);
}
void Scene::move_right_down(const Entity& entity, int x, int y)
{
	std::unordered_set<int> leave_grids, enter_grids;
	int gx1 = entity.x() / GX;
	int gy1 = entity.y() / GY;
	int gx2 = x / GX;
	int gy2 = y / GY;
	leave_grids.insert({
		GxGy2Gid(gx1 - 1,gy1 + 1), //左上方
		GxGy2Gid(gx1 - 1,gy1 - 1), //左下方
		GxGy2Gid(gx1 - 1,gy1), //左方
		GxGy2Gid(gx1 + 1,gy1 + 1), //右上
		GxGy2Gid(gx1,gy1 + 1) //上
	});
	enter_grids.insert({
		GxGy2Gid(gx2,gy2 - 1),    //下方
		GxGy2Gid(gx2 - 1,gy2 - 1),    //左下方
		GxGy2Gid(gx2 + 1,gy2 + 1), //右上方
		GxGy2Gid(gx2 + 1,gy2 - 1),  //右下方
		GxGy2Gid(gx2 + 1,gy2) //右边
	});
	Leave(entity.id(), GxGy2Gid(gx1, gy1));
	Enter(entity.id(), GxGy2Gid(gx2, gy2));
	Notity(entity, leave_grids, enter_grids);
}