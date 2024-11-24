#include "stdafx.h"
#include "SurvivalAgentPlugin.h"

#include "IExamInterface.h"
#include "Behaviors.h"

using namespace std;

void SurvivalAgentPlugin::Initialize(IBaseInterface* pInterface, PluginInfo& info)
{
	m_pInterface = static_cast<IExamInterface*>(pInterface);

	// Information for the leaderboards!
	info.BotName = "Err404";
	info.Student_Name = "Ramses Everaert";
	info.Student_Class = "2DAE19";
	info.LB_Password = "TheChampIsHere123!";

	// Steering behaviors
	m_pCurrentSteering = std::make_unique<SteeringPlugin_Output_Extended>();

	m_pSeekBehaviour = std::make_unique<Seek>();
	m_pArriveBehaviour = std::make_unique<Arrive>();
	m_pFleeBehaviour = std::make_unique<Flee>();
	m_pFaceBehaviour = std::make_unique<Face>();

	m_pFleeAndFaceBehaviour = std::make_unique<BlendedSteering>(std::vector<BlendedSteering::WeightedBehavior>
	{
		{m_pFleeBehaviour.get(), 0.5f},
		{ m_pFaceBehaviour.get(), 0.5f }
	});

	m_pSeekAndFaceBehaviour = std::make_unique<BlendedSteering>(std::vector<BlendedSteering::WeightedBehavior>
	{
		{m_pSeekBehaviour.get(), 0.5f},
		{ m_pFaceBehaviour.get(), 0.5f }
	});

	// Blackboard
	m_pBlackboard = std::make_unique<Elite::Blackboard>();
	InitializeBlackboard();

	//BehaviorTree
	CreateBehaviorTree();


}

void SurvivalAgentPlugin::DllInit()
{
	//Called when the plugin is loaded
}


void SurvivalAgentPlugin::DllShutdown()
{
	//Called when the plugin gets unloaded
}

// Only works in DEBUG Mode
void SurvivalAgentPlugin::InitGameDebugParams(GameDebugParams& params)
{
	params.AutoFollowCam = true; //Automatically follow the AI? (Default = true)
	params.RenderUI = true; //Render the IMGUI Panel? (Default = true)
	params.SpawnEnemies = true; //Do you want to spawn enemies? (Default = true)
	params.EnemyCount = 20; //How many enemies? (Default = 20)
	params.GodMode = false; //GodMode > You can't die, can be useful to inspect certain behaviors (Default = false)
	params.LevelFile = "GameLevel.gppl";
	params.AutoGrabClosestItem = true; //A call to Item_Grab(...) returns the closest item that can be grabbed. (EntityInfo argument is ignored)
	params.StartingDifficultyStage = 1;
	params.InfiniteStamina = false;
	params.SpawnDebugPistol = false;
	params.SpawnDebugShotgun = false;
	params.SpawnPurgeZonesOnMiddleClick = true;
	params.PrintDebugMessages = true;
	params.ShowDebugItemNames = true;
	params.Seed = 44; //-1 = don't set seed. Any other number = fixed seed //TIP: use Seed = int(time(nullptr)) for pure randomness // seed 20 lievelings seed
}

//Only Active in DEBUG Mode
void SurvivalAgentPlugin::Update_Debug(float dt)
{
}

SteeringPlugin_Output SurvivalAgentPlugin::UpdateSteering(float dt)
{
	// Update important variables
	m_ShouldRun = false;
	m_DeltaTime = dt;

	// Update Managers
	m_AgentInfo = m_pInterface->Agent_GetInfo();
	m_pHouseManager->Update(dt);
	m_pEntityManager->Update(dt);
	m_pGrid->UpdateCurrentAgentCell(&m_AgentInfo);
	m_pInventoryManager->Update(dt);
	
	//Behaviours
	m_pBehaviourTree->Update(dt);

	//Calculate pSteering
	ISteeringBehavior* pCurrentSteeringBehavior{};
	if (m_pBlackboard->GetData("CurrentSteeringBehavior", pCurrentSteeringBehavior) && pCurrentSteeringBehavior)
	{
		*m_pCurrentSteering = pCurrentSteeringBehavior->CalculateSteering(dt, m_AgentInfo);
		m_pInterface->Draw_Direction(m_AgentInfo.Position, m_pCurrentSteering->LinearVelocity, 10.f, Elite::Vector3{ 1.f,1.f,1.f });

		m_pCurrentSteering->RunMode = m_ShouldRun || m_AgentInfo.Stamina > m_TresholdToRun;
		m_pCurrentSteering->AutoOrient = !m_CanScan;
	}

	//scanning environment
	m_pCurrentSteering->AngularVelocity = m_AgentInfo.MaxAngularSpeed;	

	return *m_pCurrentSteering;
}

void SurvivalAgentPlugin::Render(float dt) const
{
	// grid
	m_pGrid->RenderGrid();
	m_pHouseManager->Render();

	// target position
	Elite::Vector3 color = Elite::Vector3{ 1.f, 0.f, 1.f }; // purple
	float size = 2.f;
	m_pInterface->Draw_SolidCircle(m_pInterface->NavMesh_GetClosestPathPoint(m_Target), .7f, { 0,0 }, { 1, 0, 0 });
	m_pInterface->Draw_Circle(m_pInterface->NavMesh_GetClosestPathPoint(m_Target), size, color);
}

void SurvivalAgentPlugin::InitializeBlackboard()
{
	// Global Data
	m_pBlackboard->AddData("Interface", m_pInterface);

	// Grid Data
	m_pGrid = std::make_unique<ZombieGame::Grid>(m_pBlackboard.get(), 15);
	m_pBlackboard->AddData("Grid", m_pGrid.get());

	// Steering Data
	m_pBlackboard->AddData("CurrentSteering", m_pCurrentSteering.get());
	m_pBlackboard->AddData("CurrentSteeringBehavior", static_cast<ISteeringBehavior*>(nullptr));
	m_pBlackboard->AddData("Seek", m_pSeekBehaviour.get());
	m_pBlackboard->AddData("Face", m_pFaceBehaviour.get());
	m_pBlackboard->AddData("Flee", m_pFleeBehaviour.get());
	m_pBlackboard->AddData("Arrive", m_pArriveBehaviour.get());
	m_pBlackboard->AddData("SeekAndFace", m_pSeekAndFaceBehaviour.get());
	m_pBlackboard->AddData("FleeAndFace", m_pFleeAndFaceBehaviour.get());

	// Agent Data
	m_pBlackboard->AddData("AgentInfo", &m_AgentInfo);
	m_pBlackboard->AddData("RunTreshold", m_TresholdToRun);
	m_pBlackboard->AddData("ShouldRun", &m_ShouldRun);
	m_pBlackboard->AddData("CanScan", &m_CanScan);
	m_pBlackboard->AddData("AlertedTime", &m_AlertedTime);

	// Target Data
	m_pBlackboard->AddData("Target", &m_Target);
	m_pBlackboard->AddData("TargetHouse", static_cast<House*>(nullptr));
	m_pBlackboard->AddData("TargetCheckpoint", static_cast<CheckPoint*>(nullptr));
	m_pBlackboard->AddData("TargetItem", static_cast<Item*>(nullptr));
	m_pBlackboard->AddData("TargetEnemy", static_cast<EnemyInfo*>(nullptr));
	m_pBlackboard->AddData("TargetPurgeZone", static_cast<PurgeZone*>(nullptr));

	// House Data
	m_pBlackboard->AddData("StoredHouses", &m_StoredHouses);
	m_pHouseManager = std::make_unique<ZombieGame::HouseManager>(m_pBlackboard.get());
	m_pBlackboard->AddData("HouseManager", m_pHouseManager.get());

	// Inventory Data
	m_pInventoryManager = std::make_unique<ZombieGame::InventoryManager>(m_pBlackboard.get());
	m_pBlackboard->AddData("InventoryManager", m_pInventoryManager.get());

	// Enemy Data
	m_pEntityManager = std::make_unique<ZombieGame::EntityManager>(m_pBlackboard.get());
	m_pBlackboard->AddData("EntityManager", m_pEntityManager.get());

	// Timers
	m_pBlackboard->AddData("DeltaTime", &m_DeltaTime);
	m_pBlackboard->AddData("ExploreTime", &m_ExploreTime);
}

void SurvivalAgentPlugin::CreateBehaviorTree()
{
	m_pBehaviourTree = std::make_unique<Elite::BehaviorTree>(m_pBlackboard.get(),
		new Elite::BehaviorSelector({
			// Use Medkit if necessary
			new Elite::BehaviorSequence({
				new Elite::BehaviorConditional(BT_Conditions::IsMedkitNeeded),
				new Elite::BehaviorConditional(BT_Conditions::HasMedkit),
				new Elite::BehaviorAction(BT_Actions::UseMedkit)
			}),
			// Use Food if necessary
			new Elite::BehaviorSequence({
				new Elite::BehaviorConditional(BT_Conditions::IsFoodNeeded),
				new Elite::BehaviorConditional(BT_Conditions::HasFood),
				new Elite::BehaviorAction(BT_Actions::UseFood)
			}),
			//Enemy behavior
			new Elite::BehaviorSelector({
				//Purge Zones
				new Elite::BehaviorSequence(
				{
					new Elite::BehaviorConditional(BT_Conditions::IsInPurgeZone),
					new Elite::BehaviorAction(BT_Actions::TargetClosestOutPurgeZonePosition),
					new Elite::BehaviorAction(BT_Actions::SetRunning),
					new Elite::BehaviorAction(BT_Actions::SeekTarget)
				}),
				//Enemy in view
				new Elite::BehaviorSequence({
					new Elite::BehaviorConditional(BT_Conditions::IsEnemyInFOV),
					new Elite::BehaviorAction(BT_Actions::targetClosestEnemyInFOV),
					new Elite::BehaviorSelector({
						//Agent has no weapon
						new Elite::BehaviorSequence({
							new Elite::BehaviorConditional(BT_Conditions::HasNoWeapon),
							new Elite::BehaviorAction(BT_Actions::SetRunning),
							new Elite::BehaviorAction(BT_Actions::FleeAndFaceTarget)
						}),
						//Aiming finished
						new Elite::BehaviorSequence({
							new Elite::BehaviorConditional(BT_Conditions::IsAimingFinished),
							new Elite::BehaviorAction(BT_Actions::Shoot)
						}),
						//Aim at the target
						new Elite::BehaviorAction(BT_Actions::FaceTarget)
					}),
				}),

				//Attack from behind
				new Elite::BehaviorSequence(
				{
					new Elite::BehaviorAction(BT_Actions::HandleAttackFromBehind),
					new Elite::BehaviorAction(BT_Actions::SetRunning),
					new Elite::BehaviorAction(BT_Actions::FleeAndFaceTarget)
				})
			}),
			// Item behaviors
			new Elite::BehaviorSelector({
				// Target Item in FOV and Move to Target
				new Elite::BehaviorSequence({
					new Elite::BehaviorConditional(BT_Conditions::HasNoItemTarget),
					new Elite::BehaviorConditional(BT_Conditions::IsItemInFOV),
					new Elite::BehaviorConditional(BT_Conditions::CanVisitItemInFOV),
					new Elite::BehaviorAction(BT_Actions::TargetItemInFOV),
					new Elite::BehaviorAction(BT_Actions::SeekTarget)
				}),
				// Target closest unvisited Item and Move to Target
				new Elite::BehaviorSequence({
					new Elite::BehaviorConditional(BT_Conditions::HasNoItemTarget),
					new Elite::BehaviorConditional(BT_Conditions::CanVisitKnownItems),
					new Elite::BehaviorAction(BT_Actions::SetClosestItemAsTarget),
					new Elite::BehaviorAction(BT_Actions::SeekTarget)
				}),
				// Check if target item can be grabbed
				new Elite::BehaviorSequence({
					new Elite::BehaviorConditional(BT_Conditions::HasItemTarget),
					new Elite::BehaviorAction(BT_Actions::SeekAndFaceTarget),
					new Elite::BehaviorConditional(BT_Conditions::IsItemInGrabRange),
					// Item handling
					new Elite::BehaviorSelector({
						// add item to inventory when there is an empty slot
						new Elite::BehaviorSequence({
							new Elite::BehaviorConditional(BT_Conditions::HasEmptySlot),
							new Elite::BehaviorConditional(BT_Conditions::IsNotGarbage),
							new Elite::BehaviorAction(BT_Actions::AddItemToInventory),
							new Elite::BehaviorAction(BT_Actions::MarkItemAsVisited)
						}),
						// if all slots are full check if can replace Item
						new Elite::BehaviorSequence({
							new Elite::BehaviorConditional(BT_Conditions::IsNotGarbage),
							new Elite::BehaviorConditional(BT_Conditions::ShouldItemBeReplaced),
							new Elite::BehaviorAction(BT_Actions::ReplaceItem),
							new Elite::BehaviorAction(BT_Actions::MarkItemAsVisited)
						}),
						// If slots are full AND can't be replaced destroy the item on the ground
						new Elite::BehaviorSequence({
							new Elite::BehaviorAction(BT_Actions::DestroyItem),
							new Elite::BehaviorAction(BT_Actions::MarkItemAsVisited)
						})
					})
				})
			}),
			// House behaviors
			new Elite::BehaviorSelector({
				// House In FOV and Move to Target
				new Elite::BehaviorSequence({
					new Elite::BehaviorConditional(BT_Conditions::HasNoItemTarget),
					new Elite::BehaviorConditional(BT_Conditions::HasNoHouseTarget),
					new Elite::BehaviorConditional(BT_Conditions::IsHouseInFOV),
					new Elite::BehaviorConditional(BT_Conditions::CanVisitHouseInFOV),
					new Elite::BehaviorAction(BT_Actions::TargetHouseInFOV),
					new Elite::BehaviorAction(BT_Actions::SeekTarget)
				}),
				// No house In FOV (Target known unvisited houses) and Move to Target
				new Elite::BehaviorSequence({
					new Elite::BehaviorConditional(BT_Conditions::HasNoItemTarget),
					new Elite::BehaviorConditional(BT_Conditions::HasNoHouseTarget),
					new Elite::BehaviorConditional(BT_Conditions::CanVisitKnownHouse),
					new Elite::BehaviorAction(BT_Actions::TargetClosestUnvisitedHouse),
					new Elite::BehaviorAction(BT_Actions::SeekTarget)
				}),
				// Visit House
				new Elite::BehaviorSelector({
					// Check if the agent is outside the target house
					new Elite::BehaviorSequence({
						new Elite::BehaviorConditional(BT_Conditions::IsAgentOutsideTargetHouse),
						new Elite::BehaviorConditional(BT_Conditions::HasNoItemTarget),
						new Elite::BehaviorAction(BT_Actions::SeekTarget)
					}),
					// Search the house
					new Elite::BehaviorSequence({
						new Elite::BehaviorSelector({
							// If no checkpoint target, set closest checkpoint as target
							new Elite::BehaviorSequence({
								new Elite::BehaviorConditional(BT_Conditions::HasNoItemTarget),
								new Elite::BehaviorConditional(BT_Conditions::HasNoCheckpointTarget),
								new Elite::BehaviorConditional(BT_Conditions::CanSearchHouse),
								new Elite::BehaviorAction(BT_Actions::TargetClosestCheckpoint),
								new Elite::BehaviorAction(BT_Actions::SeekTarget)
							}),
							// If checkpoint target exists and not reached, continue to move towards it
							new Elite::BehaviorSequence({
								new Elite::BehaviorConditional(BT_Conditions::HasNoItemTarget),
								new Elite::BehaviorConditional(BT_Conditions::HasCheckpointTarget),
								new Elite::BehaviorConditional(BT_Conditions::HasNotReachedTarget),
								new Elite::BehaviorAction(BT_Actions::SeekTarget)
							}),
							// If checkpoint reached, mark it as visited
							new Elite::BehaviorSequence({
								new Elite::BehaviorConditional(BT_Conditions::HasCheckpointTarget),
								new Elite::BehaviorConditional(BT_Conditions::HasReachedTarget),
								new Elite::BehaviorAction(BT_Actions::MarkCheckPointVisited)
							})
						}),
						// After all checkpoints visited, mark house as visited
						new Elite::BehaviorConditional(BT_Conditions::AreAlLCheckpointsVisited),
						new Elite::BehaviorAction(BT_Actions::MarkHouseAsVisited)
					})
			}),
				// Fallback Exploration
				new Elite::BehaviorSequence({
					new Elite::BehaviorConditional(BT_Conditions::HasNoHouseTarget),
					new Elite::BehaviorConditional(BT_Conditions::HasNoItemTarget),
					new Elite::BehaviorAction(BT_Actions::Explore),
					new Elite::BehaviorAction(BT_Actions::SeekTarget)
				})
			})

			}));
}


