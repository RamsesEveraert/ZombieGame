#include "stdafx.h"
#include "HouseManager.h"
#include "EliteData/EBlackboard.h"

using namespace ZombieGame;

HouseManager::HouseManager(Elite::Blackboard* pBlackboard)
    : m_pBlackboard(pBlackboard)
{
    // Validate the blackboard input
    if (!m_pBlackboard)
        throw std::runtime_error("HouseManager constructor received a null blackboard.");

    // get interface from the blackboard
    bool success = m_pBlackboard->GetData("Interface", m_pInterface);
    if (!success || !m_pInterface)
        throw std::runtime_error("Interface isn't stored correctly in the HouseManager constructor.");

    // Reserve space for storing houses, Exam slides said it was max 20 houses
    m_StoredHouses.reserve(20);
}


void ZombieGame::HouseManager::Update(float dt)
{
    // only update when in field of view
    if (m_pInterface->FOV_GetStats().NumHouses > 0)
    {
        UpdateHousesInView();
    }
    ResetVisitedHouses(dt);
}

void ZombieGame::HouseManager::UpdateHousesInView()
{
    // Only update when in field of view
    if (m_pInterface->FOV_GetStats().NumHouses > 0)
    {
        auto vHousesInFOV = m_pInterface->GetHousesInFOV();
        for (auto& house : vHousesInFOV)
        {
            if (!IsHouseStored(house))
            {
                StoreNewHouse(house);
            }
        }
    }
}

bool HouseManager::IsHouseStored(const HouseInfo& house) const
{
    auto compareHouse = [&house](const std::unique_ptr<House>& storedHouse) { return storedHouse->Center == house.Center; };
    return std::find_if(m_StoredHouses.begin(), m_StoredHouses.end(), compareHouse) != m_StoredHouses.end();
}

void HouseManager::StoreNewHouse(const HouseInfo& houseInfo)
{
    std::cout << "New house stored!\n";

    std::unique_ptr<House> pHouse = CreateHouse(houseInfo);
    CalculateCheckPoints(pHouse);

    // Store house to revisit later
    m_StoredHouses.emplace_back(std::move(pHouse));
}



std::unique_ptr<House> HouseManager::CreateHouse(const HouseInfo& houseInfo) const
{
    std::unique_ptr<House> pHouse = std::make_unique<House>();
    pHouse->Center = houseInfo.Center;
    pHouse->Size = houseInfo.Size;
    pHouse->IsVisited = false;
    return pHouse;
}

void HouseManager::MarkHouseAsVisited(House* house)
{
    if (house)
    {
        house->IsVisited = true;
    }
}


void HouseManager::CalculateCheckPoints(std::unique_ptr<House>& pHouse) const
{
    const float searchRange = m_pInterface->Agent_GetInfo().FOV_Range ;

    // checkpoints evenly distributed
    Elite::Vector2 nrCheckPoints = pHouse->Size / searchRange;

    // there must be atleast 1 checkpoint each house
    int nrCheckPointsX = static_cast<int>(nrCheckPoints.x) > 1 ? static_cast<int>(nrCheckPoints.x) : 1;
    int nrCheckPointsY = static_cast<int>(nrCheckPoints.y) > 1 ? static_cast<int>(nrCheckPoints.y) : 1;


    Elite::Vector2 startPosition = pHouse->Center - 0.5f * Elite::Vector2{ (nrCheckPointsX / 2) * searchRange, (nrCheckPointsY / 2) * searchRange };

    for (int x = 0; x < nrCheckPointsX; ++x)
    {
        for (int y = 0; y < nrCheckPointsY; ++y)
        {
            AddCheckPointToHouse(pHouse, startPosition, x, y, searchRange);
        }
    }
}

void HouseManager::AddCheckPointToHouse(std::unique_ptr<House>& pHouse, const Elite::Vector2& startPosition, int x, int y, float searchRange) const
{
    CheckPoint* pCheckPoint = new CheckPoint;
    pCheckPoint->Position.x = startPosition.x + x * searchRange;
    pCheckPoint->Position.y = startPosition.y + y * searchRange;
    pHouse->pCheckPoints.emplace_back(pCheckPoint);

    std::cout << "New Check point\n";
}

void HouseManager::ResetVisitedHouses(float dt)
{
    for (auto& pHouse : m_StoredHouses)
    {
        if (!pHouse->IsVisited) continue;

        pHouse->TimeSinceVisit += dt;
        if (pHouse->TimeSinceVisit > m_ResetTimer)
        {
            ResetHouse(pHouse);
        }
    }
}

void HouseManager::ResetHouse(std::unique_ptr<House>& pHouse) const
{
    pHouse->IsVisited = false;
    for (CheckPoint* pCheckPoint : pHouse->pCheckPoints)
    {
        pCheckPoint->IsVisited = false;
    }
    pHouse->TimeSinceVisit = 0.f;
}

void HouseManager::Render() const
{
    for (auto& pHouse : m_StoredHouses)
    {
        DrawCheckPoints(pHouse);
    }
}

bool ZombieGame::HouseManager::CanVisitHouseInFOV()
{
    const auto& vHousesInFOV = m_pInterface->GetHousesInFOV();
    for (const auto& fovHouse : vHousesInFOV)
    {
        for (const auto& storedHouse : m_StoredHouses)
        {
            if (storedHouse && storedHouse->Center == fovHouse.Center && storedHouse->IsVisited == false)
            {
                // House matches and is not visited
                return true;
            }
        }
    }

    return false;
}

void HouseManager::DrawCheckPoints(const std::unique_ptr<House>& pHouse) const
{
    for (const CheckPoint* pCheckPoint : pHouse->pCheckPoints)
    {
        Elite::Vector3 color = pCheckPoint->IsVisited ? Elite::Vector3{ 1.f, 1.f, 1.f } : Elite::Vector3{ 1.f, 0.f, 0.f };
        float size = pCheckPoint->IsVisited ? 1.f : 2.f;
        m_pInterface->Draw_Circle(pCheckPoint->Position, size, color);
    }
}

CheckPoint* HouseManager::GetNextCheckpoint(const Elite::Vector2& agentPosition, House* pHouse) const
{
    CheckPoint* closestCheckpoint = nullptr;
    float closestDistanceSq = FLT_MAX;

    //check for all checkpoints in the given house if it is visited
    for (CheckPoint* pCheckPoint : pHouse->pCheckPoints)
    {
        // if visited: SKIP - we only want unvisited ;)
        if (pCheckPoint->IsVisited) continue;

        float distanceSq = Elite::DistanceSquared(pCheckPoint->Position, agentPosition);
        if (distanceSq < closestDistanceSq)
        {
            closestCheckpoint = pCheckPoint;
            closestDistanceSq = distanceSq;
        }
    }

    return closestCheckpoint;
}


std::vector<std::unique_ptr<House>>& ZombieGame::HouseManager::GetStoredHouses()
{
    return m_StoredHouses;
}

House* HouseManager::GetClosestUnvisitedHouse(const Elite::Vector2& agentPosition) const {
    House* closestHouse = nullptr;
    float closestDistSq = FLT_MAX;

    for (const auto& house : m_StoredHouses) {
        if (house->IsVisited) continue;

        float distSq = Elite::DistanceSquared(house->Center, agentPosition);
        if (distSq < closestDistSq) {
            closestDistSq = distSq;
            closestHouse = house.get();
        }
    }

    return closestHouse;
}

House* ZombieGame::HouseManager::GetHouse(const HouseInfo& houseInfo)
{
    for (auto& house : m_StoredHouses)
    {
        if (house && house->Center == houseInfo.Center && house->Size == houseInfo.Size)
        {
            return house.get();
        }
    }
    return nullptr;
}