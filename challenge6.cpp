#include "challenge6.hpp"

#include "helper.hpp"
#include "print.hpp"

#include <unordered_map>
#include <unordered_set>

namespace {
using Position = Coordinate<std::int64_t>;

Position findStart(MapView map) noexcept {
    Position::MaxRow    = static_cast<std::int64_t>(map.Base.size());
    Position::MaxColumn = static_cast<std::int64_t>(map.Base.front().size());

    for ( std::int64_t row = 0; row < Position::MaxRow; ++row ) {
        auto column = map.Base[static_cast<std::size_t>(row)].find('^');

        if ( column != std::string_view::npos ) {
            return {row, static_cast<std::int64_t>(column)};
        } //if ( column != std::string_view::npos )
    } //for ( std::int64_t row = 0; row < Position::MaxRow; ++row )
    return {};
}

auto moveFromMap(MapView map, Position currentPosition) noexcept {
    Direction                    direction = Direction::Up;
    std::unordered_set<Position> visited;

    while ( currentPosition.isValid() ) {
        if ( map[currentPosition] == '#' ) {
            currentPosition.move(turnAround(direction));
            direction = turnRight(direction);
        } //if ( map[currentPosition] == '#' )
        else {
            visited.insert(currentPosition);
            currentPosition.move(direction);
        } //else -> if ( map[currentPosition] == '#' )
    } //while ( currentPosition.isValid() )
    return visited.size();
}

Direction& operator|=(Direction& lhs, Direction rhs) noexcept {
    reinterpret_cast<std::underlying_type_t<Direction>&>(lhs) |= std::to_underlying(rhs);
    return lhs;
}

bool operator&(Direction lhs, Direction rhs) noexcept {
    return std::to_underlying(lhs) & std::to_underlying(rhs);
}

auto findObstaclesForLoop(MapView map, const Position startPosition) noexcept {
    std::unordered_map<Position, Direction> moved;
    Direction                               direction       = Direction::Up;
    std::int64_t                            obstacleCount   = 0;
    auto                                    currentPosition = startPosition;

    while ( currentPosition.isValid() ) {
        if ( map[currentPosition] == '#' ) {
            currentPosition.move(turnAround(direction));
            direction = turnRight(direction);
        } //if ( map[currentPosition] == '#' )
        else {
            moved[currentPosition] |= direction;
            const auto obstablePosition   = currentPosition.moved(direction);

            if ( obstablePosition.isValid() && map[obstablePosition] == '.' && !moved.contains(obstablePosition) ) {
                auto         thoughtPosition  = currentPosition;
                auto         thoughtDirection = turnRight(direction);
                auto         thoughtMoved     = moved;
                while ( thoughtPosition.isValid() ) {
                    if ( thoughtPosition == obstablePosition || map[thoughtPosition] == '#' ) {
                        thoughtPosition.move(turnAround(thoughtDirection));
                        thoughtMoved[thoughtPosition] |= thoughtDirection;
                        thoughtDirection               = turnRight(thoughtDirection);
                    } //if ( thoughtPosition == obstablePosition || map[thoughtPosition] == '#' )
                    else {
                        if ( thoughtMoved[thoughtPosition] & thoughtDirection ) {
                            //We were here already with the same direction, putting an obstacle will result in a loop.
                            ++obstacleCount;
                            break;
                        } //if ( thoughtMoved[thoughtPosition] & thoughtDirection )
                        thoughtMoved[thoughtPosition] |= thoughtDirection;
                        thoughtPosition.move(thoughtDirection);
                    } //else -> if ( thoughtPosition == obstablePosition || map[thoughtPosition] == '#' )
                } //while ( thoughtPosition.isValid() )
            } //if ( obstablePosition.isValid() && map[obstablePosition] == '.' && !moved.contains(obstablePosition) )

            currentPosition.move(direction);
        } //else -> if ( map[currentPosition] == '#' )
    } //while ( currentPosition.isValid() )
    return obstacleCount;
}
} //namespace

bool challenge6(const std::vector<std::string_view>& input) {
    const auto startPosition = findStart(input);
    const auto visitedFields = moveFromMap(input, startPosition);

    myPrint(" == Result of Part 1: {:d} ==\n", visitedFields);

    const auto obstacles = findObstaclesForLoop(input, startPosition);
    myPrint(" == Result of Part 2: {:d} ==\n", obstacles);

    return visitedFields == 5239 && obstacles == 1753;
}
