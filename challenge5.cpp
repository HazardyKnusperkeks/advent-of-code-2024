#include "challenge5.hpp"

#include "helper.hpp"
#include "print.hpp"

#include <algorithm>
#include <functional>
#include <iterator>
#include <numeric>
#include <ranges>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

namespace {
using Update = std::vector<std::int64_t>;

struct Data {
    std::unordered_multimap<std::int64_t, std::int64_t> ReverseOrderings;
    std::vector<Update>                                 Updates;
};

Data parse(const std::vector<std::string_view>& input) noexcept {
    auto toReverseOrdering = [](std::string_view line) noexcept {
        auto pipe = line.find('|');
        return std::pair{convert(line.substr(pipe + 1)), convert(line.substr(0, pipe))};
    };

    auto toUpdate = [](std::string_view line) noexcept {
        return splitString(line, ',') | std::views::transform(&convert<10>) | std::ranges::to<std::vector>();
    };

    Data      ret;
    std::span span          = input;
    auto      orderingInput = span | std::views::take_while(std::not_fn(&std::string_view::empty));
    auto result = std::ranges::transform(orderingInput, std::inserter(ret.ReverseOrderings, ret.ReverseOrderings.end()),
                                         toReverseOrdering);
    span        = std::span{std::next(result.in), span.end()};
    ret.Updates.resize(span.size());
    std::ranges::transform(span, ret.Updates.begin(), toUpdate);

    return ret;
}
} //namespace

bool challenge5(const std::vector<std::string_view>& input) {
    auto data               = parse(input);

    auto isCorrectlyOrdered = [&data](const Update& update) noexcept {
        std::unordered_set<std::int64_t> alreadyPrinted;
        alreadyPrinted.reserve(update.size());
        return std::ranges::all_of(update | std::views::reverse, [&alreadyPrinted, &data](std::int64_t page) noexcept {
            const auto pagesToComeBeforeThis      = data.ReverseOrderings.equal_range(page);
            const auto orderingConstraintViolated = std::ranges::any_of(
                std::ranges::subrange{pagesToComeBeforeThis.first, pagesToComeBeforeThis.second} | std::views::values,
                [&alreadyPrinted](std::int64_t pageToComeBefore) { return alreadyPrinted.contains(pageToComeBefore); });

            if ( orderingConstraintViolated ) {
                return false;
            } //if ( orderingConstraintViolated )

            alreadyPrinted.insert(page);
            return true;
        });
    };

    auto midPoint = [](const Update& update) noexcept {
        return *std::midpoint(std::addressof(update.front()), std::addressof(update.back()));
    };

    auto sum1 = std::ranges::fold_left(
        data.Updates | std::views::filter(isCorrectlyOrdered) | std::views::transform(midPoint), 0, std::plus<>{});
    myPrint(" == Result of Part 1: {:d} ==\n", sum1);

    auto bringToCorrectOrder = [&data](Update& update) noexcept {
        auto begin = update.begin();
        auto end   = update.end();
        for ( auto i = begin; i != end; ) {
            auto page = *i;
            auto                  equalRange = data.ReverseOrderings.equal_range(page);
            std::ranges::subrange pagesToComeBefore{equalRange.first, equalRange.second};
            auto j = std::ranges::find_if(std::next(i), end, [pagesToComeBefore](std::int64_t afterPage) noexcept {
                return std::ranges::contains(pagesToComeBefore | std::views::values, afterPage);
            });
            if ( j == end ) {
                ++i;
            } //if ( j == end )
            else {
                std::ranges::iter_swap(i, j);
            } //else -> if ( j == end )
        } //for ( auto i = update.rbegin(); i != end; )
        return update;
    };

    auto sum2 = std::ranges::fold_left(data.Updates | std::views::filter(std::not_fn(isCorrectlyOrdered)) |
                                           std::views::transform(bringToCorrectOrder) | std::views::transform(midPoint),
                                       0, std::plus<>{});
    myPrint(" == Result of Part 2: {:d} ==\n", sum2);

    return sum1 == 5391 && sum2 == 6142;
}
