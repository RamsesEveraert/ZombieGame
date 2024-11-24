#include "stdafx.h"
#include "CombinedSteeringBehaviors.h"
#include <algorithm>
#include "Exam_HelperStructs.h"

BlendedSteering::BlendedSteering(std::vector<WeightedBehavior> weightedBehaviors)
	:m_WeightedBehaviors(weightedBehaviors)
{
};

//****************
//BLENDED STEERING

SteeringPlugin_Output_Extended BlendedSteering::CalculateSteering(float deltaT, AgentInfo& pAgent)
{
	SteeringPlugin_Output_Extended blendedSteering{};
	float totalWeight{};

	for (auto& weightedBehavior : m_WeightedBehaviors)
	{
		if (weightedBehavior.weight <= 0.f) continue;			

		auto steering = weightedBehavior.pBehavior->CalculateSteering(deltaT, pAgent);
		blendedSteering.LinearVelocity += steering.LinearVelocity * weightedBehavior.weight;
		blendedSteering.AngularVelocity += steering.AngularVelocity * weightedBehavior.weight;

		totalWeight += weightedBehavior.weight;
	}

	if (totalWeight > 0.f)
	{
		float invScale = 1 / totalWeight;

		blendedSteering.LinearVelocity *= invScale;
		blendedSteering.AngularVelocity *= invScale;
	}

	return blendedSteering;
}

//*****************
//PRIORITY STEERING
SteeringPlugin_Output_Extended PrioritySteering::CalculateSteering(float deltaT, AgentInfo& pAgent)
{
	SteeringPlugin_Output_Extended steering = {};

	for (auto pBehavior : m_PriorityBehaviors)
	{
		steering = pBehavior->CalculateSteering(deltaT, pAgent);

		if (steering.IsValid)
			break;
	}

	//If non of the behavior return a valid output, last behavior is returned
	return steering;
}