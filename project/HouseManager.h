#pragma once
#include "EliteBehaviorTree/EDecisionMaking.h"
#include "IExamInterface.h"

namespace ZombieGame
{
	class HouseManager final
	{
	public:
		HouseManager(Elite::Blackboard* pBlackboard);
		~HouseManager() = default;

		void Update(float dt);
		void Render() const;

		// Checks
		bool CanVisitHouseInFOV();

		// Setters
		void MarkHouseAsVisited(House* house);

		// Getters
		CheckPoint* GetNextCheckpoint(const Elite::Vector2& agentPosition, House* pHouse) const;
		std::vector<std::unique_ptr<House>>& GetStoredHouses();
		House* GetClosestUnvisitedHouse(const Elite::Vector2& agentPosition) const;
		House* GetHouse(const HouseInfo& houseInfo);



	private:
		IExamInterface* m_pInterface;
		Elite::Blackboard* m_pBlackboard;
		std::vector<std::unique_ptr<House>> m_StoredHouses{};

		float m_ResetTimer{ 600.f };
	private:
		// Helper Functions
		void UpdateHousesInView();
		bool IsHouseStored(const HouseInfo& house) const;
		void StoreNewHouse(const HouseInfo& houseInfo);
		std::unique_ptr<House> CreateHouse(const HouseInfo& houseInfo) const;
		void CalculateCheckPoints(std::unique_ptr<House>& pHouse) const;
		void AddCheckPointToHouse(std::unique_ptr<House>& pHouse, const Elite::Vector2& startPosition, int x, int y, float searchRange) const;
		void ResetVisitedHouses(float dt);
		void ResetHouse(std::unique_ptr<House>& pHouse) const;
		void DrawCheckPoints(const std::unique_ptr<House>& pHouse) const;
	};
}