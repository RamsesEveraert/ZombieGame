#pragma once
#include "stdafx.h"
#include "EliteData/EBlackboard.h"
#include "IExamInterface.h"

namespace ZombieGame
{
	class EntityManager
	{
	public:
		EntityManager(Elite::Blackboard* pBlackboard);
		void Update(float dt);

		// Getters
		const std::vector<std::unique_ptr<EnemyInfo>>& GetStoredEnemies() const;
		EnemyInfo* GetStoredEnemy(const EnemyInfo& enemyInfo);
		PurgeZone* GetClosestPurgeZone(const Elite::Vector2& position) const;

		// Behavior Functions
		EnemyInfo* SetClosestEnemyAsTarget(const Elite::Vector2& agentPosition);
		bool IsAgentInPurgeZone(const Elite::Vector2& agentPosition) const;

	private:
		// Member variables
		IExamInterface* m_pInterface{};
		std::vector<std::unique_ptr<EnemyInfo>> m_Enemies;
		std::vector<std::unique_ptr<PurgeZone>> m_PurgeZones;

	private:
		// Update Functions
		void UpdatePurgeZones(float dt);
		void UpdateEnemies(float dt);
		void UpdatePurgeZoneTime(float dt);
	};
}
