/*=============================================================================*/
// Copyright 2023-2024 Elite Engine
// Authors: Matthieu Delaere, Thomas Goussaert
/*=============================================================================*/
// SteeringBehaviors.h: SteeringBehaviors interface and different implementations
/*=============================================================================*/
#pragma once

//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
#include "SteeringHelpers.h"
#include "Exam_HelperStructs.h"

class SteeringAgent;
class Obstacle;

#pragma region **ISTEERINGBEHAVIOR** (BASE)
class ISteeringBehavior
{
public:
	ISteeringBehavior() = default;
	virtual ~ISteeringBehavior() = default;

	virtual SteeringPlugin_Output_Extended CalculateSteering(float deltaT, AgentInfo& pAgent) = 0;

	//Seek Functions
	void SetTarget(const TargetData& target) { m_Target = target; }

	template<class T, typename std::enable_if<std::is_base_of<ISteeringBehavior, T>::value>::type* = nullptr>
	T* As()
	{ return static_cast<T*>(this); }

protected:
	TargetData m_Target;
};
#pragma endregion


#pragma region SEEK
class Seek : public ISteeringBehavior
{
public:
	Seek() = default;
	virtual ~Seek() = default;

	SteeringPlugin_Output_Extended CalculateSteering(float deltaT, AgentInfo& pAgent) override;
};
#pragma endregion
#pragma region FLEE
class Flee : public ISteeringBehavior
{
public:
	Flee() = default;
	virtual ~Flee() = default;

	SteeringPlugin_Output_Extended CalculateSteering(float deltaT, AgentInfo& pAgent) override;
};
#pragma endregion
#pragma region ARRIVE
class Arrive final : public ISteeringBehavior
{
public:
	Arrive() = default;
	virtual ~Arrive() = default;

	void SetTargetRadius(float radius);
	void SetSlowRadius(float radius);
	SteeringPlugin_Output_Extended CalculateSteering(float deltaT, AgentInfo& pAgent) override;
private:
	float m_TargetRadius = 3.f;
	float m_SlowRadius = 10.f;
};
#pragma endregion
#pragma region FACE
class Face final: public ISteeringBehavior
{
public:
	Face() = default;
	virtual ~Face() = default;

	SteeringPlugin_Output_Extended CalculateSteering(float deltaT, AgentInfo& pAgent) override;
private:
};
#pragma endregion
#pragma region PURSUIT
class Pursuit final : public ISteeringBehavior
{
public:
	Pursuit() = default;
	virtual ~Pursuit() = default;

	SteeringPlugin_Output_Extended CalculateSteering(float deltaT, AgentInfo& pAgent) override;
private:
};
#pragma endregion
#pragma region EVADE
class Evade final : public ISteeringBehavior
{
public:
	Evade(float evadeRadius = 10.f) : m_EvadeRadius{ evadeRadius } {};
	virtual ~Evade() = default;

	SteeringPlugin_Output_Extended CalculateSteering(float deltaT, AgentInfo& pAgent) override;
private:
	float m_EvadeRadius;
};
#pragma endregion
#pragma region WANDER
class Wander final : public Seek
{
public:
	Wander() = default;
	virtual ~Wander() = default;

	SteeringPlugin_Output_Extended CalculateSteering(float deltaT, AgentInfo& pAgent) override;

	void SetWanderOffset(float offset) { m_OffsetDistance = offset; }
	void SetWanderRadius(float radius) { m_Radius = radius; }
	void SetMaxAngleChange(float rad) { m_MaxAngleChange = rad; }

protected:
	float m_OffsetDistance{ 7.f };
	float m_Radius{ 5.f }; 
	float m_MaxAngleChange{ Elite::ToRadians(90.f) };
	float m_WanderAngle{ Elite::ToRadians(0.f) };
};
#pragma endregion
#pragma region RotateClockWise
class RotateClockWise : public ISteeringBehavior
{
public:
	RotateClockWise() = default;
	virtual ~RotateClockWise() = default;

	//Rotate Clock Wise Behaviour
	SteeringPlugin_Output_Extended CalculateSteering(float deltaT, AgentInfo& agentInfo) override;
};
#pragma endregion