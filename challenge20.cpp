#include "challenge20.hpp"

#include "helper.hpp"
#include "print.hpp"

#include <algorithm>
#include <ranges>
#include <unordered_map>

namespace {
using Position = Coordinate<std::int64_t>;

auto findStartAndEnd(MapView map) noexcept {
    int                           found = 0;
    std::pair<Position, Position> ret;

    for ( auto position : Position::allPositions() ) {
        if ( map[position] == 'E' ) {
            ret.second = position;
        } //if ( map[position] == 'E' )
        else if ( map[position] == 'S' ) {
            ret.first = position;
        } //else if ( map[position] == 'S' )
        else {
            continue;
        } //else

        if ( ++found == 2 ) {
            break;
        } //if ( ++found == 2 )
    } //for ( auto position : Position::allPositions() )

    return ret;
}

using PathResult = std::unordered_map<Position, std::int64_t>;

PathResult findPath(MapView map, const Position start, const Position end) noexcept {
    struct PathNode {
        Position     Pos;
        std::int64_t Cost;
    };

    auto     notWall  = [&map](Position pos) noexcept { return map[pos] != '#'; };
    PathNode current  = {(start.validNeighbors() | std::views::filter(notWall)).front(), 1};
    auto     previous = start;
    auto     isNext   = [&notWall, &previous](Position pos) noexcept { return pos != previous && notWall(pos); };

    PathResult ret{{start, 0}, {current.Pos, 1}};

    while ( current.Pos != end ) {
        auto nextPos = (current.Pos.validNeighbors() | std::views::filter(isNext)).front();
        previous     = std::exchange(current.Pos, nextPos);
        ++current.Cost;
        ret.emplace(current.Pos, current.Cost);
    } //while ( current.Pos != end )
    return ret;
}

std::int64_t countCheats(MapView map, const PathResult& path, const int maxCheatLength,
                         const std::int64_t saveThreshold) noexcept {
    using PositionOffset = CoordinateOffset<std::int64_t>;

    const auto possibleCheatOffsets =
        std::views::cartesian_product(std::views::iota(-maxCheatLength, maxCheatLength + 1),
                                      std::views::iota(-maxCheatLength, maxCheatLength + 1)) |
        std::views::transform(
            [](auto tuple) noexcept { return PositionOffset{std::get<0>(tuple), std::get<1>(tuple)}; }) |
        std::views::filter(
            [&maxCheatLength](PositionOffset offset) noexcept { return offset.length() <= maxCheatLength; }) |
        std::ranges::to<std::vector>();

    auto isWall               = [map](Position position) noexcept { return map[position] == '#'; };
    auto calculateCheatSaving = [&isWall, &path](const auto& tuple) noexcept -> std::int64_t {
        const auto& [start, offset]            = tuple;
        const auto& [startPosition, startCost] = start;
        const auto endPosition                 = startPosition + offset;

        if ( !endPosition.isValid() || isWall(endPosition) ) {
            return 0;
        } //if ( !endPosition.isValid() || isWall(endPosition) )

        const auto endCost = path.find(endPosition)->second;
        const auto saved   = endCost - startCost - offset.length();
        return saved;
    };

    return std::ranges::count_if(std::views::cartesian_product(path, possibleCheatOffsets) |
                                     std::views::transform(calculateCheatSaving),
                                 [&saveThreshold](std::int64_t saving) noexcept { return saving >= saveThreshold; });
}
} //namespace

bool challenge20(const std::vector<std::string_view>& input) {
    const MapView map{input};
    Position::setMaxFromMap(map);
    const auto [start, end] = findStartAndEnd(map);
    const auto path         = findPath(map, start, end);

    const auto cheatCount1  = countCheats(map, path, 2, 100);
    myPrint(" == Result of Part 1: {:d} ==\n", cheatCount1);

    const auto cheatCount2 = countCheats(map, path, 20, 100);
    myPrint(" == Result of Part 2: {:d} ==\n", cheatCount2);

    return cheatCount1 == 1454 && cheatCount2 == 243'037'165'713'371;
}
