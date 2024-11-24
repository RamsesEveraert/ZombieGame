#pragma once
#include "EliteBehaviorTree/EDecisionMaking.h"
#include "IExamInterface.h"

namespace ZombieGame
{
	class HouseManager;
	class InventoryManager final
	{
	public:
		InventoryManager(Elite::Blackboard* pBlackboard);
		~InventoryManager() = default;

		void Update(float dt);
		void UpdateInventorySlot(int slotIndex, const ItemInfo& newItem);

		// Getters
		const std::vector<std::unique_ptr<Item>>& GetStoredItems() const;
		Item* GetStoredItem(const ItemInfo& itemInfo);
		Item* GetClosestUnvisitedItem(const Elite::Vector2& agentPosition) const;

		int FindFirstEmptySlot() const;
		int FindFirstEmptySlotExcludingReserved() const;
		int GetSlotForItemToReplace(const ItemInfo& itemToReplace) const;
		int GetSlotForTheWorstItem(const ItemInfo& itemToReplace) const;

		InventoryItemType GetInventoryItem(int slotIndex) const;
		InventoryItemType GetLeastNeededItemType() const;

		int GetBestWeaponSlotIndex(const AgentInfo& agentInfo, const Elite::Vector2& target);

		int GetPistolSLot() const;
		int GetShotgunSlot() const;

		// Checks
		bool CanVisitItemInFOV();

		bool HasMedkitInInventory() const;
		bool HasFoodInInventory() const;

		bool ShouldItemBeReplaced(const ItemInfo& newItem) const;
		bool HasInventoryEmptySlot() const;

		bool HasPistol() const;
		bool HasShotgun() const;

		bool IsPistolSlotEmpty() const;
		bool IsShotgunSlotEmpty() const;

		
		// Setters
		bool UseMedkit(float healthThreshold, float agentHealth);
		bool UseFood(float staminaThreshold, float agentStamina);
		int CountItemType(const ItemInfo& checkItem) const;
		void EmptyInventorySlot(int slotIndex);

	private:
		IExamInterface* m_pInterface{nullptr};
		ZombieGame::HouseManager* m_pHouseManager{};

		std::vector<std::pair<int, InventoryItemType>> m_Inventory{};
		std::vector<std::unique_ptr<Item>> m_StoredItems{};

		float m_ResetTimer{ 600.f };

		const int m_ShotgunSlot{ 0 };
		const int m_PistolSlot{ 1 };

	private:

		// Helper Functions
		void UpdateItemsInView();

		void CheckAndUpdateNewItem(const ItemInfo& itemInView);
		void UpdateExistingItem(std::vector<std::unique_ptr<Item>>::iterator& iterator, const ItemInfo& itemInView);
		void CheckItemInHouse(std::unique_ptr<Item>& newItem);

		int GetSlotWithLowestValue(std::function<bool(const ItemInfo&)> itemCondition) const;

		void ResetVisitedItems(float dt);
		void ResetItem(std::unique_ptr<Item>& pItem) const;
	};
}


