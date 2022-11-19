#pragma once

#include <queue>
#include "Box.h"
#include <memory>
#include <vector>
#include <algorithm>

class PostStep
{
public:
	void Evaluate(std::vector<std::unique_ptr<Box>>& boxptrs)
	{
		while (!toBeDestroyed.empty())
		{
			const auto pos = std::find_if(boxptrs.begin(), boxptrs.end(),
				[this](const std::unique_ptr<Box>& b) {
					return b.get() == toBeDestroyed.front();
				});
			if (pos != boxptrs.end())
			{
				delete pos->release();
				boxptrs.erase( std::remove(boxptrs.begin(), boxptrs.end(), *pos) );
			}
			toBeDestroyed.pop();
		}
	}
	void DestroyBox(Box* box)
	{
		toBeDestroyed.push(box);
	}
private:
	std::queue<Box*> toBeDestroyed;
};