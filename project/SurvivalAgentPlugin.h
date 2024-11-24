#pragma once
#include "IExamPlugin.h"
#include "Exam_HelperStructs.h"

#include "EliteBehaviorTree/EDecisionMaking.h"
#include "Steeringbehaviors/SteeringBehaviors.h"
#include "Steeringbehaviors/CombinedSteeringBehaviors.h"
#include "EliteData/EBlackboard.h"

// eigen classes
#include "Grid.h"
#include "HouseManager.h"
#include "InventoryManager.h"
#include "EntityManager.h"

#include <memory>

class IBaseInterface;
class IExamInterface;

class SurvivalAgentPlugin :public IExamPlugin
{
public:
	SurvivalAgentPlugin() {};
	virtual ~SurvivalAgentPlugin() {};

	void Initialize(IBaseInterface* pInterface, PluginInfo& info) override;
	void DllInit() override;
	void DllShutdown() override;

	void InitGameDebugParams(GameDebugParams& params) override;
	void Update_Debug(float dt) override;

	SteeringPlugin_Output UpdateSteering(float dt) override;
	void Render(float dt) const override;

private:

	// Main variables
	IExamInterface* m_pInterface = nullptr;

	// Managers
	std::unique_ptr<ZombieGame::Grid> m_pGrid{};
	std::unique_ptr<ZombieGame::HouseManager> m_pHouseManager{};
	std::unique_ptr<ZombieGame::InventoryManager> m_pInventoryManager{};
	std::unique_ptr<ZombieGame::EntityManager> m_pEntityManager{};

	// Blackboard
	std::unique_ptr<Elite::Blackboard> m_pBlackboard{};

	// SteeringBehaviors

	std::unique_ptr<SteeringPlugin_Output_Extended> m_pCurrentSteering;
	std::unique_ptr<ISteeringBehavior> m_pCurrentSteeringBehavior;

		//--- normal steering ---//
	std::unique_ptr<Arrive> m_pArriveBehaviour;
	std::unique_ptr<Seek> m_pSeekBehaviour;
	std::unique_ptr<Face> m_pFaceBehaviour;
	std::unique_ptr<Flee> m_pFleeBehaviour;
	std::unique_ptr<RotateClockWise> m_pRotateClockWiseBehaviour;

		//---- combined steering ----//
	std::unique_ptr<BlendedSteering> m_pSeekAndFaceBehaviour;
	std::unique_ptr<BlendedSteering> m_pFleeAndFaceBehaviour;
	
	// Behavior Tree
	std::unique_ptr<Elite::BehaviorTree> m_pBehaviourTree{};

	// Agent Data
	AgentInfo m_AgentInfo{};
	const float m_TresholdToRun{ 5.f };
	bool m_ShouldRun{ false };
	bool m_CanScan{ true };

	// Inventory
	UINT m_InventorySlot = 0;

	// Targets
	Elite::Vector2 m_Target;

	// House Data
	std::vector<std::unique_ptr<House>> m_StoredHouses{};

	// Timers
	float m_DeltaTime{};
	float m_ExploreTime{};
	float m_AlertedTime{};

private:

	// Blackboard
	void InitializeBlackboard();
	// Behavior Tree
	void CreateBehaviorTree();

};

//ENTRY
//This is the first function that is called by the host program
//The plugin returned by this function is also the plugin used by the host program
extern "C"
{
	__declspec (dllexport) IPluginBase* Register()
	{
		return new SurvivalAgentPlugin();
	}
}