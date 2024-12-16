#include "challenge16.hpp"

#include "helper.hpp"
#include "print.hpp"

#include <algorithm>
#include <limits>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace {
using Position = Coordinate<std::int64_t>;

struct State {
    Position                       Pos;
    std::int64_t                   Cost;
    Direction                      CurrentDirection;
    std::int64_t                   HeuristicValue;
    std::pair<Position, Direction> Predecessor;

    State(void) noexcept = default;

    State(std::pair<Position, Direction> posAndDirection) noexcept :
            Pos{posAndDirection.first}, Cost{0}, CurrentDirection{posAndDirection.second} {
        return;
    }

    bool operator==(const State& that) const noexcept {
        return Pos == that.Pos && CurrentDirection == that.CurrentDirection;
    }
};

struct StateHash {
    static std::size_t operator()(const State& state) noexcept {
        return std::hash<Position>{}(state.Pos) + static_cast<std::size_t>(state.CurrentDirection);
    }
};

struct CostData {
    std::int64_t                                Cost;
    std::vector<std::pair<Position, Direction>> Precedessors;
};

auto findCheapestPath(MapView map) {
    const Position start{Position::MaxRow - 2, 1};
    const Position end{1, Position::MaxColumn - 2};

    throwIfInvalid(map[start] == 'S');
    throwIfInvalid(map[end] == 'E');

    std::vector<State>                      toVisit;
    std::unordered_map<State, CostData, StateHash> leastCost;

    auto push = [map, &toVisit, end](State state, State previousState) noexcept {
        if ( map[state.Pos] == '#' ) {
            return;
        } //if ( map[state.Pos] == '#' )
        state.HeuristicValue = state.Cost + (end.Column - state.Pos.Column + state.Pos.Row - end.Row);
        state.Predecessor    = {previousState.Pos, previousState.CurrentDirection};
        toVisit.push_back(state);
        std::ranges::push_heap(toVisit, std::ranges::greater{}, &State::HeuristicValue);
        return;
    };

    State initial{{start, Direction::Right}};
    push(initial, initial);

    std::int64_t cheapestCost = std::numeric_limits<std::int64_t>::max();

    while ( !toVisit.empty() ) {
        std::ranges::pop_heap(toVisit, std::ranges::greater{}, &State::HeuristicValue);
        const auto currentState = toVisit.back();
        toVisit.pop_back();

        if ( auto costIter = leastCost.find(currentState); costIter != leastCost.end() ) {
            if ( currentState.Cost > costIter->second.Cost ) {
                //++skipped;
                continue;
            } //if ( currentState.Cost > costIter->second.Cost )

            if ( currentState.Cost == costIter->second.Cost ) {
                costIter->second.Precedessors.push_back(currentState.Predecessor);
                continue;
            } //if ( currentState.Cost == costIter->second.Cost )

            costIter->second = {currentState.Cost, {}};
        } //if ( auto costIter = leastCost.find(currentState); costIter != leastCost.end() )
        else {
            if ( currentState.Pos != end || currentState.Cost <= cheapestCost ) {
                leastCost.emplace(currentState, CostData{currentState.Cost, std::vector{currentState.Predecessor}});
            } //if ( currentState.Pos != end || currentState.Cost <= cheapestCost )
        } //else -> if ( auto costIter = leastCost.find(currentState); costIter != leastCost.end() )

        if ( currentState.Pos == end ) {
            cheapestCost = std::min(cheapestCost, currentState.Cost);
            continue;
        } //if ( currentState.Pos == end )

        State movedState;
        movedState.CurrentDirection = currentState.CurrentDirection;
        movedState.Pos              = currentState.Pos.moved(movedState.CurrentDirection);
        movedState.Cost             = currentState.Cost + 1;
        push(movedState, currentState);

        State turnedLeftState;
        turnedLeftState.CurrentDirection = turnLeft(currentState.CurrentDirection);
        turnedLeftState.Pos              = currentState.Pos.moved(turnedLeftState.CurrentDirection);
        turnedLeftState.Cost             = currentState.Cost + 1001;
        push(turnedLeftState, currentState);

        State turnedRightState;
        turnedRightState.CurrentDirection = turnRight(currentState.CurrentDirection);
        turnedRightState.Pos              = currentState.Pos.moved(turnedRightState.CurrentDirection);
        turnedRightState.Cost             = currentState.Cost + 1001;
        push(turnedRightState, currentState);
    } //while ( !toVisit.empty() )

    std::unordered_set<Position> visited{start};

    auto addVisited = [&visited, &leastCost, &start](this auto&                     self,
                                                     std::pair<Position, Direction> posAndDirection) noexcept -> void {
        if ( posAndDirection.first == start ) {
            return;
        } //if ( posAndDirection.first == start )

        visited.insert(posAndDirection.first);

        for ( auto precedessor : leastCost[State{posAndDirection}].Precedessors ) {
            self(precedessor);
        } //for ( auto precedessor : leastCost[State{posAndDirection}].Precedessors )
        return;
    };
    addVisited({end, Direction::Up});
    addVisited({end, Direction::Right});

    return std::pair{cheapestCost, visited.size()};
}
} //namespace

bool challenge16(const std::vector<std::string_view>& input) {
    Position::setMaxFromMap(input);

    const auto [pathCost, nodesOnPathes] = findCheapestPath(input);
    myPrint(" == Result of Part 1: {:d} ==\n", pathCost);

    // const auto nodesOnPathes = findAllNodesOnCheapestPaths(input, pathCost);
    myPrint(" == Result of Part 2: {:d} ==\n", nodesOnPathes);

    return pathCost == 105496 && nodesOnPathes == 524;
}
