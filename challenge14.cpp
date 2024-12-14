#include "challenge14.hpp"

#include "helper.hpp"
#include "print.hpp"
#include "3rdParty/ctre/include/ctre.hpp"

#include <algorithm>
#include <ranges>

namespace {
using Position                 = Coordinate<std::int64_t>;
using Offset                   = CoordinateOffset<std::int64_t>;

constexpr auto NumberOfRows    = 103;
constexpr auto NumberOfColumns = 101;

inline void mod(std::int64_t& x, int m) noexcept {
    x = ((x % m) + m) % m;
    return;
}

struct Robot {
    Position Start;
    Offset   Movement;

    void move(void) noexcept {
        Start += Movement;
        mod(Start.Row, NumberOfRows);
        mod(Start.Column, NumberOfColumns);
        return;
    }

    Position positionAfter100Moves(void) const noexcept {
        auto ret = Start + Movement * 100;
        mod(ret.Row, NumberOfRows);
        mod(ret.Column, NumberOfColumns);
        return ret;
    }

    static Robot parse(std::string_view line) noexcept {
        auto match = ctre::match<R"(p=(\d+),(\d+) v=(-?\d+),(-?\d+))">(line);
        return {{convert(match.get<2>().view()), convert(match.get<1>().view())},
                {convert(match.get<4>().view()), convert(match.get<3>().view())}};
    }
};
} //namespace

bool challenge14(const std::vector<std::string_view>& input) {
    auto robots            = input | std::views::transform(&Robot::parse) | std::ranges::to<std::vector>();

    auto topLeftRobots     = 0;
    auto topRightRobots    = 0;
    auto bottomLeftRobots  = 0;
    auto bottomRightRobots = 0;
    auto count = [&topLeftRobots, &topRightRobots, &bottomLeftRobots, &bottomRightRobots](const Robot& robot) noexcept {
        const auto position         = robot.positionAfter100Moves();

        constexpr auto middleColumn = NumberOfColumns / 2;
        constexpr auto middleRow    = NumberOfRows / 2;

        if ( position.Row == middleRow || position.Column == middleColumn ) {
            return;
        } //if ( position.Row == middleRow || position.Column == middleColumn )

        auto left  = &bottomLeftRobots;
        auto right = &bottomRightRobots;

        if ( position.Row < middleRow ) {
            left  = &topLeftRobots;
            right = &topRightRobots;
        } //if ( position.Row < middleRow )

        if ( position.Column < middleColumn ) {
            ++*left;
        } //if ( position.Column < middleColumn )
        else {
            ++*right;
        } //else -> if ( position.Column < middleColumn )
        return;
    };
    std::ranges::for_each(robots, count);

    const auto safetyFactor = topLeftRobots * topRightRobots * bottomLeftRobots * bottomRightRobots;
    myPrint(" == Result of Part 1: {:d} ==\n", safetyFactor);

    //Had to look at reddit... who thinks of something like this?!?
    auto isATree = [&robots](void) noexcept {
        auto fullyCircled = [&robots](const Robot& robot) noexcept {
            return std::ranges::all_of(
                std::views::cartesian_product(std::views::iota(-1, 2), std::views::iota(-1, 2)) |
                    std::views::transform(
                        [&robot](auto p) noexcept { return robot.Start + Offset{std::get<0>(p), std::get<1>(p)}; }),
                [&robots](const Position& position) { return std::ranges::contains(robots, position, &Robot::Start); });
        };
        return std::ranges::any_of(robots, fullyCircled);
    };

    auto stepsToTree = 0;
    do {
        ++stepsToTree;
        std::ranges::for_each(robots, &Robot::move);
    } while ( !isATree() );
    myPrint(" == Result of Part 2: {:d} ==\n", stepsToTree);

    return safetyFactor == 225'521'010 && stepsToTree == 7774;
}
