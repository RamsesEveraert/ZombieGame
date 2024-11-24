#include "stdafx.h"
#include "EntityManager.h"

ZombieGame::EntityManager::EntityManager(Elite::Blackboard* pBlackboard)
{
	bool success = pBlackboard->GetData("Interface", m_pInterface);
	assert(success && m_pInterface && "Interface isn't stored correctly in Blackboard - EntityManager");
}

void ZombieGame::EntityManager::Update(float dt)
{
	// Update PurgeZones
	if (m_pInterface->FOV_GetStats().NumPurgeZones > 0)
	{
		UpdatePurgeZones(dt);
	}

	//Update Enemies
	if (m_pInterface->FOV_GetStats().NumEnemies > 0)
	{
		UpdateEnemies(dt);
	}
}

const std::vector<std::unique_ptr<EnemyInfo>>& ZombieGame::EntityManager::GetStoredEnemies() const
{
	return m_Enemies;
}

EnemyInfo* ZombieGame::EntityManager::GetStoredEnemy(const EnemyInfo& enemyInfo)
{
	for (auto& enemy : m_Enemies)
	{
		if (enemy && enemy->EnemyHash == enemyInfo.EnemyHash)
		{
			return enemy.get();
		}
	}
	return nullptr;
}

void ZombieGame::EntityManager::UpdatePurgeZones(float dt)
{
	for (auto& purgeZone : m_pInterface->GetPurgeZonesInFOV())
	{
		// focus only on enemies inFOV :reset and recalculate purgezones in fov
		/*m_PurgeZones.clear();*/

		auto comparePurgeZone = [&](const std::unique_ptr<PurgeZone>& pStoredPurgeZone) -> bool {
			return pStoredPurgeZone->Center == purgeZone.Center;
			};

		// check if current purgezone isn't stored
		if (std::find_if(m_PurgeZones.begin(), m_PurgeZones.end(), comparePurgeZone) == m_PurgeZones.end())
		{
			std::cout << "New Purge Zone stored\n";

			auto pPurgeZone = std::make_unique<PurgeZone>();
			pPurgeZone->Center = purgeZone.Center;
			pPurgeZone->Radius = purgeZone.Radius + 10.f;
			pPurgeZone->ZoneHash = purgeZone.ZoneHash;
			pPurgeZone->EstimatedLifeTime = 8.f; // max time of purge zone
			m_PurgeZones.emplace_back(std::move(pPurgeZone));
		}
	}

	// check when agent can continue after seeing purge zone
	UpdatePurgeZoneTime(dt);
}

void ZombieGame::EntityManager::UpdatePurgeZoneTime(float dt)
{
	// count down life time of purgezones
	for (auto& pPurgeZone : m_PurgeZones)
	{
		pPurgeZone->EstimatedLifeTime -= dt;
	}

	// remove purgezone when it dies
	auto purgeZoneRemover = [&](const std::unique_ptr<PurgeZone>& pPurgeZone) -> bool 
	{
		return pPurgeZone->EstimatedLifeTime <= 0.f;
	};

	auto iterator = std::remove_if(m_PurgeZones.begin(), m_PurgeZones.end(), purgeZoneRemover);

	if (iterator != m_PurgeZones.end())
	{
		m_PurgeZones.erase(iterator, m_PurgeZones.end());
	}
}

void ZombieGame::EntityManager::UpdateEnemies(float dt)
{
	// focus only on enemies inFOV :reset and recalculate enemiesinfov
	m_Enemies.clear();

	for (auto& enemyInfo : m_pInterface->GetEnemiesInFOV())
	{
		auto uniqueEnemy = std::make_unique<EnemyInfo>(enemyInfo);
		m_Enemies.emplace_back(std::move(uniqueEnemy));
	}
}

EnemyInfo* ZombieGame::EntityManager::SetClosestEnemyAsTarget(const Elite::Vector2& agentPosition)
{
	EnemyInfo* closestStoredEnemy = nullptr;
	float closestDistanceSquared = FLT_MAX;

	// Get enemies from FOV
	auto enemiesInFOV = m_pInterface->GetEnemiesInFOV();

	// Find closest enemy
	for (const auto& storedEnemy : m_Enemies)
	{
		for (const auto& enemyInFOV : enemiesInFOV)
		{
			// Check if stored enemy is the same as the enemy in FOV
			if (storedEnemy && storedEnemy->EnemyHash == enemyInFOV.EnemyHash)
			{
				float distanceSquared = Elite::DistanceSquared(enemyInFOV.Location, agentPosition);
				if (distanceSquared < closestDistanceSquared)
				{
					closestDistanceSquared = distanceSquared;
					closestStoredEnemy = storedEnemy.get();
				}
			}
		}
	}

	return closestStoredEnemy; 
}

bool ZombieGame::EntityManager::IsAgentInPurgeZone(const Elite::Vector2& agentPosition) const
{
	if (m_pInterface->FOV_GetStats().NumPurgeZones > 0)
	{
		float safeDistance{ 35.f };
		for (const auto& purgeZone : m_pInterface->GetPurgeZonesInFOV())
		{
				const float squaredDistance = Elite::DistanceSquared(agentPosition, purgeZone.Center);
				if (squaredDistance - safeDistance < purgeZone.Radius * purgeZone.Radius)
				{
					return true;
				}
			
		}
	}
	
	return false;
}

PurgeZone* ZombieGame::EntityManager::GetClosestPurgeZone(const Elite::Vector2& position) const
{
	PurgeZone* pClosestPurgeZone = nullptr;
	float closestDistSq = INFINITY;

	for (const auto& purgeZone : m_PurgeZones)
	{
		if (purgeZone != nullptr)
		{
			float distSq = Elite::DistanceSquared(position, purgeZone->Center);

			if (distSq < closestDistSq)
			{
				closestDistSq = distSq;
				pClosestPurgeZone = purgeZone.get();
			}
		}
	}

	return pClosestPurgeZone;
}