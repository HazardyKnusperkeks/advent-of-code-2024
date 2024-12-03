#include "challenge3.hpp"

#include "helper.hpp"
#include "print.hpp"

#include "3rdParty/ctre/include/ctre.hpp"

#include <algorithm>
#include <ranges>

using namespace std::string_view_literals;

namespace {
std::int64_t findMul(std::string_view text) noexcept {
    constexpr auto regEx = ctre::search_all<R"(mul\((\d{1,3}),(\d{1,3})\))">;
    auto mul = [](auto match) noexcept { return convert(match.template get<1>()) * convert(match.template get<2>()); };
    return std::ranges::fold_left(regEx(text) | std::views::transform(mul), 0, std::plus<>{});
}

std::int64_t findMulWithEnabled(std::string_view text, bool& enabled) noexcept {
    constexpr auto regEx       = ctre::search_all<R"((mul\((\d{1,3}),(\d{1,3})\)|do\(\)|don't\(\)))">;
    auto           onlyEnabled = [&enabled](auto match) noexcept {
        if ( match.template get<0>().view() == "do()"sv ) {
            enabled = true;
            return false;
        } //if ( match.template get<0>().view() == "do()"sv )

        if ( match.template get<0>().view() == "don't()"sv ) {
            enabled = false;
            return false;
        } //if ( match.template get<0>().view() == "don't()"sv )

        return enabled;
    };
    auto mul = [](auto match) noexcept { return convert(match.template get<2>()) * convert(match.template get<3>()); };
    return std::ranges::fold_left(regEx(text) | std::views::filter(onlyEnabled) | std::views::transform(mul), 0, std::plus<>{});
}
} //namespace

bool challenge3(const std::vector<std::string_view>& input) {
    const auto sum1 = std::ranges::fold_left(input | std::views::transform(findMul), 0, std::plus<>{});

    myPrint(" == Result of Part 1: {:d} ==\n", sum1);

    bool       enabled = true;
    const auto sum2    = std::ranges::fold_left(
        input | std::views::transform([&enabled](auto text) noexcept { return findMulWithEnabled(text, enabled); }), 0,
        std::plus<>{});
    myPrint(" == Result of Part 2: {:d} ==\n", sum2);

    return sum1 == 173419328 && sum2 == 90669332;
}
