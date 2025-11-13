#include "challenge21.hpp"

#include "helper.hpp"
#include "print.hpp"

#include <algorithm>
#include <queue>
#include <ranges>
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

struct PossibleMoves {
    std::vector<Move> OneWay;
    std::vector<Move> OtherWay;
};

template<typename Key, std::size_t Rows, std::size_t Columns>
auto buildDistances(const std::array<std::array<Key, Columns>, Rows>& map) noexcept {
    std::unordered_map<FullMove<Key>, PossibleMoves, MyHash> distances;

    auto addMove = [&map, &distances](std::size_t beginRow, std::size_t beginColumn, std::size_t endRow,
                                      std::size_t endColumn) noexcept {
        FullMove<Key> move{map[beginRow][beginColumn], map[endRow][endColumn]};

        if ( move.To == Key::Invalid ) {
            return;
        } //if ( move.To == Key::Invalid )

        if ( move.To == move.From ) {
            distances.emplace(move, PossibleMoves{.OneWay{Move::Push}, .OtherWay{}});
        } //if ( move.To == move.From )

        const auto numberOfHorizontalSteps =
            static_cast<std::int64_t>(endColumn) - static_cast<std::int64_t>(beginColumn);
        const auto numberOfVerticalSteps = static_cast<std::int64_t>(endRow) - static_cast<std::int64_t>(beginRow);
        const bool isToRight             = numberOfHorizontalSteps >= 0;
        const bool isDown                = numberOfVerticalSteps >= 0;

        const auto horizontalSteps =
            std::views::repeat(isToRight ? Move::Right : Move::Left, std::abs(numberOfHorizontalSteps)) |
            std::ranges::to<std::vector>();
        const auto verticalSteps = std::views::repeat(isDown ? Move::Down : Move::Up, std::abs(numberOfVerticalSteps)) |
                                   std::ranges::to<std::vector>();

        //First horizontal, then vertical.
        PossibleMoves moves{std::views::concat(horizontalSteps, verticalSteps, std::array{Move::Push}) |
                                std::ranges::to<std::vector>(),
                            std::views::concat(verticalSteps, horizontalSteps, std::array{Move::Push}) |
                                std::ranges::to<std::vector>()};

        //From and to switched.
        PossibleMoves reversedMoves{moves.OtherWay | std::views::transform(invert) | std::ranges::to<std::vector>(),
                                    moves.OneWay | std::views::transform(invert) | std::ranges::to<std::vector>()};

        if ( map[beginRow][endColumn] == Key::Invalid ) {
            //Would hit the empty space!
            moves.OneWay.clear();
            reversedMoves.OneWay.clear();
        } //if ( map[beginRow][endColumn] == Key::Invalid )

        if ( map[endRow][beginColumn] == Key::Invalid ) {
            moves.OtherWay.clear();
            reversedMoves.OtherWay.clear();
        } //if ( map[endRow][beginColumn] != Key::Invalid )

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

Moves findShortestMove(FullMove<NumericKey> move, std::size_t robots) {
    struct X {
        Moves MovesSoFar;
        int   LevelsToGo;

        bool operator<(const X& that) const noexcept {
            return MovesSoFar.size() > that.MovesSoFar.size();
        }

        bool operator==(const X& that) const noexcept = default;
    };

    struct Hash {
        static std::size_t operator()(const X& entry) noexcept {
            std::hash<int> h;
            return h(static_cast<int>(entry.MovesSoFar.size())) ^ h(entry.LevelsToGo);
        }
    };

    static std::unordered_map<X, Moves, Hash> cache;
    std::priority_queue<X>                    queue;

    const auto& possibleMoves = Distances<NumericKey>.at(move);
    if ( !possibleMoves.OneWay.empty() ) {
        queue.emplace(possibleMoves.OneWay, robots);
    } //if ( !possibleMoves.OneWay.empty() )

    if ( !possibleMoves.OtherWay.empty() ) {
        queue.emplace(possibleMoves.OtherWay, robots);
    } //if ( !possibleMoves.OtherWay.empty() )

    throwIfInvalid(!queue.empty());

    while ( !queue.empty() ) {
        const auto current = queue.top();
        queue.pop();

        if ( current.LevelsToGo == 0 ) {
            return std::move(current.MovesSoFar);
        } //if ( current.LevelsToGo == 0 )

        if ( const auto iter = cache.find(current); iter != cache.end() ) {
            queue.emplace(iter->second, current.LevelsToGo - 1);
            continue;
        } //if ( const auto iter = cache.find(current); iter != cache.end() )

        auto addToState = [end = std::size(current.MovesSoFar), nextLevel = current.LevelsToGo - 1, &queue, &current](
                              this const auto& self, std::size_t index = 0, const Moves& moves = {}) noexcept -> void {
            if ( index == end ) {
                queue.emplace(moves, nextLevel);
                cache.emplace(current, moves);
                return;
            } //if ( index == end )

            FullMove<Move> localMove{index == 0 ? Move::Push : current.MovesSoFar[index - 1],
                                     current.MovesSoFar[index]};
            const auto&    nextPossibleMoves = Distances<Move>.at(localMove);

            if ( !nextPossibleMoves.OneWay.empty() ) {
                self(index + 1, std::views::concat(moves, nextPossibleMoves.OneWay) | std::ranges::to<std::vector>());
            } //if ( !nextPossibleMoves.OneWay.empty() )

            if ( !nextPossibleMoves.OtherWay.empty() ) {
                self(index + 1, std::views::concat(moves, nextPossibleMoves.OtherWay) | std::ranges::to<std::vector>());
            } //if ( !nextPossibleMoves.OtherWay.empty() )
            return;
        };

        addToState();
    } //while ( !queue.empty() )

    fail();
}

std::int64_t getShortestSequenceLength(std::string_view code, std::size_t robots) {
    throwIfInvalid(code.size() == 4);
    throwIfInvalid(code.back() == 'A');
    std::array<NumericKey, 4> sequence;
    sequence.back() = NumericKey::A;
    std::ranges::transform(code.substr(0, 3), sequence.begin(),
                           [](char c) noexcept { return static_cast<NumericKey>(c - '0'); });

    Moves      moves;
    NumericKey at = NumericKey::A;

    //myPrint("Moves:");
    for ( auto to : sequence ) {
        auto nextMove = findShortestMove({at, to}, robots);
        //myPrint(" ");
        std::ranges::for_each(nextMove, &print);
        myFlush();
        moves.append_range(std::move(nextMove));
        at = to;
    } //for ( auto to : sequence )
    myPrint("\n");

    return std::ssize(moves);
}

template<std::size_t Robots>
std::int64_t getComplexity(std::string_view code) noexcept {
    auto length  = getShortestSequenceLength(code, Robots);
    auto numeric = convert(code);
    myPrint("Length: {:3d} Numeric: {:3d}\n", length, numeric);
    return length * numeric;
}
} //namespace

bool challenge21(const std::vector<std::string_view>& input) {
    const auto sum1 = std::ranges::fold_left(input | std::views::transform(getComplexity<2>), 0, std::plus<>{});
    myPrint(" == Result of Part 1: {:d} ==\n", sum1);
    myFlush();

    const auto sum2 = std::ranges::fold_left(input | std::views::transform(getComplexity<25>), 0, std::plus<>{});;
    myPrint(" == Result of Part 2: {:d} ==\n", sum2);

    return sum1 == 238078 && sum2 == 636'350'496'972'143;
}
