//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "SteeringBehaviors.h"

#include "EliteMath\EMatrix2x3.h"
#include <limits>

#pragma region SEEK
SteeringPlugin_Output_Extended Seek::CalculateSteering(float deltaT, AgentInfo& pAgent)
{
	SteeringPlugin_Output_Extended steering = {};

	steering.LinearVelocity = m_Target.Position - pAgent.Position;
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent.MaxLinearSpeed;

	return steering;
}
#pragma endregion
#pragma region FLEE
SteeringPlugin_Output_Extended Flee::CalculateSteering(float deltaT, AgentInfo& pAgent)
{
	SteeringPlugin_Output_Extended steering = {};

	steering.LinearVelocity = pAgent.Position - m_Target.Position;
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent.MaxLinearSpeed;

	return steering;
}
#pragma endregion
#pragma region ARRIVE
void Arrive::SetTargetRadius(float radius)
{
	m_TargetRadius = radius;
}

void Arrive::SetSlowRadius(float radius)
{
	m_SlowRadius = radius;
}

SteeringPlugin_Output_Extended Arrive::CalculateSteering(float deltaT, AgentInfo& pAgent)
{
	SteeringPlugin_Output_Extended steering{};

	Elite::Vector2 toTarget{ m_Target.Position - pAgent.Position };
	float distance{ toTarget.Magnitude() }; 

	// Calculate values for blending the steering based on distance
	float distanceToSlowDown{ distance - m_TargetRadius };
	float distanceToStop{ m_SlowRadius - m_TargetRadius };

	// Calculate the ratio of how close the agent is to the target, clamped between [0, 1]
	float slowingRatio = distanceToSlowDown / distanceToStop;
	slowingRatio = Elite::Clamp(slowingRatio, 0.f, 1.f);

	// Calculate the desired linear velocity towards the target based on the ratio
	steering.LinearVelocity = toTarget.GetNormalized() * pAgent.MaxLinearSpeed * slowingRatio;

	// Debug rendering
	/*if (pAgent->GetDebugRenderingEnabled())
	{
		DEBUGRENDERER2D->DrawCircle(positionAgent, m_TargetRadius, Elite::Color{ 1.f,0.f,0.f }, DEBUGRENDERER2D->NextDepthSlice());
		DEBUGRENDERER2D->DrawCircle(positionAgent, m_SlowRadius, Elite::Color{ 0.f,0.f,1.f }, DEBUGRENDERER2D->NextDepthSlice());
	}*/

	return steering;
}
#pragma endregion
#pragma region FACE
SteeringPlugin_Output_Extended Face::CalculateSteering(float deltaT, AgentInfo& pAgent)
{

	SteeringPlugin_Output_Extended steering {};

	// Calculate the desired orientation to face the target
	const Elite::Vector2 desiredOrientation{ m_Target.Position - pAgent.Position };
	const Elite::Vector2 currentOrientation{ Elite::OrientationToVector(pAgent.Orientation) };

	// Calculate the angle between current orientation and desired orientation
	const float angle{ Elite::AngleBetween(desiredOrientation,currentOrientation) };

	const float slowAngle{ 0.2f };
	const float stopAngle{ 0.07f };

	if (abs(angle) < slowAngle)
	{
		if (abs(angle) <= stopAngle)
		{
			steering.IsValid = false;
			steering.AngularVelocity = 0.f;
		}
		else
		{
			steering.AngularVelocity = (angle < 0 ? -1 : 1) * pAgent.MaxAngularSpeed * abs(angle) / slowAngle;
		}
	}
	else
	{
		steering.AngularVelocity = (angle < 0 ? -1 : 1) * pAgent.MaxAngularSpeed;
	}
	steering.AutoOrient = false;
	return steering;

}
#pragma endregion
#pragma region PURSUIT
SteeringPlugin_Output_Extended Pursuit::CalculateSteering(float deltaT, AgentInfo& pAgent)
{
	SteeringPlugin_Output_Extended steering{};

	Elite::Vector2 targetDirection = m_Target.Position - pAgent.Position;
	float distanceToTarget = targetDirection.Magnitude();

	float predictionTime = distanceToTarget / pAgent.MaxLinearSpeed;
	Elite::Vector2 predictedTargetPosition = m_Target.Position + m_Target.LinearVelocity * predictionTime;

	// Calculate the desired direction and velocity
	Elite::Vector2 desiredDirection = predictedTargetPosition - pAgent.Position;
	desiredDirection.Normalize();
	steering.LinearVelocity = desiredDirection * pAgent.MaxLinearSpeed;

	// Debug rendering
	/*if (pAgent->GetDebugRenderingEnabled())
	{
		DEBUGRENDERER2D->DrawCircle(predictedTargetPosition, 0.5f, Elite::Color{ 0.f, 1.f, 0.f }, DEBUGRENDERER2D->NextDepthSlice());DEBUGRENDERER2D->DrawSegment(pAgent->GetPosition() + steering.LinearVelocity, pAgent->GetPosition() + steering.LinearVelocity, Elite::Color{ 1.f, 0.f, 0.f }, DEBUGRENDERER2D->NextDepthSlice());
	}*/

	return steering;

}
#pragma endregion
#pragma region EVADE
SteeringPlugin_Output_Extended Evade::CalculateSteering(float deltaT, AgentInfo& pAgent)
{
	SteeringPlugin_Output_Extended steering{};

	Elite::Vector2 targetDirection = m_Target.Position - pAgent.Position;

	float distanceSquared{ targetDirection.MagnitudeSquared() };

	if (distanceSquared > m_EvadeRadius * m_EvadeRadius)
	{
		steering.IsValid = false;
		return steering;
	}

	float distanceToTarget{ sqrtf(distanceSquared) };

	// Limit the prediction time to avoid overshooting
	float predictionTime{ distanceToTarget / pAgent.MaxLinearSpeed };
	Elite::Vector2 predictedTargetPosition = m_Target.Position + m_Target.LinearVelocity * predictionTime;

	// Check if the agent is within the evade radius
	Elite::Vector2 desiredDirection{};

	// Reverse the desired direction for evasion
	desiredDirection = predictedTargetPosition - pAgent.Position;
	desiredDirection.Normalize();
	steering.LinearVelocity = desiredDirection * -pAgent.MaxLinearSpeed;
	

	// Debug rendering
	//if (pAgent->GetDebugRenderingEnabled())
	//{
	//	DEBUGRENDERER2D->DrawCircle(predictedTargetPosition, 0.5f, Elite::Color{ 0.f, 1.f, 0.f }, DEBUGRENDERER2D->NextDepthSlice());
	//}

	return steering;

}
#pragma endregion
#pragma region WANDER 
SteeringPlugin_Output_Extended Wander::CalculateSteering(float deltaT, AgentInfo& pAgent)
{
	Elite::Vector2 circleOrigin{
		pAgent.Position + Elite::OrientationToVector(pAgent.Orientation) * m_OffsetDistance
	};

	float maxWanderAngle{ m_WanderAngle + m_MaxAngleChange };
	float minWanderAngle{ m_WanderAngle - m_MaxAngleChange };

	m_WanderAngle = Elite::randomFloat(minWanderAngle, maxWanderAngle);

	Elite::Vector2 randomPointOnCircle{
		circleOrigin.x + cosf(m_WanderAngle) * m_Radius,
		circleOrigin.y + sinf(m_WanderAngle) * m_Radius
	};

	m_Target = randomPointOnCircle;

	// Debug rendering
	/*if (pAgent->GetDebugRenderingEnabled())
	{
		DEBUGRENDERER2D->DrawCircle(circleOrigin, m_Radius, Elite::Color{ 0.f, 1.f, 0.f, 0.5f }, DEBUGRENDERER2D->NextDepthSlice());
		DEBUGRENDERER2D->DrawPoint(randomPointOnCircle, 2.f, Elite::Color{ 0.f, 0.f, 1.f, 1.f }, DEBUGRENDERER2D->NextDepthSlice());
	}*/


	return Seek::CalculateSteering(deltaT, pAgent);
}
#pragma endregion
#pragma region RotateClockWise 
SteeringPlugin_Output_Extended RotateClockWise::CalculateSteering(float deltaT, AgentInfo& agentInfo)
{
	SteeringPlugin_Output_Extended steering{};

	steering.AutoOrient = false;
	steering.AngularVelocity = -agentInfo.MaxAngularSpeed;

	return steering;
}
#pragma endregion

