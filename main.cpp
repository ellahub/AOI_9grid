/*
author: atao 1628025718@qq.com
*/
#include <stdio.h>
#include <iostream>
#include <vector>

#include "Scene.h"
#include "Entity.h"
using namespace TT;
int main()
{
	//100x100的场景，也就是10000个小格子
	//转换成9宫格后，变成200多个大格子
	TT::Scene scene(100, 100);

	//准备10000个实体,坐标分别是(0,0),(0,1),...(99,99)
	std::vector<Entity> e_vec;
	e_vec.reserve(10000);
	for (int x = 0; x < 100; ++x)
	{
		for (int y = 0; y < 100; ++y)
		{
			Entity e(x * 100 + y, x, y);
			e_vec.push_back(e);
		}
	}
	//全部进入场景
	for (auto it : e_vec)
	{
		scene.Enter(it);
	}
	scene.Stat();
	//printf("66th id %I64u x %d y %d\n", e_vec[66].id(), e_vec[66].x(), e_vec[66].y());
	getchar();
	//scene.Leave(e2);
	//测试移动
	for (int i = 1; i < 1000; ++i)
	{
		scene.Move(e_vec[0], i, i);
		getchar();
		e_vec[0].set_x(i);
		e_vec[0].set_y(i);
	}
	getchar();
}