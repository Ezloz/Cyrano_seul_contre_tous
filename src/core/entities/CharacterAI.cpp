#include "entities/Character.h"
#include "entities/Soldier.h"
#include <queue>
#include <unordered_set>
#include <unordered_map>
#include <ranges>
#include <algorithm>


struct CoordHash {
    size_t operator()(const Coord& c) const {
        return std::hash<int>()(c.x) ^ (std::hash<int>()(c.y) << 1);
    }
};

std::vector<Coord> BFSPath(const std::vector<Coord>& moveRange,
                           const Coord& start,
                           const Coord& target)
{
    std::unordered_set<Coord, CoordHash> walkable(moveRange.begin(), moveRange.end());

    if (!walkable.count(start) || !walkable.count(target))
        return {};

    if (start == target)
        return {start};

    std::queue<Coord> q;
    std::unordered_map<Coord, Coord, CoordHash> parent;
    std::unordered_set<Coord, CoordHash> visited;

    auto push = [&](const Coord& from, const Coord& to) {
        if (walkable.count(to) && !visited.count(to)) {
            visited.insert(to);
            parent[to] = from;
            q.push(to);
        }
    };

    q.push(start);
    visited.insert(start);

    const int dx[4] = {1, -1, 0, 0};
    const int dy[4] = {0, 0, 1, -1};

    bool found = false;

    while (!q.empty() && !found) {
        Coord cur = q.front();
        q.pop();

        for (int i = 0; i < 4; i++) {
            Coord nxt{cur.x + dx[i], cur.y + dy[i]};

            if (visited.count(nxt) || !walkable.count(nxt))
                continue;

            parent[nxt] = cur;

            if (nxt == target) {
                found = true;
                break;
            }

            visited.insert(nxt);
            q.push(nxt);
        }
    }

    if (!found)
        return {}; // no path

    // Reconstruct path
    std::vector<Coord> path;
    for (Coord cur = target; ; cur = parent[cur]) {
        path.push_back(cur);
        if (cur == start) break;
    }

    std::reverse(path.begin(), path.end());
    return path;
}


std::vector<Coord> simplePath(const std::vector<Coord>& moveRange, const Coord& start, const Coord& target)
{
    if (start == target || moveRange.empty()){
        return {};
    }
    auto it = std::ranges::min_element(moveRange,
    [&](const Coord& a, const Coord& b) {
        return manhattanDistance(a, target) < manhattanDistance(b, target);
    });

  Coord closestCoord = *it;

  auto moveRangeAndStart = moveRange;
  moveRangeAndStart.push_back(start);

  return BFSPath(moveRangeAndStart, start, closestCoord);

}



Coord FindNearestPlayer(Coord origin, const std::vector<std::unique_ptr<Character>>& characters){
    bool found = false;
    int bestDist = 0;
    Coord bestCoord = origin;

    for (const auto& c : characters)
    {
        if (!c || !c->isPlayer())
            continue;

        Coord p = c->getCoord();

        int dist = manhattanDistance(origin, p);

        if (!found || dist < bestDist)
        {
            found = true;
            bestDist = dist;
            bestCoord = p;
        }
    }

    return bestCoord; // returns origin if no player found
}


std::pair<Action, std::vector<Coord>> Soldier::workAI(const std::vector<size_t>& walkableGrid, const int gridWidth, const int gridHeight,
                     const std::vector<std::unique_ptr<Character>>& characters){

    if (this->workFinished || this->getOccupied()){
        return {Action::NOTHING,  {}};
    }

    this->usedAV = 50.0f;
    auto player = FindNearestPlayer(this->getCoord(), characters);
    if (manhattanDistance(this->getCoord(), player) == 1){
        this->workFinished = true;
        return {Action::ATTACK, {player}};
    }

    std::vector<Coord> moveRange = this->calculateMoveRange(walkableGrid, gridWidth, gridHeight, characters);
    std::vector<Coord> path = simplePath(moveRange, this->coord, FindNearestPlayer(this->coord, characters));
    float case_av = 100.0f / this->getStats().speed; // TO REWORK : Take tile propreties into account (not implemented yet)
    if (!path.empty()){
        this->workFinished = true;
        this->usedAV = (case_av * path.size());
    }
    return {Action::MOVE, path};
}