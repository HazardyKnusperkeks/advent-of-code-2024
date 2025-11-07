#include "challenge25.hpp"

#include "helper.hpp"
#include "print.hpp"

#include <algorithm>
#include <array>
#include <ranges>

namespace {
using HeighList = std::array<std::int8_t, 5>;

using Key       = HeighList;
using Lock      = HeighList;

struct Data {
    std::vector<Key>  Keys;
    std::vector<Lock> Locks;
};

Data parse(const std::vector<std::string_view>& input) {
    enum State { Start, Row1, Row2, Row3, Row4, Row5, Row6, Row7, Empty } state = Row1;

    Data      ret;
    HeighList current   = {};
    auto*     listToAdd = &ret.Keys;

    for ( auto row : input ) {
        switch ( state ) {
            case Start : std::unreachable();

            case Empty : {
                throwIfInvalid(row.empty());
                state = Start;
                break;
            } //case Empty

            case Row1 : {
                throwIfInvalid(row.size() == 5);
                listToAdd = row.front() == '.' ? &ret.Keys : &ret.Locks;
                break;
            } //case Row1

            case Row7 : {
                throwIfInvalid(row.size() == 5);
                listToAdd->emplace_back(current);
                current.fill(0);
                break;
            } //case Row7

            default : {
                throwIfInvalid(row.size() == 5);
                for ( auto&& [height, element] : std::views::zip(current, row) ) {
                    if ( element == '#' ) {
                        ++height;
                    } //if ( element == '#' )
                } //for ( auto&& [height, element] : std::views::zip(current, row) )
                break;
            } //default
        } //switch ( state )

        state = static_cast<State>(state + 1);
    } //for ( auto row : input )

    std::ranges::sort(ret.Keys);
    std::ranges::sort(ret.Locks);

    return ret;
}

bool doesNotOverlap(std::int8_t a, std::int8_t b) {
    constexpr auto OverlapThreshold = 5;
    return a + b <= OverlapThreshold;
}

bool doesNotOverlapTuple(std::tuple<std::int8_t, std::int8_t> t) {
    return doesNotOverlap(std::get<0>(t), std::get<1>(t));
}

std::int64_t countPossibleKeyMatches(const Data& data) {
    auto countMatchesPerKey = [&data](const Key& key) {
        const auto firstPinMatches = [&key](const Lock& lock) { return doesNotOverlap(key.front(), lock.front()); };
        const auto keyMatches = [&key](const Lock& lock) {
            return std::ranges::all_of(std::views::zip(key, lock) | std::views::drop(1), doesNotOverlapTuple);
        };
        return std::ranges::count_if(data.Locks | std::views::take_while(firstPinMatches), keyMatches);
    };
    return std::ranges::fold_left(data.Keys | std::views::transform(countMatchesPerKey), 0, std::plus<>{});
}
} //namespace

bool challenge25(const std::vector<std::string_view>& input) {
    const auto data = parse(input);

    //auto [group1, group2] = calculateCut(graph, 3);
    //auto groupSizeProduct = group1.size() * group2.size();
    const auto result = countPossibleKeyMatches(data);
    myPrint(" == Result of Part 1: {:d} ==\n", result);

    return result == 3255;
}
