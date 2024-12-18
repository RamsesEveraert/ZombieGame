/*=============================================================================*/
// Copyright 2020-2021 Elite Engine
// Authors: Andries Geens
/*=============================================================================*/
// EDecisionMaking.h: General Decision making header that includes all Decision making structures and blackboard data
/*=============================================================================*/
#ifndef ELITE_DECISION_MAKING
#define ELITE_DECISION_MAKING

/* --- Data --- */
//General Interface
namespace Elite
{
	class IDecisionMaking
	{
	public:
		IDecisionMaking() = default;
		virtual ~IDecisionMaking() = default;

		virtual void Update(float deltaT) = 0;

	};
}

/* --- Data --- */
//Blackboard
#include "../EliteData/EBlackboard.h"

/* --- Decision Making Structures --- */
//BT
#include "EBehaviorTree.h"


#endif




