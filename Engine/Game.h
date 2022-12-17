/******************************************************************************************
*	Chili DirectX Framework Version 16.10.01											  *
*	Game.h																				  *
*	Copyright 2016 PlanetChili.net <http://www.planetchili.net>							  *
*																						  *
*	This file is part of The Chili DirectX Framework.									  *
*																						  *
*	The Chili DirectX Framework is free software: you can redistribute it and/or modify	  *
*	it under the terms of the GNU General Public License as published by				  *
*	the Free Software Foundation, either version 3 of the License, or					  *
*	(at your option) any later version.													  *
*																						  *
*	The Chili DirectX Framework is distributed in the hope that it will be useful,		  *
*	but WITHOUT ANY WARRANTY; without even the implied warranty of						  *
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the						  *
*	GNU General Public License for more details.										  *
*																						  *
*	You should have received a copy of the GNU General Public License					  *
*	along with The Chili DirectX Framework.  If not, see <http://www.gnu.org/licenses/>.  *
******************************************************************************************/
#pragma once

#include "Graphics.h"
#include <memory>
#include <vector>
#include "FrameTimer.h"
#include <Box2D\Box2D.h>
#include "Box.h"
#include "Boundaries.h"
#include "Pipeline.h"
#include "SolidEffect.h"
#include <random>
#include "PostStep.h"

class Game
{
public:
	Game( class MainWindow& wnd );
	Game( const Game& ) = delete;
	Game& operator=( const Game& ) = delete;
	void Go();
private:
	void ComposeFrame();
	void UpdateModel();
	/********************************/
	/*  User Functions              */
	/********************************/
private:
	MainWindow& wnd;
	Graphics gfx;
	/********************************/
	/*  User Variables              */
	static constexpr float boundarySize = 10.0f;
	static constexpr float boxSize = 1.0f;
	static constexpr int nBoxes = 9;
	std::mt19937 rng = std::mt19937( std::random_device{}() );
	FrameTimer ft;
	Pipeline<SolidEffect> pepe;
	b2World world;
	Boundaries bounds = Boundaries( world,boundarySize );
	std::vector<std::unique_ptr<Box>> boxPtrs;
	PostStep postSteper = PostStep(boxPtrs);
	/********************************/
};