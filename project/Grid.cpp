#include "stdafx.h"
#include "Grid.h"

#include <algorithm>
#include <cmath>

#include "IExamInterface.h"
#include "HouseManager.h"

using namespace ZombieGame;

Grid::Grid(Elite::Blackboard* pBlackboard, int cellDimension)
    : m_CellDimensions(cellDimension), m_pBlackboard(pBlackboard)
{
    // Check if the blackboard and the interface are valid
    if (!m_pBlackboard)
        throw std::runtime_error("Blackboard is null in Grid constructor.");

    if (!m_pBlackboard->GetData("Interface", m_pInterface) || !m_pInterface)
        throw std::runtime_error("Interface isn't correctly stored in blackboard.");

    // Initialize grid and neighborins
    InitializeGrid();
    InitializeNeighbors();
}

    void Grid::InitializeGrid()
    {
        // start middle of the world -> middle of the grid
        WorldInfo world = m_pInterface->World_GetInfo();
        m_GridStartPosition = world.Center - world.Dimensions * 0.5f;

        m_CellSize = world.Dimensions.x / m_CellDimensions;
        m_HalfCellSize = m_CellSize * 0.5f;

        int maxCells{ m_CellDimensions * m_CellDimensions };

        // optimization reasons
        m_GridCells.reserve(maxCells);
        m_VisitedCells.reserve(maxCells);
        m_UnVisitedCells.reserve(maxCells);

        for (int row = 0; row < m_CellDimensions; ++row)
        {
            for (int column = 0; column < m_CellDimensions; ++column)
            {
                auto pNewCell = std::make_unique<Cell>();
                pNewCell->Position = m_GridStartPosition + Elite::Vector2{ column * m_CellSize, row * m_CellSize };
                m_GridCells.emplace_back(std::move(pNewCell));                
            }
        }
    }

    void Grid::InitializeNeighbors()
    {
        for (int row = 0; row < m_CellDimensions; ++row)
        {
            for (int column = 0; column < m_CellDimensions; ++column)
            {
                Cell* currentCell = m_GridCells[row * m_CellDimensions + column].get();
                for (int i = -1; i <= 1; ++i)
                {
                    for (int j = -1; j <= 1; ++j)
                    {
                        if (i == 0 && j == 0) continue; // Skip the current cell
                        int neighborRow = row + i;
                        int neighborColumn = column + j;
                        if (neighborRow >= 0 && neighborRow < m_CellDimensions && neighborColumn >= 0 && neighborColumn < m_CellDimensions) // grid boundaries
                        {
                            Cell* neighborCell = m_GridCells[neighborRow * m_CellDimensions + neighborColumn].get();
                            currentCell->pNeighbors.push_back(neighborCell);
                        }
                    }
                }
            }
        }
    }

    void Grid::UpdateCurrentAgentCell(AgentInfo* agent)
    {

        // Convert agents position to grid coordinates
        int row, column;
        PositionToGridCoordinates(agent->Position, row, column);

        // grid boundary check
        if (row >= 0 && row < m_CellDimensions && column >= 0 && column < m_CellDimensions)
        {
            Cell* potentialAgentCell = m_GridCells[row * m_CellDimensions + column].get();

            // Check if agent is close enough to the center of the cell to mark it as visited
            Elite::Vector2 cellCenter = potentialAgentCell->Position;
            float distanceToCellCenter = Elite::Distance(agent->Position, cellCenter);

            if (distanceToCellCenter < m_VisitDistanceThreshold)
            {
                m_pCurrentAgentCell = potentialAgentCell;
                m_pCurrentAgentCell->IsVisited = true; // Mark as visited only if within threshold distance
            }
            else
            {
                m_pCurrentAgentCell = potentialAgentCell; // Update current cell but don't mark as visited
            }
        }
        else
        {
            m_pCurrentAgentCell = nullptr; // Outside grid bounds
        }
    }

    void Grid::RenderGrid() const
    {
        RenderVisitedCells(Elite::Vector3{ 0.f, 1.f, 0.0f }); // green
        RenderCurrentCell(Elite::Vector3{ 0.5f, 0.f, 0.5f }); // purple
    }

    void Grid::RenderVisitedCells(const Elite::Vector3& color) const
    {
        for (const auto& cell : m_GridCells) {
            if (cell && cell->IsVisited) {
                RenderCell(*cell, color);
            }
        }
    }

    void Grid::RenderCurrentCell(const Elite::Vector3& color) const
    {
        if (m_pCurrentAgentCell) 
        {
            RenderCell(*m_pCurrentAgentCell, color);
        }
    }

    void Grid::RenderCell(const Cell& cell, const Elite::Vector3& color) const
    {
        Elite::Vector2 topLeft = cell.Position + Elite::Vector2{ -m_HalfCellSize, -m_HalfCellSize };
        Elite::Vector2 topRight = cell.Position + Elite::Vector2{ m_HalfCellSize, -m_HalfCellSize };
        Elite::Vector2 bottomLeft = cell.Position + Elite::Vector2{ -m_HalfCellSize, m_HalfCellSize };
        Elite::Vector2 bottomRight = cell.Position + Elite::Vector2{ m_HalfCellSize, m_HalfCellSize };

        m_pInterface->Draw_Segment(topLeft, bottomLeft, color);
        m_pInterface->Draw_Segment(bottomLeft, bottomRight, color);
        m_pInterface->Draw_Segment(bottomRight, topRight, color);
        m_pInterface->Draw_Segment(topRight, topLeft, color);
    }

    void ZombieGame::Grid::SetCurrentCellVisited()
    {
        for (auto& pCell : m_GridCells)
        {
            if (m_pCurrentAgentCell == pCell.get())
            {
                pCell->IsVisited = true;
            }

        }
    }


    //https://en.wikipedia.org/wiki/Water_surface_searches#Expanding_square_search    

    Cell* Grid::GetExpandSquareSearchCell(const AgentInfo* agent)
    {
        // No lastVisitedCell = Start Expanding Square Search
        if (m_pLastVisitedCell == nullptr)
        {
            m_pLastVisitedCell = m_pCurrentAgentCell;
            m_StepsTaken = 0; // steps = moved cells
            m_CurrentSideLength = 1; // smallest side length of 1 cell
            m_CurrentDirection = Direction::RIGHT; // Start moving to the right
            PlanLeg(); // Plan the first leg
            return m_pLastVisitedCell;
        }

        // Cell has to be visited before going to the next cell
        if (!m_pLastVisitedCell->IsVisited)
        {
            return m_pLastVisitedCell;
        }

        // If the current leg is finished, plan the next leg
        if (m_LegQueue.empty())
        {
            PlanLeg();
        }

        while (!m_LegQueue.empty())
        {
            Cell* nextCell = m_LegQueue.front();
            m_LegQueue.pop();

            if (!nextCell->IsVisited)
            {
                m_pLastVisitedCell = nextCell; 
                return nextCell;
            }
            
        }

        return nullptr;
    }


    void ZombieGame::Grid::UpdateCoordinatesForDirection(int& row, int& column) {
        switch (m_CurrentDirection)
        {
        case Direction::RIGHT:
            column++;
            break;
        case Direction::DOWN:
            row++;
            break;
        case Direction::LEFT:
            column--;
            break;
        case Direction::UP:
            row--;
            break;
        }

        // Correction for grid boundaries
        column = (column + m_CellDimensions) % m_CellDimensions;
        row = (row + m_CellDimensions) % m_CellDimensions;
    }

    Cell* ZombieGame::Grid::GetCellAtGridCoordinates(int row, int column) {
        if (row < 0 || row >= m_CellDimensions || column < 0 || column >= m_CellDimensions)
            return nullptr;

        return m_GridCells[row * m_CellDimensions + column].get();
    }


    void ZombieGame::Grid::PositionToGridCoordinates(const Elite::Vector2& position, int& row, int& column) const
    {
        column = static_cast<int>(std::floor((position.x - m_GridStartPosition.x + m_HalfCellSize) / m_CellSize));
        row = static_cast<int>(std::floor((position.y - m_GridStartPosition.y + m_HalfCellSize) / m_CellSize));
    }

    const std::vector<std::unique_ptr<Cell>>& Grid::GetGridCells() const
    {
        return m_GridCells;;
    }

    Cell* Grid::GetCurrentAgentCell() const
    {
        return m_pCurrentAgentCell;
    }

    void Grid::MarkCellVisited(const Elite::Vector2& position)
    {
        // Convert the position to grid coordinates
        int row, column;
        PositionToGridCoordinates(position, row, column);

        // Check if the calculated row and column are within the grid boundaries
        if (row >= 0 && row < m_CellDimensions && column >= 0 && column < m_CellDimensions)
        {
            Cell* cell = m_GridCells[row * m_CellDimensions + column].get();
            if (cell)
            {
                cell->IsVisited = true;
            }
        }
    }

    void Grid::PlanLeg()
    {
        int row, column;
        PositionToGridCoordinates(m_pLastVisitedCell->Position, row, column);

        for (int step = 0; step < m_CurrentSideLength; ++step)
        {
            UpdateCoordinatesForDirection(row, column);
            Cell* nextCell = GetCellAtGridCoordinates(row, column);
            if (nextCell)
            {
                m_LegQueue.push(nextCell);
            }
        }

        // Change direction and update next leg length
        m_CurrentDirection = static_cast<Direction>((static_cast<int>(m_CurrentDirection) + 1) % 4);
        if (m_CurrentDirection == Direction::LEFT || m_CurrentDirection == Direction::RIGHT)
        {
            m_CurrentSideLength++;
        }
    }

    void Grid::StoreLastVisitedCell()
    {
        // Call before deviating from search: visiting houses, grabbing items...
        m_pLastCellBeforeDeviation = m_pLastVisitedCell;
    }

    void Grid::ResumeSearchFromLastVisitedCell()
    {
        if (m_pLastCellBeforeDeviation != nullptr)
        {
            // resume from the last visited cell
            m_pLastVisitedCell = m_pLastCellBeforeDeviation;
            m_pLastCellBeforeDeviation = nullptr;
            UpdateLegQueue(); // remove visited cells from que
            PlanLeg(); 
        }
    }

    void Grid::UpdateLegQueue()
    {
        std::queue<Cell*> updatedQueue;
        while (!m_LegQueue.empty())
        {
            Cell* cell = m_LegQueue.front();
            m_LegQueue.pop();
            if (!cell->IsVisited)
            {
                updatedQueue.push(cell);
            }
        }
        m_LegQueue = std::move(updatedQueue);
    }

