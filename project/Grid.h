#pragma once
#include "EliteBehaviorTree/EDecisionMaking.h"

#include <vector>
#include <memory> // smartpointers

struct Cell
{
	Elite::Vector2 Position{};
	bool IsVisited{ false };
    std::vector<Cell*> pNeighbors{}; // Cells are already owned
    float Influence{};
};

class IExamInterface;
struct AgentInfo;

namespace ZombieGame
{
    class Grid final
    {
    public:
        Grid(Elite::Blackboard* pBlackboard, int cellDimension);
        virtual ~Grid() = default;

        void RenderGrid() const;
        void UpdateCurrentAgentCell(AgentInfo* agent);

        // Getters
        Cell* Grid::GetExpandSquareSearchCell(const AgentInfo* agent);
        const std::vector<std::unique_ptr<Cell>>& GetGridCells() const;
        Cell* GetCurrentAgentCell() const;

        // Setters
        void SetCurrentCellVisited();
        void MarkCellVisited(const Elite::Vector2& position);
        void StoreLastVisitedCell();



    private:
        // Main variables
        IExamInterface* m_pInterface{};
        Elite::Blackboard* m_pBlackboard{};

        std::vector<std::unique_ptr<Cell>> m_GridCells{};
        std::vector<Cell*> m_VisitedCells{};
        std::vector<Cell*> m_UnVisitedCells{};

        Elite::Vector2 m_GridStartPosition{};

        // Cell variables
        Cell* m_pCurrentAgentCell{};
        float m_CellSize{};
        float m_HalfCellSize{};
        int m_CellDimensions{};

        // Neighbor variables
        float m_NeighborInfluenceBoost{0.5f};
        float m_VisitDistanceThreshold{ 10.f };

        // Expanding Square Search variables
        int m_CurrentSideLength{ 1 }; // Start with a side length of 1
        int m_StepsTaken{ 0 };
        enum class Direction 
        {   
            RIGHT,
            UP,
            LEFT, 
            DOWN 
        };

        Direction m_CurrentDirection{ Direction::RIGHT };

        Cell* m_pLastVisitedCell{ nullptr };
        Cell* m_pLastCellBeforeDeviation{ nullptr };
        std::queue<Cell*> m_LegQueue; // Queue to store the path for the current leg

    private:

        // Initialization functions
        void InitializeGrid();
        void InitializeNeighbors();

        // Render functions
        void RenderVisitedCells(const Elite::Vector3& color) const;
        void RenderCurrentCell(const Elite::Vector3& color) const;
        void RenderCell(const Cell& cell, const Elite::Vector3& color) const;

        // Helper functions
        void Grid::PositionToGridCoordinates(const Elite::Vector2& position, int& row, int& column) const;
        void UpdateCoordinatesForDirection(int& row, int& column);
        Cell* GetCellAtGridCoordinates(int row, int column);
        void PlanLeg(); // Method to plan the entire leg
        void ResumeSearchFromLastVisitedCell();
        void UpdateLegQueue();
    };

}

