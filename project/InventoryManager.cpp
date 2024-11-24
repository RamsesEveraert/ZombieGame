#include "stdafx.h"
#include "InventoryManager.h"

#include "HouseManager.h"

ZombieGame::InventoryManager::InventoryManager(Elite::Blackboard* pBlackboard)
{
    if (!pBlackboard)
        throw std::runtime_error("InventoryManager constructor received a null blackboard.");

    // Get Interface
    bool successInterface = pBlackboard->GetData("Interface", m_pInterface);

    if (!successInterface || !m_pInterface)
        throw std::runtime_error("Interface isn't stored correctly in Blackboard - InventoryManager.");

    // Get HouseManager
    bool successHouseManager = pBlackboard->GetData("HouseManager", m_pHouseManager);

    if (!successHouseManager || !m_pHouseManager)
        throw std::runtime_error("HouseManager isn't stored correctly in Blackboard - InventoryManager.");

    // Initialize Inventory with empty items
    UINT maxInventoryItems = m_pInterface->Inventory_GetCapacity();
    m_Inventory.reserve(maxInventoryItems);
    for (UINT index = 0; index < maxInventoryItems; ++index)
    {
        m_Inventory.push_back(std::make_pair(index, InventoryItemType::Empty));
    }
}


void ZombieGame::InventoryManager::Update(float dt)
{
    // Only update when item is in fov
   if (m_pInterface->FOV_GetStats().NumItems > 0)
    {
       auto vItemsInFOV = m_pInterface->GetItemsInFOV();
       for (auto& house : vItemsInFOV)
       {
           UpdateItemsInView();
       }
    }
   ResetVisitedItems(dt);
   
}

const std::vector<std::unique_ptr<Item>>& ZombieGame::InventoryManager::GetStoredItems() const
{
    return m_StoredItems;
}

bool ZombieGame::InventoryManager::CanVisitItemInFOV()
{
    const auto& vIntemsInFOV = m_pInterface->GetItemsInFOV();
    for (const auto& fovItem : vIntemsInFOV)
    {
        for (const auto& storedItem : m_StoredItems)
        {
            if (storedItem && storedItem->itemInfo.Location == fovItem.Location && storedItem->IsVisited == false)
            {
                return true;
            }
        }
    }

    return false;
}

Item* ZombieGame::InventoryManager::GetStoredItem(const ItemInfo& itemInfo)
{
    for (auto& item : m_StoredItems)
    {
        if (item && item->itemInfo.Location == itemInfo.Location && item->itemInfo.Type == itemInfo.Type)
        {
            return item.get();
        }
    }
    return nullptr;
}

Item* ZombieGame::InventoryManager::GetClosestUnvisitedItem(const Elite::Vector2& agentPosition) const
{
    Item* closestItem = nullptr;
    float closestDistSq = FLT_MAX;

    for (const auto& item : m_StoredItems) {
        if (item->IsVisited) continue;

        float distSq = Elite::DistanceSquared(item->itemInfo.Location, agentPosition);
        if (distSq < closestDistSq) {
            closestDistSq = distSq;
            closestItem = item.get();
        }
    }

    return closestItem;
}


void ZombieGame::InventoryManager::UpdateItemsInView()
{
    auto itemsInView = m_pInterface->GetItemsInFOV();

    for (const auto& itemInView : itemsInView)
    {
        auto compareItem = [&](const std::unique_ptr<Item>& pItemPtr) -> bool
            {
                return pItemPtr->entityInfo.Location == itemInView.Location;
            };

        auto iterator = std::find_if(m_StoredItems.begin(), m_StoredItems.end(), compareItem);

        if (iterator == m_StoredItems.end())
        {
            CheckAndUpdateNewItem(itemInView);
        }
        else
        {
            UpdateExistingItem(iterator, itemInView);
        }
    }
}

void ZombieGame::InventoryManager::CheckAndUpdateNewItem(const ItemInfo& itemInView)
{
    std::cout << "New item\n";
    auto newItem = std::make_unique<Item>();
    newItem->entityInfo.Type = eEntityType::ITEM;
    newItem->entityInfo.Location = itemInView.Location;
    newItem->itemInfo = itemInView;

    CheckItemInHouse(newItem);

    m_StoredItems.emplace_back(std::move(newItem));
}

void ZombieGame::InventoryManager::UpdateExistingItem(std::vector<std::unique_ptr<Item>>::iterator& iterator, const ItemInfo& itemInView)
{
    (*iterator)->entityInfo = { eEntityType::ITEM, itemInView.Location };
    (*iterator)->itemInfo = itemInView;
}


void ZombieGame::InventoryManager::CheckItemInHouse(std::unique_ptr<Item>& newItem)
{
    for (auto& house : m_pHouseManager->GetStoredHouses())
    {
        Elite::Vector2 distance = house->Center - newItem->entityInfo.Location;

        if (abs(distance.x) < house->Size.x * 0.5f && abs(distance.y) < house->Size.y * 0.5f)
        {
            newItem->IsVisited = false;
            newItem->pHouse = house.get();
            break;
        }
    }
}

InventoryItemType ZombieGame::InventoryManager::GetLeastNeededItemType() const
{
    std::unordered_map<InventoryItemType, int> itemCounts;

    // Count each item type
    for (const auto& inventoryPair : m_Inventory)
    {
        ++itemCounts[inventoryPair.second];
    }

    // Find the item type with the highest count
    InventoryItemType leastNeededType = InventoryItemType::Empty;
    int highestCount = 0;

    for (const auto& pair : itemCounts)
    {
        if (pair.second > highestCount)
        {
            highestCount = pair.second;
            leastNeededType = pair.first;
        }
    }

    return leastNeededType;
}

bool ZombieGame::InventoryManager::ShouldItemBeReplaced(const ItemInfo& newItem) const
{
    // check for each itemslot in Inventory
    for (const auto& inventorySlot : m_Inventory)
    {
        // Obtain current item information in this slot
        ItemInfo currentInventoryItem;

        if (m_pInterface->Inventory_GetItem(inventorySlot.first, currentInventoryItem))
        {
            // check value for the same itemtypes
            if (static_cast<int>(inventorySlot.second) == static_cast<int>(newItem.Type))
            {
                // if new value is more than old value -> replace!
                if (newItem.Value > currentInventoryItem.Value)
                {
                    return true;
                }
            }
        }
  
    }

    return false;
}

bool ZombieGame::InventoryManager::HasInventoryEmptySlot()const
{
    for (const auto& inventorySlot : m_Inventory)
    {
        if (inventorySlot.second == InventoryItemType::Empty)
        {
            // empty slot found
            return true;
        }
    }

    // no empty slot found
    return false;
}

int ZombieGame::InventoryManager::GetSlotWithLowestValue(std::function<bool(const ItemInfo&)> itemCondition) const
{
    int slotWithLowestValue = -1; // Invalid slotIdx
    int lowestValue = INT_MAX;    

    // Check each slot in the inventory
    for (const auto& inventorySlot : m_Inventory)
    {
        ItemInfo currentInventoryItem;
        if (m_pInterface->Inventory_GetItem(inventorySlot.first, currentInventoryItem))
        {
            // Check if the current item meets the condition & has a lower value
            if (itemCondition(currentInventoryItem) && currentInventoryItem.Value < lowestValue)
            {
                // Update the lowest value and slot number
                lowestValue = currentInventoryItem.Value;
                slotWithLowestValue = inventorySlot.first;
            }
        }
        
    }

    return slotWithLowestValue;
}

void ZombieGame::InventoryManager::ResetVisitedItems(float dt)
{
    for (auto& pItem : m_StoredItems)
    {
        if (!pItem->IsVisited) continue;

        pItem->TimeSinceVisit += dt;
        if (pItem->TimeSinceVisit > m_ResetTimer)
        {
            ResetItem(pItem);
        }
    }
}

void ZombieGame::InventoryManager::ResetItem(std::unique_ptr<Item>& pItem) const
{
    pItem->IsVisited = false;
    pItem->TimeSinceVisit = 0.f;
}

int ZombieGame::InventoryManager::GetSlotForItemToReplace(const ItemInfo& itemToReplace) const
{
    auto replaceCondition // Lamda function
    {
        [&](const ItemInfo& item) {
        return static_cast<int>(item.Type) == static_cast<int>(itemToReplace.Type);
        }
    };

    return GetSlotWithLowestValue(replaceCondition);
}

int ZombieGame::InventoryManager::GetSlotForTheWorstItem(const ItemInfo& newItem) const
{
    InventoryItemType leastNeededItemType = GetLeastNeededItemType();

    auto replaceCondition // Lamda function
    {
        [&](const ItemInfo& item) 
        {
            return static_cast<int>(item.Type) == static_cast<int>(leastNeededItemType);
        }
    };

    return GetSlotWithLowestValue(replaceCondition);
}

int ZombieGame::InventoryManager::FindFirstEmptySlot() const
{
    // Check each slot in the inventory
    for (const auto& inventorySlot : m_Inventory)
    {
        // Check if the slot is empty
        if (inventorySlot.second == InventoryItemType::Empty)
        {
            // Return the index of the empty slot
            return static_cast<int>(inventorySlot.first);  
        }
    }
    return -1;  // Return -1 if no empty slot is found
}

void ZombieGame::InventoryManager::UpdateInventorySlot(int slotIndex, const ItemInfo& newItem)
{
    if (slotIndex >= 0 && slotIndex < static_cast<int>(m_Inventory.size()))
    {
        m_Inventory[slotIndex].second = static_cast<InventoryItemType>(newItem.Type);
    }
    else
    {
        std::cout << "Invalid inventory slot index: " << slotIndex << std::endl;
    }
}

int ZombieGame::InventoryManager::FindFirstEmptySlotExcludingReserved() const
{
    for (int i = 2; i < static_cast<int>(m_Inventory.size()); ++i)
    {
        if (m_Inventory[i].second == InventoryItemType::Empty)
        {
            return i;
        }
    }
    return -1; // Return -1 if no non-reserved empty slot is found
}

InventoryItemType ZombieGame::InventoryManager::GetInventoryItem(int slotIndex) const
{
    if (slotIndex >= 0 && slotIndex < static_cast<int>(m_Inventory.size()))
    {
        return m_Inventory[slotIndex].second;
    }
    else
    {
        std::cout << "Invalid inventory slot index: " << slotIndex << std::endl;
        return InventoryItemType::Empty; // Return empty if invalid index
    }
}

bool ZombieGame::InventoryManager::UseMedkit(float healthThreshold, float agentHealth)
{
    // check if Agent's health is below threshold
    if (agentHealth < healthThreshold)
    {
        // Calculate the amount of health needed to reach maximum
        const float neededHealth = 10 - agentHealth;

        int bestMedkitSlot = -1;
        int bestMedkitValue = INT_MAX;
        bool foundBetterFit = false;

        for (auto& inventorySlot : m_Inventory)
        {
            // Only interessed in Medkits
            if (inventorySlot.second != InventoryItemType::Medkit) continue;

            ItemInfo medKit;
            if (m_pInterface->Inventory_GetItem(inventorySlot.first, medKit))
            {
                // Better match than previously found
                if (medKit.Value >= neededHealth && medKit.Value < bestMedkitValue)
                {
                    bestMedkitSlot = inventorySlot.first;
                    bestMedkitValue = medKit.Value;
                    foundBetterFit = true;
                }
                else if (!foundBetterFit && medKit.Value < bestMedkitValue)
                {
                    // Lowest value but doesn't meet the needed health
                    bestMedkitSlot = inventorySlot.first;
                    bestMedkitValue = medKit.Value;
                }
            }
        }

        // Medkit is found
        if (bestMedkitSlot != -1)
        {
            ItemInfo medKitAfterUse;
            // Use the selected medkit
            m_pInterface->Inventory_UseItem(bestMedkitSlot);
            // Check the state of the medkit after use
            if (m_pInterface->Inventory_GetItem(bestMedkitSlot, medKitAfterUse))
            {
                // If the medkit is depleted, remove it from the inventory
                if (medKitAfterUse.Value == 0)
                {
                    m_Inventory[bestMedkitSlot].second = InventoryItemType::Empty;
                    m_pInterface->Inventory_RemoveItem(bestMedkitSlot);
                }
                return true; 
            }
        }
    }
    return false;
}

bool ZombieGame::InventoryManager::HasMedkitInInventory() const
{
    for (const auto& inventorySlot : m_Inventory)
    {
        if (inventorySlot.second == InventoryItemType::Medkit)
        {
            return true;
        }
    }
    return false;
}

bool ZombieGame::InventoryManager::HasFoodInInventory() const
{
    for (const auto& inventorySlot : m_Inventory)
    {
        if (inventorySlot.second == InventoryItemType::Food)
        {
            return true;
        }
    }
    return false;
}

bool ZombieGame::InventoryManager::UseFood(float staminaThreshold, float agentEnergy)
{
    if (agentEnergy < staminaThreshold)
    {
        const float neededEnergy = 10 - agentEnergy;

        int bestFoodSlot = -1;
        int bestFoodValue = INT_MAX;
        bool foundBetterFit = false;

        for (auto& inventorySlot : m_Inventory)
        {
            if (inventorySlot.second != InventoryItemType::Food) continue;

            ItemInfo foodItem;
            if (m_pInterface->Inventory_GetItem(inventorySlot.first, foodItem))
            {
                if (foodItem.Value >= neededEnergy && foodItem.Value < bestFoodValue)
                {
                    bestFoodSlot = inventorySlot.first;
                    bestFoodValue = foodItem.Value;
                    foundBetterFit = true;
                }
                else if (!foundBetterFit && foodItem.Value < bestFoodValue)
                {
                    bestFoodSlot = inventorySlot.first;
                    bestFoodValue = foodItem.Value;
                }
            }

        }

        if (bestFoodSlot != -1)
        {
            m_pInterface->Inventory_UseItem(bestFoodSlot);
            // Check if the food is depleted after use
            ItemInfo foodItemAfterUse;
            m_pInterface->Inventory_GetItem(bestFoodSlot, foodItemAfterUse);
            if (foodItemAfterUse.Value == 0)
            {
                // Remove the used food item from the inventory
                m_Inventory[bestFoodSlot].second = InventoryItemType::Empty;
                m_pInterface->Inventory_RemoveItem(bestFoodSlot);
            }
            return true;
        }
    }
    return false;
}

bool ZombieGame::InventoryManager::HasPistol() const
{
    for (const auto& inventorySlot : m_Inventory)
    {
        if (inventorySlot.second == InventoryItemType::Pistol)
        {
            return true;
        }
    }
    return false;
}

bool ZombieGame::InventoryManager::HasShotgun() const
{
    for (const auto& inventorySlot : m_Inventory)
    {
        if (inventorySlot.second == InventoryItemType::Shotgun)
        {
            return true;
        }
    }
    return false;
}

int ZombieGame::InventoryManager::GetBestWeaponSlotIndex(const AgentInfo& agentInfo, const Elite::Vector2& target)
{
    int bestSlotIndex = -1; // Initialize with an invalid index
    int leastAmmo = INT_MAX;
    const float distanceToTarget = agentInfo.Position.DistanceSquared(target);

    for (const auto& inventoryPair : m_Inventory)
    {
        ItemInfo inventoryItem{};
        int currentAmmo{};
        if (m_pInterface->Inventory_GetItem(inventoryPair.first, inventoryItem))
        {
            // Determine the best weapon to use
            if (inventoryPair.second == InventoryItemType::Pistol && currentAmmo < leastAmmo)
            {
                bestSlotIndex = inventoryPair.first;
                leastAmmo = currentAmmo;
            }
            else if (inventoryPair.second == InventoryItemType::Shotgun)
            {
                const float squaredFOVRange = agentInfo.FOV_Range * agentInfo.FOV_Range;
                if (distanceToTarget < 0.7f * squaredFOVRange && distanceToTarget > 0.2f)
                {
                    bestSlotIndex = inventoryPair.first;
                    break; // Found the best weapon, no need to continue
                }
            }
        }

    }

    return bestSlotIndex;
}

void ZombieGame::InventoryManager::EmptyInventorySlot(int slotIndex)
{
    if (slotIndex >= 0 && static_cast<size_t>(slotIndex) < m_Inventory.size())
    {
        // Remove from interface
        m_pInterface->Inventory_RemoveItem(slotIndex);
        // Update inventory
        m_Inventory[slotIndex] = std::make_pair(slotIndex, InventoryItemType::Empty);
    }
}

int ZombieGame::InventoryManager::CountItemType(const ItemInfo& checkItem) const
{
    int count = 0;
    for (const auto& inventorySlot : m_Inventory)
    {
        if (static_cast<int>(inventorySlot.second) == static_cast<int>(checkItem.Type))
        {
            ++count;
        }
    }
    return count;
}

int ZombieGame::InventoryManager::GetPistolSLot() const
{
    return m_PistolSlot;
}

int ZombieGame::InventoryManager::GetShotgunSlot() const
{
    return m_ShotgunSlot;
}
bool ZombieGame::InventoryManager::IsPistolSlotEmpty() const
{
    return m_Inventory[m_PistolSlot].second == InventoryItemType::Empty;
}

bool ZombieGame::InventoryManager::IsShotgunSlotEmpty() const
{
     return m_Inventory[m_ShotgunSlot].second == InventoryItemType::Empty;
}
