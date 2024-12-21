#include "challenge21.hpp"

#include "helper.hpp"
#include "print.hpp"

#include <algorithm>
#include <queue>
#include <unordered_map>

namespace {
enum class NumericKey { Zero, One, Two, Three, Four, Five, Six, Seven, Eight, Nine, A, Invalid };
enum class DirectionalKey { Up, Left, Down, Right, A, Invalid };

template<typename Key>
struct FullMove {
    Key From;
    Key To;

    bool operator==(const FullMove&) const noexcept = default;
};

using NumericMove = FullMove<NumericKey>;

struct MyHash {
    template<typename Key>
    static std::size_t operator()(const FullMove<Key>& move) noexcept {
        return std::hash<int>{}(std::to_underlying(move.From)) + static_cast<std::size_t>(std::to_underlying(move.To));
    }
};

enum class Move : char { Up = '^', Down = 'v', Left = '<', Right = '>', Push = 'A', Invalid = '?' };

Move invert(Move m) noexcept {
    using enum Move;
    switch ( m ) {
        case Up      : return Down;
        case Down    : return Up;
        case Left    : return Right;
        case Right   : return Left;
        case Push    : break;
        case Invalid : break;
    } //switch ( m )
    return Push;
}

void print(Move m) noexcept {
    myPrint("{:c}", std::to_underlying(m));
    return;
}

using Moves = std::vector<Move>;

template<typename Key, std::size_t Rows, std::size_t Columns>
auto buildDistances(std::array<std::array<Key, Columns>, Rows> map) noexcept {
    std::unordered_map<FullMove<Key>, Moves, MyHash> distances;

    auto addMove = [&map, &distances](std::size_t beginRow, std::size_t beginColumn, std::size_t endRow,
                                      std::size_t endColumn) noexcept {
        FullMove<Key> move{map[beginRow][beginColumn], map[endRow][endColumn]};

        if ( move.To == Key::Invalid ) {
            return;
        } //if ( move.To == Key::Invalid )

        const auto numberOfHorizontalSteps = static_cast<std::int64_t>(endColumn - beginColumn);
        const auto numberOfVerticalSteps   = static_cast<std::int64_t>(endRow - beginRow);
        const bool isToRight               = numberOfHorizontalSteps >= 0;
        const bool isDown                  = numberOfVerticalSteps >= 0;
        const auto horizontalSteps =
            std::views::repeat(isToRight ? Move::Right : Move::Left, std::abs(numberOfHorizontalSteps)) |
            std::ranges::to<std::vector>();
        const auto verticalSteps = std::views::repeat(isDown ? Move::Down : Move::Up, std::abs(numberOfVerticalSteps)) |
                                   std::ranges::to<std::vector>();

        Moves moves;
        Moves reversedMoves;
        auto  forwardIter  = std::back_inserter(moves);
        auto  reversedIter = std::back_inserter(reversedMoves);

        if constexpr ( std::same_as<Key, NumericKey> ) {
            if ( move.From == NumericKey::Seven && move.To == NumericKey::Three ) {
                myPrint("");
            } //if ( move.From == NumericKey::Seven && move.To == NumericKey::Three )
        } //if ( std::same_as<Key, NumericKey> )

        //const bool hitsInvalid = map[beginRow][endColumn]==Key::Invalid

        if ( isToRight ) {
            forwardIter = std::ranges::copy(horizontalSteps, forwardIter).out;
        } //if ( isToRight )
        else {
            reversedIter = std::ranges::copy(horizontalSteps | std::views::transform(invert), reversedIter).out;
        } //else -> if ( isToRight )

        forwardIter  = std::ranges::copy(verticalSteps, forwardIter).out;
        reversedIter = std::ranges::copy(verticalSteps | std::views::transform(invert), reversedIter).out;

        if ( !isToRight ) {
            forwardIter = std::ranges::copy(horizontalSteps, forwardIter).out;
        } //if ( !isToRight )
        else {
            reversedIter = std::ranges::copy(horizontalSteps | std::views::transform(invert), reversedIter).out;
        } //else -> if ( !isToRight )

        moves.push_back(Move::Push);
        reversedMoves.push_back(Move::Push);
        distances.emplace(move, std::move(moves));
        std::swap(move.From, move.To);
        distances.emplace(move, std::move(reversedMoves));
        return;
    };

    for ( auto beginRow = 0u; beginRow < Rows; ++beginRow ) {
        for ( auto beginColumn = 0u; beginColumn < Columns; ++beginColumn ) {
            const auto beginKey = map[beginRow][beginColumn];

            if ( beginKey == Key::Invalid ) {
                continue;
            } //if ( beginKey == Key::Invalid )

            addMove(beginRow, beginColumn, beginRow, beginColumn);

            //Same Row:
            for ( auto endColumn = beginColumn + 1; endColumn < Columns; ++endColumn ) {
                addMove(beginRow, beginColumn, beginRow, endColumn);
            } //for ( auto endColumn = beginColumn + 1; endColumn < Columns; ++endColumn )

            for ( auto endRow = beginRow + 1; endRow < Rows; ++endRow ) {
                for ( auto endColumn = 0u; endColumn < Columns; ++endColumn ) {
                    addMove(beginRow, beginColumn, endRow, endColumn);
                } //for ( auto endColumn = 0u; endColumn < Columns; ++endColumn )
            } //for ( auto endRow = beginRow + 1; endRow < Rows; ++endRow )
        } //for ( auto beginColumn = 0u; beginColumn < Columns; ++beginColumn )
    } //for ( auto beginRow = 0u; beginRow < Rows; ++beginRow )
    return distances;
}

template<typename T>
extern int Distances;

template<>
const auto Distances<NumericKey> =
    buildDistances(std::array{std::array{NumericKey::Seven, NumericKey::Eight, NumericKey::Nine},
                              std::array{NumericKey::Four, NumericKey::Five, NumericKey::Six},
                              std::array{NumericKey::One, NumericKey::Two, NumericKey::Three},
                              std::array{NumericKey::Invalid, NumericKey::Zero, NumericKey::A}});

template<>
const auto Distances<Move> = buildDistances(
    std::array{std::array{Move::Invalid, Move::Up, Move::Push}, std::array{Move::Left, Move::Down, Move::Right}});

Moves moveRobot(Moves robotMoves) noexcept {
    robotMoves.insert(robotMoves.begin(), Move::Push);
    Moves ret = robotMoves | std::views::slide(2) | std::views::transform([](const auto& twoPositionRange) noexcept {
                    const Move from = twoPositionRange.front();
                    const Move to   = twoPositionRange.back();
                    return Distances<Move>.find({from, to})->second;
                }) |
                std::views::join | std::ranges::to<std::vector>();
    return ret;
}

std::int64_t getShortestSequenceLength(std::string_view code) {
    throwIfInvalid(code.size() == 4);
    throwIfInvalid(code.back() == 'A');
    std::array<NumericKey, 4> sequence;
    sequence.back() = NumericKey::A;
    std::ranges::transform(code.substr(0, 3), sequence.begin(),
                           [](char c) noexcept { return static_cast<NumericKey>(c - '0'); });

    Moves moves = sequence | std::views::transform([at = NumericKey::A](NumericKey to) mutable noexcept {
                      return Distances<NumericKey>.find({std::exchange(at, to), to})->second;
                  }) |
                  std::views::join | std::ranges::to<std::vector>();

    myPrint("Moves: ");
    std::ranges::for_each(moves, &print);
    myPrint("\n");

    moves = moveRobot(std::move(moves));

    myPrint("Moves: ");
    std::ranges::for_each(moves, &print);
    myPrint("\n");

    moves = moveRobot(std::move(moves));

    myPrint("Moves: ");
    std::ranges::for_each(moves, &print);
    myPrint("\n");

    return static_cast<std::int64_t>(moves.size());
}

std::int64_t getComplexity(std::string_view code) noexcept {
    auto length  = getShortestSequenceLength(code);
    auto numeric = convert(code);
    myPrint("Length: {:3d} Numeric: {:3d}\n", length, numeric);
    return length * numeric;
}
} //namespace

bool challenge21(const std::vector<std::string_view>& input) {
    const auto sum1 = std::ranges::fold_left(input | std::views::transform(getComplexity), 0, std::plus<>{});
    myPrint(" == Result of Part 1: {:d} ==\n", sum1);

    const auto sum2 = 0;
    myPrint(" == Result of Part 2: {:d} ==\n", sum2);

    return sum1 == 3858 && sum2 == 636'350'496'972'143;
}
