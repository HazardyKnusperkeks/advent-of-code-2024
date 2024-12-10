#include "challenge10.hpp"

#include "helper.hpp"
#include "print.hpp"

#include <algorithm>
#include <ranges>
#include <stack>
#include <unordered_set>

namespace {
using Position = Coordinate<std::int64_t>;

auto calcScore(Position startPosition, MapView map) noexcept {
    struct ToCheck {
        Position Pos;
        int      Distance;
    };

    std::stack<ToCheck>          toCheck;
    std::unordered_set<Position> uniqueEndPositions;
    toCheck.push({startPosition, 0});

    while ( !toCheck.empty() ) {
        auto [position, distance] = toCheck.top();
        toCheck.pop();

        if ( distance == 9 ) {
            uniqueEndPositions.insert(position);
            continue;
        } //if ( distance == 9 )

        const auto nextDistance  = distance + 1;
        const char expectedField = static_cast<char>('0' + nextDistance);
        std::ranges::for_each(
            position.validNeighbors() | std::views::filter([expectedField, map](Position nextPosition) noexcept {
                return map[nextPosition] == expectedField;
            }),
            [&toCheck, nextDistance](Position nextPosition) noexcept { toCheck.push({nextPosition, nextDistance}); });
    } //while ( !toCheck.empty() )
    return uniqueEndPositions.size();
}

auto calcRating(Position startPosition, MapView map) noexcept {
    struct ToCheck {
        Position Pos;
        int      Distance;
    };

    std::stack<ToCheck> toCheck;
    toCheck.push({startPosition, 0});
    std::int64_t rating = 0;

    while ( !toCheck.empty() ) {
        auto [position, distance] = toCheck.top();
        toCheck.pop();

        if ( distance == 9 ) {
            ++rating;
            continue;
        } //if ( distance == 9 )

        const auto nextDistance  = distance + 1;
        const char expectedField = static_cast<char>('0' + nextDistance);
        std::ranges::for_each(
            position.validNeighbors() | std::views::filter([expectedField, map](Position nextPosition) noexcept {
                return map[nextPosition] == expectedField;
            }),
            [&toCheck, nextDistance](Position nextPosition) noexcept { toCheck.push({nextPosition, nextDistance}); });
    } //while ( !toCheck.empty() )
    return rating;
}
} //namespace

bool challenge10(const std::vector<std::string_view>& input) {
    MapView map{input};
    Position::setMaxFromMap(map);
    auto allStartPositions =
        Position::allPositions() | std::views::filter([map](Position pos) noexcept { return map[pos] == '0'; });
    const auto sum1 =
        std::ranges::fold_left(allStartPositions | std::views::transform([map](Position startPosition) noexcept {
                                   return calcScore(startPosition, map);
                               }),
                               0, std::plus<>{});
    myPrint(" == Result of Part 1: {:d} ==\n", sum1);

    auto sum2 =
        std::ranges::fold_left(allStartPositions | std::views::transform([map](Position startPosition) noexcept {
                                   return calcRating(startPosition, map);
                               }),
                               0, std::plus<>{});
    ;
    myPrint(" == Result of Part 2: {:d} ==\n", sum2);

    return sum1 == 538 && sum2 == 1110;
}
