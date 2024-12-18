#include "challenge18.hpp"

#include "helper.hpp"
#include "print.hpp"

#include <algorithm>
#include <ranges>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace {
using Position = Coordinate<std::int64_t>;

using Bytes    = std::unordered_set<Position>;

struct Node {
    Position     Pos;
    std::int64_t Cost;
    std::int64_t Heuristic;
};

Position lineToPosition(std::string_view line) noexcept {
    auto comma = line.find(',');
    return Position{convert(line.substr(comma + 1)), convert(line.substr(0, comma))};
}

Bytes read(std::span<const std::string_view> input, int limit) noexcept {
    return input | std::views::take(limit) | std::views::transform(lineToPosition) |
           std::ranges::to<std::unordered_set>();
}

std::int64_t findShortestPath(const Bytes& bytes) noexcept {
    std::unordered_map<Position, std::int64_t> visited;
    std::vector<Node>                          toVisit;
    const Position                             start{0, 0};
    const Position                             end{Position::MaxRow - 1, Position::MaxColumn - 1};

    auto buildHeuristic = [end](Position from) noexcept { return end.Row - from.Row + end.Column - from.Column; };

    toVisit.emplace_back(start, 0, buildHeuristic(start));

    while ( !toVisit.empty() ) {
        std::ranges::pop_heap(toVisit, std::ranges::greater{}, &Node::Heuristic);
        auto current = toVisit.back();
        toVisit.pop_back();

        auto position = current.Pos;

        if ( position == end ) {
            return current.Cost;
        } //if ( position == end )

        auto push = [&visited, &toVisit, &current, &bytes, &buildHeuristic](Position pos) noexcept {
            if ( !pos.isValid() ) {
                return;
            } //if ( !pos.isValid() )

            if ( bytes.contains(pos) ) {
                return;
            } //if ( bytes.contains(pos) )

            if ( auto iter = visited.find(pos); iter != visited.end() ) {
                if ( iter->second <= current.Cost ) {
                    return;
                } //if ( iter->second <= current.Cost )

                iter->second = current.Cost;
            } //if ( auto iter = visited.find(pos); iter != visited.end() )
            else {
                visited.emplace(pos, current.Cost);
            } //else -> if ( auto iter = visited.find(pos); iter != visited.end() )

            current.Pos       = pos;
            current.Heuristic = current.Cost + buildHeuristic(pos);

            toVisit.push_back(current);
            std::ranges::push_heap(toVisit, std::ranges::greater{}, &Node::Heuristic);
            return;
        };

        ++current.Cost;
        push(position.down());
        push(position.left());
        push(position.right());
        push(position.up());
    } //while ( !toVisit.empty() )
    return -1;
}
} //namespace

bool challenge18(const std::vector<std::string_view>& input) {
    Position::MaxColumn = Position::MaxRow = 71;
    auto bytes                             = read(input, 1024);

    const auto shortestPath                = findShortestPath(bytes);
    myPrint(" == Result of Part 1: {:d} ==\n", shortestPath);

    std::string_view blockingByte;
    for ( auto line : input | std::views::drop(1024) ) {
        bytes.emplace(lineToPosition(line));

        //This runs in Debug for about 8 seconds.
        //A bit more clever would be adding bytes until one is really blocking the old path.
        if ( findShortestPath(bytes) == -1 ) {
            blockingByte = line;
            break;
        } //if ( findShortestPath(bytes) == -1 )
    } //for ( auto line : input | std::views::drop(1024) )
    myPrint(" == Result of Part 2: {:s} ==\n", blockingByte);

    return shortestPath == 316 && blockingByte == "45,18";
}
