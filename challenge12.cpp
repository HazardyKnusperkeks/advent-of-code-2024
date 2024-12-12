#include "challenge12.hpp"

#include "helper.hpp"
#include "print.hpp"

#include <algorithm>
#include <ranges>

namespace {
using Position = Coordinate<std::int64_t>;

struct Region {
    std::int64_t LowestRow     = 0;
    std::int64_t HighestRow    = 0;
    std::int64_t LowestColumn  = 0;
    std::int64_t HighestColumn = 0;

    std::int32_t Area          = 0;
    std::int32_t Perimeter     = 0;
    std::int64_t Sides         = 0;

    auto price(void) const noexcept {
        return Area * Perimeter;
    }

    auto bulkPrice(void) const noexcept {
        return Area * Sides;
    }
};

struct ParseAssignment {
    std::vector<std::size_t> Assignment;

    static constexpr std::size_t Unassigned = 0;

    ParseAssignment(void) noexcept {
        Assignment.resize(static_cast<std::size_t>(Position::MaxColumn * Position::MaxRow), Unassigned);
        return;
    }

    decltype(auto) assignment(this auto& self, Position pos) noexcept {
        return self.Assignment[static_cast<std::size_t>(pos.Row * Position::MaxColumn + pos.Column)];
    }

    std::size_t operator()(Position pos) const noexcept {
        return assignment(pos);
    }

    bool isAssigned(Position pos) const noexcept {
        return assignment(pos) != Unassigned;
    }

    void assign(Position pos, std::size_t index) noexcept {
        assignment(pos) = index;
        return;
    }
};

auto parse(MapView map) noexcept {
    Position::setMaxFromMap(map);
    std::vector<Region> regions;
    ParseAssignment     assignment;

    auto notAssigned = [&assignment](Position pos) noexcept { return !assignment.isAssigned(pos); };
    auto getIndex    = [&assignment](Position pos) noexcept {
        return pos.isValid() ? assignment(pos) : ParseAssignment::Unassigned;
    };

    for ( auto position : Position::allPositions() | std::views::filter(notAssigned) ) {
        auto&      region      = regions.emplace_back();
        const auto regionIndex = regions.size();
        const auto regionType  = map[position];
        region.LowestRow = region.HighestRow = position.Row;
        region.LowestColumn = region.HighestColumn = position.Column;

        auto assignPosition                        = [&assignment, map, &region, regionIndex, regionType,
                               &notAssigned](this auto& self, const Position positionToAdd) noexcept -> void {
            ++region.Area;
            assignment.assign(positionToAdd, regionIndex);
            region.LowestRow     = std::min(region.LowestRow, positionToAdd.Row);
            region.HighestRow    = std::max(region.HighestRow, positionToAdd.Row);
            region.LowestColumn  = std::min(region.LowestColumn, positionToAdd.Column);
            region.HighestColumn = std::max(region.HighestColumn, positionToAdd.Column);

            for ( auto neighbor : positionToAdd.neighbors() ) {
                if ( !neighbor.isValid() ) {
                    ++region.Perimeter;
                } //if ( !neighbor.isValid() )
                else if ( map[neighbor] != regionType ) {
                    ++region.Perimeter;
                } //if ( map[neighbor] != regionType )
                else if ( notAssigned(neighbor) ) {
                    self(neighbor);
                } //else if ( notAssigned(neighbor) )
            } //for ( auto neighbor : positionToAdd.neighbors() )
            return;
        };

        assignPosition(position);
    } //for ( auto position : Position::allPositions() | std::views::filter(notAssigned) )

    for ( auto&& [regionIndex, region] : regions | std::views::enumerate ) {
        std::vector<std::int64_t> lastSideCoordinates;
        std::vector<std::int64_t> currentSideCoordinates;
        auto                      newCoordinate = [&lastSideCoordinates](std::int64_t coordinate) noexcept {
            return !std::ranges::contains(lastSideCoordinates, coordinate);
        };
        ++regionIndex;

        auto generateEdgeDetected = [&getIndex, regionIndex](auto makePosition) noexcept {
            return [lastIndex = ParseAssignment::Unassigned, makePosition, &getIndex,
                    regionIndex](std::int64_t secondCoordinate) mutable noexcept {
                auto pos   = makePosition(secondCoordinate);
                auto index = getIndex(pos);
                auto ret   = index == static_cast<std::size_t>(regionIndex) && lastIndex != index;
                lastIndex  = index;
                return ret;
            };
        };

        //Sides to the left:
        for ( auto row = region.LowestRow; row <= region.HighestRow; ++row ) {
            auto makePosition = [row](std::int64_t column) noexcept { return Position{row, column}; };
            auto edgeDetected = generateEdgeDetected(makePosition);

            std::ranges::copy(std::views::iota(region.LowestColumn, region.HighestColumn + 1) |
                                  std::views::filter(edgeDetected),
                              std::back_inserter(currentSideCoordinates));
            region.Sides += std::ranges::count_if(currentSideCoordinates, newCoordinate);
            std::swap(lastSideCoordinates, currentSideCoordinates);
            currentSideCoordinates.clear();
        } //for ( auto row = region.LowestRow; row <= region.HighestRow; ++row )

        //Sides to the right:
        lastSideCoordinates.clear();
        for ( auto row = region.LowestRow; row <= region.HighestRow; ++row ) {
            auto makePosition = [row](std::int64_t column) noexcept { return Position{row, column}; };
            auto edgeDetected = generateEdgeDetected(makePosition);

            std::ranges::copy(std::views::iota(region.LowestColumn, region.HighestColumn + 1) | std::views::reverse |
                                  std::views::filter(edgeDetected),
                              std::back_inserter(currentSideCoordinates));
            region.Sides += std::ranges::count_if(currentSideCoordinates, newCoordinate);
            std::swap(lastSideCoordinates, currentSideCoordinates);
            currentSideCoordinates.clear();
        } //for ( auto row = region.LowestRow; row <= region.HighestRow; ++row )

        //Sides to the top:
        lastSideCoordinates.clear();
        for ( auto column = region.LowestColumn; column <= region.HighestColumn; ++column ) {
            auto makePosition = [column](std::int64_t row) noexcept { return Position{row, column}; };
            auto edgeDetected = generateEdgeDetected(makePosition);

            std::ranges::copy(std::views::iota(region.LowestRow, region.HighestRow + 1) |
                                  std::views::filter(edgeDetected),
                              std::back_inserter(currentSideCoordinates));
            region.Sides += std::ranges::count_if(currentSideCoordinates, newCoordinate);
            std::swap(lastSideCoordinates, currentSideCoordinates);
            currentSideCoordinates.clear();
        } //for ( auto column = region.LowestColumn; column <= region.HighestColumn; ++column )

        //Sides to the bottom:
        lastSideCoordinates.clear();
        for ( auto column = region.LowestColumn; column <= region.HighestColumn; ++column ) {
            auto makePosition = [column](std::int64_t row) noexcept { return Position{row, column}; };
            auto edgeDetected = generateEdgeDetected(makePosition);

            std::ranges::copy(std::views::iota(region.LowestRow, region.HighestRow + 1) | std::views::reverse |
                                  std::views::filter(edgeDetected),
                              std::back_inserter(currentSideCoordinates));
            region.Sides += std::ranges::count_if(currentSideCoordinates, newCoordinate);
            std::swap(lastSideCoordinates, currentSideCoordinates);
            currentSideCoordinates.clear();
        } //for ( auto column = region.LowestColumn; column <= region.HighestColumn; ++column )
    } //for ( auto&& [regionIndex, region] : regions | std::views::enumerate )

    return regions;
}
} //namespace

bool challenge12(const std::vector<std::string_view>& input) {
    const auto regions = parse(input);

    auto sum1          = std::ranges::fold_left(regions | std::views::transform(&Region::price), 0, std::plus<>{});
    myPrint(" == Result of Part 1: {:d} ==\n", sum1);

    auto sum2 = std::ranges::fold_left(regions | std::views::transform(&Region::bulkPrice), 0, std::plus<>{});
    myPrint(" == Result of Part 2: {:d} ==\n", sum2);

    return sum1 == 1549354 && sum2 == 937032;
}
