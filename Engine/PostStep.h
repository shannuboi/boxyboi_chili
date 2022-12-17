#pragma once

#include "Box.h"
#include <memory>
#include <vector>
#include <queue>
#include <algorithm>
#include <unordered_map>
#include <functional>
#include <assert.h>

typedef std::pair<Box*, Box*> Collision;
typedef std::pair<Color, Color> CollisionType;

namespace std
{
	template <>
	struct hash<CollisionType>
	{
		size_t operator()(const CollisionType& collType) const
		{
			std::hash<unsigned int> hasher;
			auto hashval = hasher(collType.first.dword);
			hashval ^= hasher(collType.second.dword) + 0x9e3779b9 + (hashval << 6) + (hashval >> 2);

			return hashval;
		}
	};
}

class PostStep
{
public:
	PostStep(std::vector<std::unique_ptr<Box>>& in_boxptrs)
		:
		boxptrs(in_boxptrs)
	{}
	// SetUp() should be called after boxPtrs vector is initialized
	void SetUp()
	{
		std::vector<Color> usedColors;
		for (const auto& bp : boxptrs)
		{
			Color curC = bp->GetColorTrait().GetColor();
			if (std::all_of(usedColors.begin(), usedColors.end(),
				[curC](const Color& usedBoxC) {
					return curC != usedBoxC;
				}))
			{
				usedColors.push_back(curC);
			}
		}

		for (auto C1 = usedColors.begin(); C1 != usedColors.end(); C1++)
		{
			for (auto C2 = usedColors.begin(); C2 != usedColors.end(); C2++)
			{
				CollisionType collType = { *C1, *C2 };

				CollisionResolver[collType] = GetResolverFunction(collType);
			}
		}
	}
	void AddCollision(Box* b1, Box* b2)
	{
		collisions.push({ b1, b2 });
	}
	void Evaluate()
	{
		while (!collisions.empty())
		{
			Box* b1 = collisions.front().first;
			Box* b2 = collisions.front().second;

			const bool b1Destroyed = std::all_of(boxptrs.begin(), boxptrs.end(),
				[this, b1](const std::unique_ptr<Box>& b) {
					return b.get() != b1;
				});
			const bool b2Destroyed = std::all_of(boxptrs.begin(), boxptrs.end(),
				[this, b2](const std::unique_ptr<Box>& b) {
					return b.get() != b2;
				});

			if (!b1Destroyed && !b2Destroyed)
			{
				Collision coll = { b1, b2 };
				CollisionType collType = { b1->GetColorTrait().GetColor(), b2->GetColorTrait().GetColor() };
				CollisionResolver[collType](coll);
			}

			collisions.pop();
		}
	}
private:
	std::function<void(Collision)> GetResolverFunction(const CollisionType& collType)
	{
		if (collType.first == collType.second)
		{
			return [this](Collision& coll) {
				DeleteBox(coll.first);
				DeleteBox(coll.second);
			};
		}
		else
		{
			return [](Collision& coll) {};
		}
	}
	void DeleteBox(Box* pbox)
	{
		const auto pos = std::find_if(boxptrs.begin(), boxptrs.end(),
			[this, pbox](const std::unique_ptr<Box>& b) {
				return b.get() == pbox;
			});
		assert(pos != boxptrs.end());
		delete pos->release();
		boxptrs.erase(std::remove(boxptrs.begin(), boxptrs.end(), *pos));
	}
private:
	std::vector<std::unique_ptr<Box>>& boxptrs;
	std::queue<Collision> collisions;
	std::unordered_map< CollisionType, std::function<void(Collision)> > CollisionResolver;
};