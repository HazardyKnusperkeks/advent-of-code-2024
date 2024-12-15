#include "challenge15.hpp"

#include "helper.hpp"
#include "print.hpp"

#include <algorithm>
#include <deque>
#include <ranges>
#include <unordered_map>
#include <unordered_set>

namespace {
using Position = Coordinate<std::int64_t>;

enum class Element : char { Wall = '#', Robot = '@', Crate = 'O', Free = '.' };

enum class Movement : char { Left = '<', Right = '>', Up = '^', Down = 'v' };

Direction mapMovement(Movement movement) {
    switch ( movement ) {
        case Movement::Left  : return Direction::Left;
        case Movement::Right : return Direction::Right;
        case Movement::Up    : return Direction::Up;
        case Movement::Down  : return Direction::Down;
    } //switch ( movement )

    throwIfInvalid(false);
    std::unreachable();
}

struct World {
    Position                     RobotPosition;
    std::unordered_set<Position> Walls;
    std::unordered_set<Position> Crates;
    std::vector<Direction>       Movements;

    void parse(std::span<const std::string_view> input) {
        auto addLineToMap = [this](auto rowAndLine) noexcept {
            auto [row, line] = rowAndLine;
            auto addToMap    = [this, row](auto columnAndSymbol) noexcept {
                auto [column, symbol] = columnAndSymbol;
                Position pos{row, column};
                switch ( static_cast<Element>(symbol) ) {
                    case Element::Robot : RobotPosition = pos; break;
                    case Element::Wall  : Walls.emplace(pos); break;
                    case Element::Crate : Crates.emplace(pos); break;
                    case Element::Free  : break;
                } //switch ( static_cast<Element>(symbol) )

                return;
            };
            std::ranges::for_each(line | std::views::enumerate, addToMap);
            return;
        };
        auto mapInput = input | std::views::take_while([](std::string_view line) noexcept { return !line.empty(); }) |
                        std::views::enumerate;
        auto mapEnd = std::ranges::for_each(mapInput, addLineToMap).in.base();

        Movements   = std::ranges::subrange(mapEnd, input.end()) | std::views::join |
                    std::views::transform([](char c) noexcept { return mapMovement(static_cast<Movement>(c)); }) |
                    std::ranges::to<std::vector>();
        return;
    }

    void move(Direction direction) noexcept {
        auto robotTarget = RobotPosition.moved(direction);

        if ( Walls.contains(robotTarget) ) {
            return;
        } //if ( Walls.contains(robotTarget) )

        auto crate = Crates.find(robotTarget);

        if ( crate == Crates.end() ) {
            RobotPosition = robotTarget;
            return;
        } //if ( crate == Crates.end() )

        auto crateTarget = robotTarget.moved(direction);
        bool atWall;
        auto atFreeOrWall = [this, &crateTarget, &atWall](void) noexcept {
            atWall = Walls.contains(crateTarget);
            return atWall || !Crates.contains(crateTarget);
        };
        while ( !atFreeOrWall() ) {
            crateTarget.move(direction);
        } //while ( !atFreeOrWall() )

        if ( atWall ) {
            return;
        } //if ( atWall )

        Crates.erase(crate);
        Crates.insert(crateTarget);
        RobotPosition = robotTarget;
        return;
    }
};

struct BigBigWorld {
    Position                           RobotPosition;
    std::unordered_set<Position>       Walls;
    std::unordered_map<Position, bool> Crates;

    void parse(std::span<const std::string_view> input) {
        auto addLineToMap = [this](auto rowAndLine) noexcept {
            auto [row, line] = rowAndLine;
            auto addToMap    = [this, row](auto columnAndSymbol) noexcept {
                auto [column, symbol] = columnAndSymbol;
                Position pos{row, column * 2};
                switch ( static_cast<Element>(symbol) ) {
                    case Element::Robot : RobotPosition = pos; break;

                    case Element::Wall  : {
                        Walls.emplace(pos);
                        Walls.emplace(pos.right());
                        break;
                    } //case Element::Wall

                    case Element::Crate : {
                        Crates.emplace(pos, true);
                        Crates.emplace(pos.right(), false);
                        break;
                    } //case ELement::Crate

                    case Element::Free : break;
                } //switch ( static_cast<Element>(symbol) )

                return;
            };
            std::ranges::for_each(line | std::views::enumerate, addToMap);
            return;
        };
        auto mapInput = input | std::views::take_while([](std::string_view line) noexcept { return !line.empty(); }) |
                        std::views::enumerate;
        std::ranges::for_each(mapInput, addLineToMap).in.base();
        return;
    }

    void move(Direction direction) noexcept {
        auto robotTarget = RobotPosition.moved(direction);

        std::vector<Position> cratesToMove;
        std::deque<Position>  positionsToCheck;
        positionsToCheck.emplace_back(robotTarget);

        while ( !positionsToCheck.empty() ) {
            auto nextPosition = positionsToCheck.front();
            positionsToCheck.pop_front();

            if ( Walls.contains(nextPosition) ) {
                return;
            } //if ( Walls.contains(nextPosition) )

            if ( auto crate = Crates.find(nextPosition); crate != Crates.end() ) {
                if ( !crate->second ) {
                    nextPosition.move(Direction::Left);
                } //if ( !crate->second )
                else if ( !positionsToCheck.empty() && positionsToCheck.front() == nextPosition.right() ) {
                    positionsToCheck.pop_front();
                } //else if ( !positionsToCheck.empty() && positionsToCheck.front() == nextPosition.right() )

                switch ( direction ) {
                    using enum Direction;
                    case Left  : positionsToCheck.emplace_back(nextPosition.left()); break;
                    case Right : positionsToCheck.emplace_back(nextPosition.right().right()); break;

                    case Up    :
                    case Down  : {
                        positionsToCheck.emplace_back(nextPosition.moved(direction));
                        positionsToCheck.emplace_back(nextPosition.right().moved(direction));
                        break;
                    } //case Up & Down
                } //switch ( direction )

                cratesToMove.emplace_back(nextPosition);
            } //if ( auto crate = Crates.find(nextPosition); crate != Crates.end() )
        } //while ( !positionsToCheck.empty() )

        for ( auto cratePosition : cratesToMove | std::views::reverse ) {
            Crates.erase(cratePosition);
            Crates.erase(cratePosition.right());
            auto crateTarget = cratePosition.moved(direction);
            Crates.emplace(crateTarget, true);
            Crates.emplace(crateTarget.right(), false);
        } //for ( auto cratePosition : cratesToMove | std::views::reverse )
        RobotPosition = robotTarget;
        return;
    }
};

std::int64_t toGps(Position pos) noexcept {
    return pos.Column + pos.Row * 100;
}
} //namespace

bool challenge15(const std::vector<std::string_view>& input) {
    World       world;
    BigBigWorld bigBigWorld;

    world.parse(input);
    bigBigWorld.parse(input);

    for ( auto direction : world.Movements ) {
        world.move(direction);
        bigBigWorld.move(direction);
    } //for ( auto direction : world.Movements )

    auto sum1 = std::ranges::fold_left(world.Crates | std::views::transform(toGps), 0, std::plus<>{});
    myPrint(" == Result of Part 1: {:d} ==\n", sum1);

    auto sum2 = std::ranges::fold_left(
        bigBigWorld.Crates | std::views::filter([](const std::pair<const Position, bool>& crateSide) noexcept {
            return crateSide.second;
        }) | std::views::keys |
            std::views::transform(toGps),
        0, std::plus<>{});
    myPrint(" == Result of Part 2: {:d} ==\n", sum2);

    return sum1 == 1463512 && sum2 == 1486520;
}
