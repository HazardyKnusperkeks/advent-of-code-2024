#include "challenge8.hpp"

#include "helper.hpp"
#include "print.hpp"

#include <algorithm>
#include <ranges>
#include <unordered_map>
#include <unordered_set>

namespace {
using Position = Coordinate<std::int64_t>;

auto findAntennas(MapView map) {
    Position::setMaxFromMap(map);
    std::unordered_map<char, std::vector<Position>> antennas;
    std::ranges::for_each(std::views::cartesian_product(std::views::iota(0LL, Position::MaxRow),
                                                        std::views::iota(0LL, Position::MaxColumn)) |
                              std::views::transform([](auto rowAndColumn) noexcept {
                                  auto [row, column] = rowAndColumn;
                                  return Position{row, column};
                              }),
                          [&map, &antennas](Position pos) noexcept {
                              if ( auto antenna = map[pos]; antenna != '.' ) {
                                  antennas[antenna].push_back(pos);
                              } //if ( auto antenna = map[pos]; antenna != '.' )
                              return;
                          });
    return antennas;
}

auto findAntinodes(MapView map, const std::unordered_map<char, std::vector<Position>>& antennas) noexcept {
    std::unordered_set<Position> antinodes;
    for ( const auto& [antennaType, positions] : antennas ) {
        //This tries to add the position of the antennas (but they are filtered out).
        std::ranges::copy(std::views::cartesian_product(positions, positions) |
                              std::views::transform([](auto bothPositions) noexcept {
                                  auto [first, second] = bothPositions;
                                  return first + (second - first) * 2;
                              }) |
                              std::views::filter([&map, antennaType](Position pos) noexcept {
                                  return pos.isValid() && map[pos] != antennaType;
                              }),
                          std::inserter(antinodes, antinodes.end()));
    } //for ( const auto& [antennaType, positions] : antennas )
    return antinodes;
}

auto findResonantAntinodes(const std::unordered_map<char, std::vector<Position>>& antennas) noexcept {
    std::unordered_set<Position> antinodes;
    for ( const auto& [antennaType, positions] : antennas ) {
        if ( positions.size() == 1 ) {
            continue;
        } //if ( positions.size() == 1 )

        for ( auto [index, position] : std::views::enumerate(positions) ) {
            antinodes.insert(position);

            for ( auto nextPosition : positions | std::views::drop(index + 1) ) {
                auto offset = nextPosition - position;

                for ( auto positionToCheck  = nextPosition + offset; positionToCheck.isValid();
                      positionToCheck      += offset ) {
                    antinodes.insert(positionToCheck);
                } //for ( auto positionToCheck  = nextPosition + offset; positionToCheck.isValid(); += offset )

                offset *= -1;
                for ( auto positionToCheck = position + offset; positionToCheck.isValid(); positionToCheck += offset ) {
                    antinodes.insert(positionToCheck);
                } //for ( auto positionToCheck  = position + offset; positionToCheck.isValid(); += offset )
            } //for ( auto nextPosition : positions | std::views::drop(index + 1) )
        } //for ( auto [index, position] : std::views::enumerate(positions) )
    } //for ( const auto& [antennaType, positions] : antennas )
    return antinodes;
}
} //namespace

bool challenge8(const std::vector<std::string_view>& input) {
    const auto antennas          = findAntennas(input);
    const auto antinodes         = findAntinodes(input, antennas);
    const auto numberOfAntinodes = antinodes.size();

    myPrint(" == Result of Part 1: {:d} ==\n", numberOfAntinodes);

    const auto numberOfResonantAntinodes = findResonantAntinodes(antennas).size();
    myPrint(" == Result of Part 2: {:d} ==\n", numberOfResonantAntinodes);

    return numberOfAntinodes == 273 && numberOfResonantAntinodes == 1017;
}
