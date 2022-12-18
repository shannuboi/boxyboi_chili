#pragma once

#include "Box.h"
#include "Vec2.h"
#include "Mat2.h"
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
	PostStep(b2World& world, std::vector<std::unique_ptr<Box>>& in_boxptrs)
		:
		world(world),
		boxptrs(in_boxptrs)
	{}
	// SetUp() should be called after boxPtrs vector is initialized
	void SetUp()
	{
		std::vector<Color> usedColors = { Colors::Red, Colors::Blue, Colors::White, Colors::Yellow, Colors::Green };

		for (auto C1 = usedColors.begin(); C1 != usedColors.end(); C1++)
		{
			for (auto C2 = usedColors.begin(); C2 != usedColors.end(); C2++)
			{
				CollisionType collType = { *C1, *C2 };
				std::function<void(Collision)> func;				
				if (collType.first == Colors::Red && collType.second == Colors::Red)
				{
					func = [this](Collision& coll) {
						DeleteBox(coll.first);
						DeleteBox(coll.second);
					};
				}
				else if(collType.first == collType.second)
				{
					func = [](Collision& coll) {

					};
				}
				else if (collType.first == Colors::White && collType.second == Colors::Blue)
				{
					func = [this](Collision& coll) {
						coll.second->SetColorTrait(coll.first->GetColorTrait().Clone());
					};
				}
				else if (collType.second == Colors::White && collType.first == Colors::Blue)
				{
					func = [this](Collision& coll) {
						coll.first->SetColorTrait(coll.second->GetColorTrait().Clone());
					};
				}
				else if (collType.first == Colors::Red)
				{
					func = [this](Collision& coll) {
						DeleteBox(coll.second);
					};
				}
				else if (collType.second == Colors::Red)
				{
					func = [this](Collision& coll) {
						DeleteBox(coll.first);
					};
				}
				else if (collType.first == Colors::Blue)
				{
					func = [this](Collision& coll) {
						SplitBox(coll.second);
					};
				}
				else if (collType.second == Colors::Blue)
				{
					func = [this](Collision& coll) {
						SplitBox(coll.first);
					};
				}
				else
				{
					func = [](Collision& coll) {

					};
				}
				CollisionResolver[collType] = func;
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
		if (collType.first == Colors::Red && collType.second == Colors::Red)
		{
			return [this](Collision& coll) {
				DeleteBox(coll.first);
				DeleteBox(coll.second);
			};
		}
		else if (collType.first == Colors::White && collType.second == Colors::Red)
		{
			return [this](Collision& coll) {
				coll.second->SetColorTrait(coll.first->GetColorTrait().Clone());
			};
		}
		else if (collType.first == Colors::White && collType.second == Colors::Red)
		{
			return [this](Collision& coll) {
				coll.second->SetColorTrait(coll.first->GetColorTrait().Clone());
			};
		}
		else if (collType.first == Colors::Blue)
		{
			return [this](Collision& coll) {
				SplitBox(coll.second);
			};
		}
		else if (collType.second == Colors::Blue)
		{
			return [this](Collision& coll) {
				SplitBox(coll.first);
			};
		}
		else
		{
			return [](Collision& coll) {
				
			};
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
	void SplitBox(Box* pbox)
	{
		if (boxptrs.capacity() < boxptrs.size() + 4)
		{
			boxptrs.reserve(boxptrs.capacity() + 32);
		}

		const auto BoxIter = std::find_if(boxptrs.begin(), boxptrs.end(),
			[this, pbox](const std::unique_ptr<Box>& b) {
				return b.get() == pbox;
			});
		assert(BoxIter != boxptrs.end());

		const float size = pbox->GetSize() / 2.0f;
		const float angle = pbox->GetAngle();
		const Vec2 vel = pbox->GetVelocity();
		const float angVel = pbox->GetAngularVelocity();

		if (size < 0.1f) return;

		const Vec2 deltaPos[4] = { {0.25f, -0.25f}, {0.25f, 0.25f}, {-0.25f, 0.25f}, {-0.25f, -0.25f} };
		for (int i = 0; i < 4; i++)
		{
			const Vec2 pos = pbox->GetPosition() + deltaPos[i] * Mat2::Rotation(angle);
			boxptrs.push_back(std::make_unique<Box>(pbox->GetColorTrait().Clone(), world,
				pos, size, angle, vel, angVel));
		}

		delete BoxIter->release();
		boxptrs.erase(std::remove(boxptrs.begin(), boxptrs.end(), *BoxIter));
	}
private:
	b2World& world;
	std::vector<std::unique_ptr<Box>>& boxptrs;
	std::queue<Collision> collisions;
	std::unordered_map< CollisionType, std::function<void(Collision)> > CollisionResolver;
};