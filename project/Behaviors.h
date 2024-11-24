/*=============================================================================*/
// Copyright 2020-2021 Elite Engine
/*=============================================================================*/
// Behaviors.h: Implementation of certain reusable behaviors for the BT version of the Agario Game
/*=============================================================================*/
#ifndef ELITE_APPLICATION_BEHAVIOR_TREE_BEHAVIORS
#define ELITE_APPLICATION_BEHAVIOR_TREE_BEHAVIORS
//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
#include "EliteMath/EMath.h"

#include "Exam_HelperStructs.h"

#include "EliteBehaviorTree/EBehaviorTree.h"
#include "Steeringbehaviors/SteeringBehaviors.h"
#include "EliteData/EBlackboard.h"

#include "Grid.h"
#include "HouseManager.h"
#include "InventoryManager.h"



// ----------------------------------------------------------------
// Helper Functions
// ----------------------------------------------------------------

template<typename T>
bool GetDataFromBlackboard(Elite::Blackboard* pBlackboard, const std::string& dataName, T*& outputData)
{
    if (!pBlackboard->GetData(dataName, outputData))
    {
        std::cerr << dataName << " isn't stored correctly in Blackboard\n";
        return false;
    }
    return true;
}
bool SetCurrentSteeringBehavior(Elite::Blackboard* pBlackboard, ISteeringBehavior* pNewBehavior, const std::string& behaviorName)
{
    ISteeringBehavior* pCurrentSteeringBehavior{};
    if (!GetDataFromBlackboard(pBlackboard, "CurrentSteeringBehavior", pCurrentSteeringBehavior))
        return false;

    if (pCurrentSteeringBehavior != pNewBehavior)
    {
        std::cout << "SteeringBehavior changed to " << behaviorName << ". \n";
        pCurrentSteeringBehavior = pNewBehavior;
        pBlackboard->ChangeData("CurrentSteeringBehavior", pCurrentSteeringBehavior);
    }
    return true;
}

//-----------------------------------------------------------------
// Behaviors
//-----------------------------------------------------------------

namespace BT_Actions
{
    // Steering Actions
    Elite::BehaviorState SetRunning(Elite::Blackboard* pBlackboard)
    {
        bool* pShouldRun{};
        if (!GetDataFromBlackboard(pBlackboard, "ShouldRun", pShouldRun))
            return Elite::BehaviorState::Failure;

        *pShouldRun = true;
        return Elite::BehaviorState::Success;
    }

    Elite::BehaviorState SeekTarget(Elite::Blackboard* pBlackboard)
    {
        IExamInterface* pInterface{};
        if (!GetDataFromBlackboard(pBlackboard, "Interface", pInterface))
            return Elite::BehaviorState::Failure;

        Elite::Vector2* pTarget{};
        if (!GetDataFromBlackboard(pBlackboard, "Target", pTarget))
            return Elite::BehaviorState::Failure;

        Seek* pSeek{};
        if (!GetDataFromBlackboard(pBlackboard, "Seek", pSeek))
            return Elite::BehaviorState::Failure;

        // set target to closest point in navmesh
        pSeek->SetTarget(pInterface->NavMesh_GetClosestPathPoint(*pTarget));

        //change behavior if necessary
        if (!SetCurrentSteeringBehavior(pBlackboard, pSeek, "Seek"))
            return Elite::BehaviorState::Failure;

        return Elite::BehaviorState::Success;
    }   
    Elite::BehaviorState SeekAndFaceTarget(Elite::Blackboard* pBlackboard)
    {
        IExamInterface* pInterface{};
        if (!GetDataFromBlackboard(pBlackboard, "Interface", pInterface))
            return Elite::BehaviorState::Failure;

        Elite::Vector2* pTarget{};
        if (!GetDataFromBlackboard(pBlackboard, "Target", pTarget))
            return Elite::BehaviorState::Failure;

        Seek* pSeek{};
        if (!GetDataFromBlackboard(pBlackboard, "Seek", pSeek))
            return Elite::BehaviorState::Failure;

        Face* pFace{};
        if (!GetDataFromBlackboard(pBlackboard, "Face", pFace))
            return Elite::BehaviorState::Failure;

        BlendedSteering* pSeekAndFace{};
        if (!GetDataFromBlackboard(pBlackboard, "SeekAndFace", pSeekAndFace))
            return Elite::BehaviorState::Failure;

        // set target to closest point in navmesh
        pSeek->SetTarget(pInterface->NavMesh_GetClosestPathPoint(*pTarget));
        pFace->SetTarget(pInterface->NavMesh_GetClosestPathPoint(*pTarget));

        //change behavior if necessary
        if (!SetCurrentSteeringBehavior(pBlackboard, pSeekAndFace, "SeekAndFace"))
            return Elite::BehaviorState::Failure;

        return Elite::BehaviorState::Success;
    }
    Elite::BehaviorState FleeAndFaceTarget(Elite::Blackboard* pBlackboard)
    {
        IExamInterface* pInterface{};
        if (!GetDataFromBlackboard(pBlackboard, "Interface", pInterface))
            return Elite::BehaviorState::Failure;

        Flee* pFlee{};
        if (!GetDataFromBlackboard(pBlackboard, "Flee", pFlee))
            return Elite::BehaviorState::Failure;

        Face* pFace{};
        if (!GetDataFromBlackboard(pBlackboard, "Face", pFace))
            return Elite::BehaviorState::Failure;

        BlendedSteering* pFleeAndFace{};
        if (!GetDataFromBlackboard(pBlackboard, "FleeAndFace", pFleeAndFace))
            return Elite::BehaviorState::Failure;

        Elite::Vector2* pTarget{};
        if (!GetDataFromBlackboard(pBlackboard, "Target", pTarget))
            return Elite::BehaviorState::Failure;

        // set target to closest point in navmesh
        pFace->SetTarget(pInterface->NavMesh_GetClosestPathPoint(*pTarget));
        pFlee->SetTarget(pInterface->NavMesh_GetClosestPathPoint(*pTarget));

        //change behavior if necessary
        if (!SetCurrentSteeringBehavior(pBlackboard, pFleeAndFace, "FleeAndFace"))
            return Elite::BehaviorState::Failure;

        return Elite::BehaviorState::Success;
    }
    Elite::BehaviorState FaceTarget(Elite::Blackboard* pBlackboard)
    {
        IExamInterface* pInterface{};
        if (!GetDataFromBlackboard(pBlackboard, "Interface", pInterface))
            return Elite::BehaviorState::Failure;

        Face* pFace{};
        if (!GetDataFromBlackboard(pBlackboard, "Face", pFace))
            return Elite::BehaviorState::Failure;

        Elite::Vector2* pTarget{};
        if (!GetDataFromBlackboard(pBlackboard, "Target", pTarget))
            return Elite::BehaviorState::Failure;

        // set target to closest point in navmesh
        pFace->SetTarget(pInterface->NavMesh_GetClosestPathPoint(*pTarget));

        //change behavior if necessary
        if (!SetCurrentSteeringBehavior(pBlackboard, pFace, "Face"))
            return Elite::BehaviorState::Failure;

        return Elite::BehaviorState::Success;
    }

    // Exploration Actions

    Elite::BehaviorState Explore(Elite::Blackboard* pBlackboard)
    {
        ZombieGame::Grid* pGrid{};
        if (!GetDataFromBlackboard(pBlackboard, "Grid", pGrid))
            return Elite::BehaviorState::Failure;

        AgentInfo* pAgentInfo{};
        if (!GetDataFromBlackboard(pBlackboard, "AgentInfo", pAgentInfo))
            return Elite::BehaviorState::Failure;

        // Get next cell of current Direction Leg
        Cell* bestTargetCell = pGrid->GetExpandSquareSearchCell(pAgentInfo);
        if (bestTargetCell == nullptr)
            return Elite::BehaviorState::Failure;

        Elite::Vector2* pTarget{};
        if (!GetDataFromBlackboard(pBlackboard, "Target", pTarget))
            return Elite::BehaviorState::Failure;

        SteeringPlugin_Output_Extended* pSteering;
        if (!GetDataFromBlackboard(pBlackboard, "CurrentSteering", pSteering))
            return Elite::BehaviorState::Failure;

        // set center next cell as target
        *pTarget = bestTargetCell->Position;
        pBlackboard->ChangeData("TargetHouse", static_cast<House*>(nullptr));

        return Elite::BehaviorState::Success;
    }

    // House Actions
    Elite::BehaviorState TargetHouseInFOV(Elite::Blackboard* pBlackboard)
    {
        IExamInterface* pInterface{};
        if (!GetDataFromBlackboard(pBlackboard, "Interface", pInterface))
            return Elite::BehaviorState::Failure;

        ZombieGame::HouseManager* pHouseManager{};
        if (!GetDataFromBlackboard(pBlackboard, "HouseManager", pHouseManager))
            return Elite::BehaviorState::Failure;

        Elite::Vector2* pTarget{};
        if (!GetDataFromBlackboard(pBlackboard, "Target", pTarget))
            return Elite::BehaviorState::Failure;

        House* pTargetHouse{};
        if (!GetDataFromBlackboard(pBlackboard, "TargetHouse", pTargetHouse) || pTargetHouse == nullptr)
        {
            // check houses in FOV
            for (const auto& houseFOV : pInterface->GetHousesInFOV())
            {
                if (!pHouseManager->GetHouse(houseFOV)->IsVisited)
                {
                    // set house as target
                    std::cout << "New House as target \n";
                    pTargetHouse = pHouseManager->GetHouse(houseFOV);
                    pBlackboard->ChangeData("TargetHouse", pTargetHouse);

                    // set house as next position target 
                    *pTarget = pTargetHouse->Center;
                    return Elite::BehaviorState::Success;
                }
            }
        }

        std::cout << "There is already a target house, TargetHouseInFOV returned failure! \n";
        return Elite::BehaviorState::Failure;
    }
    Elite::BehaviorState TargetClosestUnvisitedHouse(Elite::Blackboard* pBlackboard)
    {
        std::cout << "TargetClosestUnvisitedHouse called \n";

        ZombieGame::Grid* pGrid{};
        if (!GetDataFromBlackboard(pBlackboard, "Grid", pGrid))
            return Elite::BehaviorState::Failure;

        pGrid->StoreLastVisitedCell();

        AgentInfo* pAgentInfo{};
        if (!GetDataFromBlackboard(pBlackboard, "AgentInfo", pAgentInfo))
            return Elite::BehaviorState::Failure;

        ZombieGame::HouseManager* pHouseManager{};
        if (!GetDataFromBlackboard(pBlackboard, "HouseManager", pHouseManager))
            return Elite::BehaviorState::Failure;

        // Get closest unvited house
        House* pClosestHouse = pHouseManager->GetClosestUnvisitedHouse(pAgentInfo->Position);
        if (pClosestHouse == nullptr)
            return Elite::BehaviorState::Failure;

        Elite::Vector2* pTarget{};
        if (!GetDataFromBlackboard(pBlackboard, "Target", pTarget))
            return Elite::BehaviorState::Failure;

        House* pTargetHouse{};

        //only when there isn't already a target house
        if (GetDataFromBlackboard(pBlackboard, "TargetHouse", pTargetHouse) && pTargetHouse == nullptr)
        {
            *pTarget = pClosestHouse->Center;
            pBlackboard->ChangeData("TargetHouse", pClosestHouse);
            return Elite::BehaviorState::Success;
        }

        std::cout << "There is already a target house, TargetClosestUnvisitedHouse returned failure! \n";
        return Elite::BehaviorState::Failure;
    }
    Elite::BehaviorState MarkHouseAsVisited(Elite::Blackboard* pBlackboard)
    {
        ZombieGame::HouseManager* pHouseManager{};
        if (!GetDataFromBlackboard(pBlackboard, "HouseManager", pHouseManager))
            return Elite::BehaviorState::Failure;

        ZombieGame::Grid* pGrid{};
        if (!GetDataFromBlackboard(pBlackboard, "Grid", pGrid))
            return Elite::BehaviorState::Failure;

        House* pHouse{};
        if (!GetDataFromBlackboard(pBlackboard, "TargetHouse", pHouse))
            return Elite::BehaviorState::Failure;

        // Mark house as visited
        std::cout << "House marked as visited... \n";
        pHouseManager->MarkHouseAsVisited(pHouse);
        // Mark cell of the house as visited
        pGrid->MarkCellVisited(pHouse->Center);
        // Remove house as target
        pBlackboard->ChangeData("TargetHouse", static_cast<House*>(nullptr));

        return Elite::BehaviorState::Success;
    }
    Elite::BehaviorState TargetClosestCheckpoint(Elite::Blackboard* pBlackboard)
    {
        IExamInterface* pInterface{};
        if (!GetDataFromBlackboard(pBlackboard, "Interface", pInterface))
            return Elite::BehaviorState::Failure;

        AgentInfo* pAgentInfo{};
        if (!GetDataFromBlackboard(pBlackboard, "AgentInfo", pAgentInfo))
            return Elite::BehaviorState::Failure;

        ZombieGame::HouseManager* pHouseManager{};
        if (!GetDataFromBlackboard(pBlackboard, "HouseManager", pHouseManager))
            return Elite::BehaviorState::Failure;

        House* pHouse{};
        if (!GetDataFromBlackboard(pBlackboard, "TargetHouse", pHouse))
            return Elite::BehaviorState::Failure;

        Elite::Vector2* pTarget{};
        if (!GetDataFromBlackboard(pBlackboard, "Target", pTarget))
            return Elite::BehaviorState::Failure;

        CheckPoint* pCheckpoint{};
        if (!GetDataFromBlackboard(pBlackboard, "TargetCheckpoint", pCheckpoint))
            return Elite::BehaviorState::Failure;

        // Only target next checkpoint if there isn't alreadya checkpoint
        if (pCheckpoint == nullptr)
        {
            std::cout << "New checkpoint as target\n";

            // Get next checkpoint
            pCheckpoint = pHouseManager->GetNextCheckpoint(pAgentInfo->Position, pHouse);

            // set next checkpoint as target
            pBlackboard->ChangeData("TargetCheckpoint", pCheckpoint);
            *pTarget = pCheckpoint->Position;
        }

        std::cout << "There was already a checkpoint as target\n";

        return Elite::BehaviorState::Success;
    }
    Elite::BehaviorState MarkCheckPointVisited(Elite::Blackboard* pBlackboard)
    {
        CheckPoint* pCheckpoint{};
        if (!GetDataFromBlackboard(pBlackboard, "TargetCheckpoint", pCheckpoint))
            return Elite::BehaviorState::Failure;

        House* pHouse{};
        if (!GetDataFromBlackboard(pBlackboard, "TargetHouse", pHouse))
            return Elite::BehaviorState::Failure;

        auto finder = [&](CheckPoint* pCheckPoint) { return pCheckPoint == pCheckpoint; };
        auto iterator = std::find_if(pHouse->pCheckPoints.begin(), pHouse->pCheckPoints.end(), finder);

        if (iterator == pHouse->pCheckPoints.end())
            return Elite::BehaviorState::Failure;

        (*iterator)->IsVisited = true;
        pBlackboard->ChangeData("TargetCheckpoint", static_cast<CheckPoint*>(nullptr));

        return Elite::BehaviorState::Success;
    }

    // Item Actions

    Elite::BehaviorState TargetItemInFOV(Elite::Blackboard* pBlackboard)
    {
        IExamInterface* pInterface{};
        if (!GetDataFromBlackboard(pBlackboard, "Interface", pInterface))
            return Elite::BehaviorState::Failure;

        ZombieGame::InventoryManager* pInventoryManager{};
        if (!GetDataFromBlackboard(pBlackboard, "InventoryManager", pInventoryManager))
            return Elite::BehaviorState::Failure;

        Elite::Vector2* pTarget{};
        if (!GetDataFromBlackboard(pBlackboard, "Target", pTarget))
            return Elite::BehaviorState::Failure;

        bool* pCanScan{};
        if (!GetDataFromBlackboard(pBlackboard, "CanScan", pCanScan))
            return Elite::BehaviorState::Failure;

        Item* pTargetItem{};
        if (!GetDataFromBlackboard(pBlackboard, "TargetItem", pTargetItem) || pTargetItem != nullptr)
            std::cout << "There is already a target item.\n";
            return Elite::BehaviorState::Failure;

        for (const auto& itemFOV : pInterface->GetItemsInFOV())
        {
            // return first item in FOV
            if (!pInventoryManager->GetStoredItem(itemFOV)->IsVisited)
            {
                // Set item as Target
                std::cout << "New item set as target\n";
                pTargetItem = pInventoryManager->GetStoredItem(itemFOV);
                pBlackboard->ChangeData("TargetItem", pTargetItem);

                // disable scanning so the itemisin fov
                *pCanScan = false;

                // Removes checkpoint as target
                pBlackboard->ChangeData("TargetCheckpoint", static_cast<CheckPoint*>(nullptr));

                //Set items location as target goal
                *pTarget = pTargetItem->itemInfo.Location;
                return Elite::BehaviorState::Success;
            }
        }

        return Elite::BehaviorState::Failure;
    }
    Elite::BehaviorState SetClosestItemAsTarget(Elite::Blackboard* pBlackboard)
    {
        ZombieGame::Grid* pGrid{};
        if (!GetDataFromBlackboard(pBlackboard, "Grid", pGrid))
            return Elite::BehaviorState::Failure;

        // store last visited cell so it can continue exploring from that position
        pGrid->StoreLastVisitedCell();

        AgentInfo* pAgentInfo{};
        if (!GetDataFromBlackboard(pBlackboard, "AgentInfo", pAgentInfo))
            return Elite::BehaviorState::Failure;

        ZombieGame::InventoryManager* pInventoryManager{};
        if (!GetDataFromBlackboard(pBlackboard, "InventoryManager", pInventoryManager))
            return Elite::BehaviorState::Failure;

        Item* pClosestItem = pInventoryManager->GetClosestUnvisitedItem(pAgentInfo->Position);
        if (pClosestItem == nullptr)
            return Elite::BehaviorState::Failure;

        Elite::Vector2* pTarget{};
        if (!GetDataFromBlackboard(pBlackboard, "Target", pTarget))
            return Elite::BehaviorState::Failure;

        bool* pCanScan{};
        if (!GetDataFromBlackboard(pBlackboard, "CanScan", pCanScan))
            return Elite::BehaviorState::Failure;

        // Set closest item as new item target
        std::cout << "New closest item set as target\n";
        pBlackboard->ChangeData("TargetItem", pClosestItem);

        // Disable scanning so the item is in fov
        *pCanScan = false;

        // Set new item position as target goal
        *pTarget = pClosestItem->itemInfo.Location;

        return Elite::BehaviorState::Success;
    }
    Elite::BehaviorState AddItemToInventory(Elite::Blackboard* pBlackboard)
    {
        IExamInterface* pInterface{};
        if (!GetDataFromBlackboard(pBlackboard, "Interface", pInterface))
            return Elite::BehaviorState::Failure;

        ZombieGame::InventoryManager* pInventoryManager{};
        if (!GetDataFromBlackboard(pBlackboard, "InventoryManager", pInventoryManager))
            return Elite::BehaviorState::Failure;

        Item* pTargetItem{};
        if (!GetDataFromBlackboard(pBlackboard, "TargetItem", pTargetItem))
            return Elite::BehaviorState::Failure;

        // Determine slot index for item
        int slotIndex;
        if (pTargetItem->itemInfo.Type == eItemType::SHOTGUN && pInventoryManager->IsShotgunSlotEmpty())
        {
            slotIndex = 0; // Reserved slot for Shotgun
        }
        else if (pTargetItem->itemInfo.Type == eItemType::PISTOL && pInventoryManager->IsPistolSlotEmpty())
        {
            slotIndex = 1; // Reserved slot for Pistol
        }
        else
        {
            slotIndex = pInventoryManager->FindFirstEmptySlotExcludingReserved();
            if (slotIndex == -1)
            {
                std::cout << "No empty slot available for item\n";
                return Elite::BehaviorState::Failure;
            }
        }

        if (pInterface->GrabItem(pTargetItem->itemInfo))
        {
            if (pInterface->Inventory_AddItem(slotIndex, pTargetItem->itemInfo))
            {
                pInventoryManager->UpdateInventorySlot(slotIndex, pTargetItem->itemInfo);
                return Elite::BehaviorState::Success;
            }
            else
            {
                // Adding failed
                return Elite::BehaviorState::Failure;
            }
        }
        else
        {
            // Grabbing failed
            return Elite::BehaviorState::Failure;
        }
    }
    Elite::BehaviorState ReplaceItem(Elite::Blackboard* pBlackboard)
    {
        IExamInterface* pInterface{};
        if (!GetDataFromBlackboard(pBlackboard, "Interface", pInterface))
            return Elite::BehaviorState::Failure;

        ZombieGame::InventoryManager* pInventoryManager{};
        if (!GetDataFromBlackboard(pBlackboard, "InventoryManager", pInventoryManager))
            return Elite::BehaviorState::Failure;

        Item* pTargetItem{};
        if (!GetDataFromBlackboard(pBlackboard, "TargetItem", pTargetItem))
            return Elite::BehaviorState::Failure;

        int slotToReplace = pInventoryManager->GetSlotForItemToReplace(pTargetItem->itemInfo);
        if (slotToReplace == -1)
            return Elite::BehaviorState::Failure;

        // remove item from interface and add it
        if (pInterface->GrabItem(pTargetItem->itemInfo) &&
            pInterface->Inventory_RemoveItem(slotToReplace) &&
            pInterface->Inventory_AddItem(slotToReplace, pTargetItem->itemInfo))
        {
            // recalibrate the Inventory data with interface inventory data
            pInventoryManager->UpdateInventorySlot(slotToReplace, pTargetItem->itemInfo);
            return Elite::BehaviorState::Success;
        }

        return Elite::BehaviorState::Failure;
    }
    Elite::BehaviorState DestroyItem(Elite::Blackboard* pBlackboard)
    {
        IExamInterface* pInterface{};
        if (!GetDataFromBlackboard(pBlackboard, "Interface", pInterface))
            return Elite::BehaviorState::Failure;

        Item* pTargetItem{};
        if (!GetDataFromBlackboard(pBlackboard, "TargetItem", pTargetItem))
            return Elite::BehaviorState::Failure;

        if (pInterface->DestroyItem(pTargetItem->itemInfo))
            return Elite::BehaviorState::Success;

        return Elite::BehaviorState::Failure;
    }
    Elite::BehaviorState MarkItemAsVisited(Elite::Blackboard* pBlackboard)
    {
        Item* pTargetItem{};
        if (!GetDataFromBlackboard(pBlackboard, "TargetItem", pTargetItem))
            return Elite::BehaviorState::Failure;

        ZombieGame::InventoryManager* pInventoryManager{};
        if (!GetDataFromBlackboard(pBlackboard, "InventoryManager", pInventoryManager))
            return Elite::BehaviorState::Failure;

        bool* pCanScan{};
        if (!GetDataFromBlackboard(pBlackboard, "CanScan", pCanScan))
            return Elite::BehaviorState::Failure;

        // Check if target item exists in stored items
        auto finder = [&](const std::unique_ptr<Item>& pItem) { return pItem.get() == pTargetItem; };
        auto& storedItems = pInventoryManager->GetStoredItems();
        auto iterator = std::find_if(storedItems.begin(), storedItems.end(), finder);

        // Target item doesn't exist in stored items
        if (iterator == storedItems.end())
            return Elite::BehaviorState::Failure;

        // Set item as visited
        (*iterator)->IsVisited = true;

        // Remove the taken item from Item target
        pBlackboard->ChangeData("TargetItem", static_cast<Item*>(nullptr));

        // Reset scanningto default
        *pCanScan = true;

        return Elite::BehaviorState::Success;
    }
    Elite::BehaviorState UseMedkit(Elite::Blackboard* pBlackboard)
    {
        ZombieGame::InventoryManager* pInventoryManager{};
        if (!GetDataFromBlackboard(pBlackboard, "InventoryManager", pInventoryManager))
            return Elite::BehaviorState::Failure;

        AgentInfo* pAgentInfo{};
        if (!GetDataFromBlackboard(pBlackboard, "AgentInfo", pAgentInfo))
            return Elite::BehaviorState::Failure;

        const float healthThreshold = 5.0f;

        if (pInventoryManager->UseMedkit(healthThreshold, pAgentInfo->Health))
            return Elite::BehaviorState::Success;

        return Elite::BehaviorState::Failure;
    }
    Elite::BehaviorState UseFood(Elite::Blackboard* pBlackboard)
    {
        ZombieGame::InventoryManager* pInventoryManager{};
        if (!GetDataFromBlackboard(pBlackboard, "InventoryManager", pInventoryManager))
            return Elite::BehaviorState::Failure;

        AgentInfo* pAgentInfo{};
        if (!GetDataFromBlackboard(pBlackboard, "AgentInfo", pAgentInfo))
            return Elite::BehaviorState::Failure;

        const float energyThreshold = 6.0f;
        if (pInventoryManager->UseFood(energyThreshold, pAgentInfo->Energy))
            return Elite::BehaviorState::Success;

        return Elite::BehaviorState::Failure;
    }

    // Enemy Actions

    Elite::BehaviorState targetClosestEnemyInFOV(Elite::Blackboard* pBlackboard)
    {
        std::cout << "TargetEnemyInFOV called\n";

        float* pAlertedTime{};
        if (!GetDataFromBlackboard(pBlackboard, "AlertedTime", pAlertedTime))
            return Elite::BehaviorState::Failure;

        // Reset time of alert
        *pAlertedTime = 0.f;

        IExamInterface* pInterface{};
        if (!GetDataFromBlackboard(pBlackboard, "Interface", pInterface))
            return Elite::BehaviorState::Failure;

        ZombieGame::EntityManager* EntityManager{};
        if (!GetDataFromBlackboard(pBlackboard, "EntityManager", EntityManager))
            return Elite::BehaviorState::Failure;

        AgentInfo* pAgentInfo{};
        if (!GetDataFromBlackboard(pBlackboard, "AgentInfo", pAgentInfo))
            return Elite::BehaviorState::Failure;

        Elite::Vector2* pTarget{};
        if (!GetDataFromBlackboard(pBlackboard, "Target", pTarget))
            return Elite::BehaviorState::Failure;

        EnemyInfo* pTargetEnemy = EntityManager->SetClosestEnemyAsTarget(pAgentInfo->Position);
        if (!pTargetEnemy)
            return Elite::BehaviorState::Failure;

        // Reset other targets to avoid problems by reassigning targets
        pBlackboard->ChangeData("TargetEnemy", pTargetEnemy);
        pBlackboard->ChangeData("TargetHouse", static_cast<House*>(nullptr));
        pBlackboard->ChangeData("TargetItem", static_cast<Item*>(nullptr));

        // Set agent target location to enemies location
        *pTarget = pTargetEnemy->Location;

        return Elite::BehaviorState::Success;
    }
    Elite::BehaviorState Shoot(Elite::Blackboard* pBlackboard)
    {
        IExamInterface* pInterface{};
        if (!GetDataFromBlackboard(pBlackboard, "Interface", pInterface))
            return Elite::BehaviorState::Failure;

        Elite::Vector2* pTarget{};
        if (!GetDataFromBlackboard(pBlackboard, "Target", pTarget))
            return Elite::BehaviorState::Failure;

        AgentInfo* pAgent{};
        if (!GetDataFromBlackboard(pBlackboard, "AgentInfo", pAgent))
            return Elite::BehaviorState::Failure;

        ZombieGame::InventoryManager* pInventoryManager{};
        if (!GetDataFromBlackboard(pBlackboard, "InventoryManager", pInventoryManager))
            return Elite::BehaviorState::Failure;

        // Get slot of the weapon with most ammo
        int weaponSlotIndex = pInventoryManager->GetBestWeaponSlotIndex(*pAgent, *pTarget);
        if (weaponSlotIndex == -1)
            return Elite::BehaviorState::Failure;

        if (!pInterface->Inventory_UseItem(weaponSlotIndex))
            pInventoryManager->EmptyInventorySlot(weaponSlotIndex); // is use item = false : there is no ammo so BYEBYE :)

        return Elite::BehaviorState::Success;
    }
    Elite::BehaviorState HandleAttackFromBehind(Elite::Blackboard* pBlackboard)
    {
        float* pDeltaTime{};
        if (!GetDataFromBlackboard(pBlackboard, "DeltaTime", pDeltaTime))
            return Elite::BehaviorState::Failure;

        float* pAlertedTime{};
        if (!GetDataFromBlackboard(pBlackboard, "AlertedTime", pAlertedTime))
            return Elite::BehaviorState::Failure;

        const float maxAlertTime = 4.f;

        AgentInfo* pAgentInfo{};
        if (!GetDataFromBlackboard(pBlackboard, "AgentInfo", pAgentInfo))
            return Elite::BehaviorState::Failure;

        Elite::Vector2* pTarget{};
        if (!GetDataFromBlackboard(pBlackboard, "Target", pTarget))
            return Elite::BehaviorState::Failure;

        const float safeDistance = 100.f;

        if (pAgentInfo->Bitten)
        {
            *pAlertedTime += *pDeltaTime;
            std::cout << "Attack from behind\n";

            // Move to a safe distance from the enemy + far enough so it has the enemy in fov to shoot
            *pTarget = pAgentInfo->Position - Elite::OrientationToVector(pAgentInfo->Orientation) * safeDistance;
            return Elite::BehaviorState::Success;
        }
        else if (*pAlertedTime > 0.f && *pAlertedTime < maxAlertTime)
        {
            *pAlertedTime += *pDeltaTime;
            // continue if agent got attacked within max alert time
            return Elite::BehaviorState::Success;
        }

        // Agent didn't get attacked within max alert time
        *pAlertedTime = 0.f;
        return Elite::BehaviorState::Failure;
    }

    //Purge zone Actions

    Elite::BehaviorState TargetClosestOutPurgeZonePosition(Elite::Blackboard* pBlackboard)
{
    ZombieGame::EntityManager* pEntityManager{};
    if (!GetDataFromBlackboard(pBlackboard, "EntityManager", pEntityManager))
        return Elite::BehaviorState::Failure;

    AgentInfo* pAgentInfo{};
    if (!GetDataFromBlackboard(pBlackboard, "AgentInfo", pAgentInfo))
        return Elite::BehaviorState::Failure;

    PurgeZone* pClosestPurgeZone = pEntityManager->GetClosestPurgeZone(pAgentInfo->Position);
    if (!pClosestPurgeZone)
        return Elite::BehaviorState::Failure;

    // Get opposite direction of the purge zone center
    Elite::Vector2 direction = pAgentInfo->Position - pClosestPurgeZone->Center;
    direction.Normalize();

    Elite::Vector2* pTarget{};
    if (!GetDataFromBlackboard(pBlackboard, "Target", pTarget))
        return Elite::BehaviorState::Failure;

    // reset all targets, because purge zone is more urgent
    pBlackboard->ChangeData("TargetHouse", static_cast<House*>(nullptr));
    pBlackboard->ChangeData("TargetCheckpoint", static_cast<CheckPoint*>(nullptr));
    pBlackboard->ChangeData("TargetItem", static_cast<Item*>(nullptr));

    // Set closest point outside the purgezone as target
    *pTarget = pClosestPurgeZone->Center + direction * pClosestPurgeZone->Radius;

    return Elite::BehaviorState::Success;
}
}
//-----------------------------------------------------------------
// Conditions
//-----------------------------------------------------------------

namespace BT_Conditions
{
    // House Conditions

    bool HasNoHouseTarget(Elite::Blackboard* pBlackboard)
    {
        House* pHouse{};
        return !GetDataFromBlackboard(pBlackboard, "TargetHouse", pHouse) || pHouse == nullptr;
    }
    bool IsHouseInFOV(Elite::Blackboard* pBlackboard)
    {
        IExamInterface* pInterface{};
        if (!GetDataFromBlackboard(pBlackboard, "Interface", pInterface))
            return false;

        return pInterface->FOV_GetStats().NumHouses > 0;
    }
    bool CanVisitHouseInFOV(Elite::Blackboard* pBlackboard)
    {
        ZombieGame::HouseManager* pHouseManager{};
        return GetDataFromBlackboard(pBlackboard, "HouseManager", pHouseManager) && pHouseManager && pHouseManager->CanVisitHouseInFOV();
    }
    bool CanVisitKnownHouse(Elite::Blackboard* pBlackboard)
    {
        ZombieGame::HouseManager* pHouseManager{};
        if (!GetDataFromBlackboard(pBlackboard, "HouseManager", pHouseManager))
            return false;

        // check if there is a stored house that isn't visited yet
        auto& houses = pHouseManager->GetStoredHouses();
        return std::any_of(houses.begin(), houses.end(), [](const std::unique_ptr<House>& pHouse)
            {
                return pHouse && !pHouse->IsVisited;
            });
    }
    bool IsAgentOutsideTargetHouse(Elite::Blackboard* pBlackboard)
    {
        House* pHouse{};
        if (!GetDataFromBlackboard(pBlackboard, "TargetHouse", pHouse) || pHouse == nullptr)
            return false;

        AgentInfo* pAgentInfo{};
        if (!GetDataFromBlackboard(pBlackboard, "AgentInfo", pAgentInfo))
            return false;

        Elite::Vector2 halfSize = pHouse->Size * 0.5f;
        Elite::Vector2 distance = pAgentInfo->Position - pHouse->Center;

        return std::abs(distance.x) >= halfSize.x || std::abs(distance.y) >= halfSize.y;
    }
    bool CanSearchHouse(Elite::Blackboard* pBlackboard)
    {
        House* pHouse{};
        if (!GetDataFromBlackboard(pBlackboard, "TargetHouse", pHouse) || pHouse == nullptr)
            return false;

        // check if thereis atleast 1 checkpoint not visited in the target house
        return std::any_of(pHouse->pCheckPoints.begin(), pHouse->pCheckPoints.end(), [](CheckPoint* pCheckpoint)
            {
                return pCheckpoint && !pCheckpoint->IsVisited;
            });
    }
    bool HasCheckpointTarget(Elite::Blackboard* pBlackboard)
    {
        CheckPoint* pCheckpoint{};
        return GetDataFromBlackboard(pBlackboard, "TargetCheckpoint", pCheckpoint) && pCheckpoint != nullptr;
    }
    bool HasNoCheckpointTarget(Elite::Blackboard* pBlackboard)
    {
        return !HasCheckpointTarget(pBlackboard);
    }
    bool AreAlLCheckpointsVisited(Elite::Blackboard* pBlackboard)
    {
        return !CanSearchHouse(pBlackboard);
    }

    // Steering Behaviors Conditions

    bool HasReachedTarget(Elite::Blackboard* pBlackboard)
    {
        Elite::Vector2* pTarget{};
        if (!GetDataFromBlackboard(pBlackboard, "Target", pTarget))
            return false;

        AgentInfo* pAgentInfo{};
        if (!GetDataFromBlackboard(pBlackboard, "AgentInfo", pAgentInfo))
            return false;

        // so the target stays in fov distance
        float threshold = 3.f;
        return Elite::DistanceSquared(*pTarget, pAgentInfo->Position) < threshold * threshold;
    }
    bool HasNotReachedTarget(Elite::Blackboard* pBlackboard)
    {
        return !HasReachedTarget(pBlackboard);
    }

    // Item Conditions

    bool IsItemInFOV(Elite::Blackboard* pBlackboard)
    {
        IExamInterface* pInterface{};
        if (!GetDataFromBlackboard(pBlackboard, "Interface", pInterface))
            return false;

        return pInterface->FOV_GetStats().NumItems > 0;
    }
    bool CanVisitItemInFOV(Elite::Blackboard* pBlackboard)
    {
        ZombieGame::InventoryManager* pInventoryManager{};
        return GetDataFromBlackboard(pBlackboard, "InventoryManager", pInventoryManager) &&
            pInventoryManager && pInventoryManager->CanVisitItemInFOV();
    }
    bool CanVisitKnownItems(Elite::Blackboard* pBlackboard)
    {
        ZombieGame::InventoryManager* pInventoryManager{};
        if (!GetDataFromBlackboard(pBlackboard, "InventoryManager", pInventoryManager))
            return false;

        House* pTargetHouse{};
        if (!GetDataFromBlackboard(pBlackboard, "TargetHouse", pTargetHouse))
            return false;

        // check in stored items if there isan item not visited yet
        auto& pStoredItems = pInventoryManager->GetStoredItems();
        auto findNotVisited = [&](const std::unique_ptr<Item>& pItem)
            {
                return pItem && !pItem->IsVisited && pItem->pHouse == pTargetHouse;
            };

        return std::find_if(pStoredItems.begin(), pStoredItems.end(), findNotVisited) != pStoredItems.end();
    }
    bool IsItemInGrabRange(Elite::Blackboard* pBlackboard)
    {
        Item* pTargetItem{};
        if (!GetDataFromBlackboard(pBlackboard, "TargetItem", pTargetItem))
            return false;

        AgentInfo* pAgentInfo{};
        if (!GetDataFromBlackboard(pBlackboard, "AgentInfo", pAgentInfo))
            return false;

        IExamInterface* pInterface{};
        if (!GetDataFromBlackboard(pBlackboard, "Interface", pInterface))
            return false;

        // check if item in grab range is also in FOV
        auto itemsInFOV = pInterface->GetItemsInFOV();
        bool isInFov = std::any_of(itemsInFOV.begin(), itemsInFOV.end(), [&](const ItemInfo& itemInfo)
            {
                return itemInfo.ItemHash == pTargetItem->itemInfo.ItemHash;
            });

        return pTargetItem->itemInfo.Location.DistanceSquared(pAgentInfo->Position) <= pAgentInfo->GrabRange * pAgentInfo->GrabRange && isInFov;
    }
    bool HasItemTarget(Elite::Blackboard* pBlackboard)
    {
        Item* pTargetItem{};
        return GetDataFromBlackboard(pBlackboard, "TargetItem", pTargetItem) && pTargetItem != nullptr;
    }
    bool HasNoItemTarget(Elite::Blackboard* pBlackboard)
    {
        return !HasItemTarget(pBlackboard);
    }
    bool IsGarbage(Elite::Blackboard* pBlackboard)
    {
        Item* pTargetItem{};
        if (!GetDataFromBlackboard(pBlackboard, "TargetItem", pTargetItem))
            return false;

        return pTargetItem->itemInfo.Type == eItemType::GARBAGE;
    }
    bool IsNotGarbage(Elite::Blackboard* pBlackboard)
    {
        return !IsGarbage(pBlackboard);
    }
    bool HasEmptySlot(Elite::Blackboard* pBlackboard)
    {
        ZombieGame::InventoryManager* pInventoryManager{};
        if (!GetDataFromBlackboard(pBlackboard, "InventoryManager", pInventoryManager))
            return false;

        Item* pTargetItem{};
        if (!GetDataFromBlackboard(pBlackboard, "TargetItem", pTargetItem))
            return false;

        // Specific checks for pistol and shotgun slots
        if (pTargetItem->itemInfo.Type == eItemType::PISTOL)
        {
            if (pInventoryManager->IsPistolSlotEmpty())
            {
                std::cout << "Pistol slot is empty\n";
                return true;
            }
            std::cout << "Pistol slot is not empty\n";
            return false;
        }
        else if (pTargetItem->itemInfo.Type == eItemType::SHOTGUN)
        {
            if (pInventoryManager->IsShotgunSlotEmpty())
            {
                std::cout << "Shotgun slot is empty\n";
                return true;
            }
            std::cout << "Shotgun slot is not empty\n";
            return false;
        }

        // For other items, find the first empty slot excluding reserved slots
        int emptySlotIndex = pInventoryManager->FindFirstEmptySlotExcludingReserved();
        if (emptySlotIndex != -1)
        {
            std::cout << "Found an empty slot for items\n";
            return true;
        }

        std::cout << "No empty slots available\n";
        return false;
    }
    bool IsMedkitNeeded(Elite::Blackboard* pBlackboard)
    {
        ZombieGame::InventoryManager* pInventoryManager{};
        if (!GetDataFromBlackboard(pBlackboard, "InventoryManager", pInventoryManager))
            return false;

        AgentInfo* pAgentInfo{};
        if (!GetDataFromBlackboard(pBlackboard, "AgentInfo", pAgentInfo))
            return false;

        const float healthThreshold = 8.0f;
        return pAgentInfo->Health < healthThreshold;
    }
    bool IsFoodNeeded(Elite::Blackboard* pBlackboard)
    {
        ZombieGame::InventoryManager* pInventoryManager{};
        if (!GetDataFromBlackboard(pBlackboard, "InventoryManager", pInventoryManager))
            return false;

        AgentInfo* pAgentInfo{};
        if (!GetDataFromBlackboard(pBlackboard, "AgentInfo", pAgentInfo))
            return false;

        const float staminaThreshold = 7.0f;
        return pAgentInfo->Energy < staminaThreshold;
    }

    bool HasMedkit(Elite::Blackboard* pBlackboard)
    {
        ZombieGame::InventoryManager* pInventoryManager{};
        return GetDataFromBlackboard(pBlackboard, "InventoryManager", pInventoryManager) &&
            pInventoryManager->HasMedkitInInventory();
    }
    bool HasFood(Elite::Blackboard* pBlackboard)
    {
        ZombieGame::InventoryManager* pInventoryManager{};
        return GetDataFromBlackboard(pBlackboard, "InventoryManager", pInventoryManager) &&
            pInventoryManager->HasFoodInInventory();
    }
    bool HasPistol(Elite::Blackboard* pBlackboard)
    {
        ZombieGame::InventoryManager* pInventoryManager{};
        return GetDataFromBlackboard(pBlackboard, "InventoryManager", pInventoryManager) &&
            pInventoryManager->HasPistol();
    }
    bool HasShotgun(Elite::Blackboard* pBlackboard)
    {
        ZombieGame::InventoryManager* pInventoryManager{};
        return GetDataFromBlackboard(pBlackboard, "InventoryManager", pInventoryManager) &&
            pInventoryManager->HasShotgun();
    }
    bool HasNoWeapon(Elite::Blackboard* pBlackboard)
    {
        return (!HasPistol(pBlackboard) && !HasShotgun(pBlackboard));
    }
    bool ShouldItemBeReplaced(Elite::Blackboard* pBlackboard)
    {
        ZombieGame::InventoryManager* pInventoryManager{};
        Item* pTargetItem{};
        return GetDataFromBlackboard(pBlackboard, "InventoryManager", pInventoryManager) &&
            GetDataFromBlackboard(pBlackboard, "TargetItem", pTargetItem) &&
            pInventoryManager->ShouldItemBeReplaced(pTargetItem->itemInfo);
    }

    // Enemy Conditions

    bool IsEnemyInFOV(Elite::Blackboard* pBlackboard)
    {
        IExamInterface* pInterface{};
        return GetDataFromBlackboard(pBlackboard, "Interface", pInterface) &&
            pInterface->FOV_GetStats().NumEnemies > 0;
    }
    bool IsAimingFinished(Elite::Blackboard* pBlackboard)
    {
        Elite::Vector2* pTarget{};
        if (!GetDataFromBlackboard(pBlackboard, "Target", pTarget))
            return false;

        AgentInfo* pAgentInfo{};
        if ( !GetDataFromBlackboard(pBlackboard, "AgentInfo", pAgentInfo))
            return false;

        // check if if agent faces the enemy , treshholdfor floating point errors
        const float currentOrientation = pAgentInfo->Orientation;
        const float desiredOrientation = Elite::VectorToOrientation(*pTarget - pAgentInfo->Position);
        const float aimThreshold = 0.05f;

        return std::abs(currentOrientation - desiredOrientation) < aimThreshold;
    }

    // Purge zone

    bool IsInPurgeZone(Elite::Blackboard* pBlackboard)
    {
        ZombieGame::EntityManager* pEntityManager{};
        AgentInfo* pAgentInfo{};

        return GetDataFromBlackboard(pBlackboard, "EntityManager", pEntityManager) &&
            GetDataFromBlackboard(pBlackboard, "AgentInfo", pAgentInfo) &&
            pEntityManager->IsAgentInPurgeZone(pAgentInfo->Position);
    }
}
#endif