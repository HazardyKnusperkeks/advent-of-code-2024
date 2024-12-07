#include "challenge7.hpp"

#include "helper.hpp"
#include "print.hpp"

#include <algorithm>
#include <ranges>

namespace {
struct Equation {
    std::vector<std::int64_t> Operands;
    std::int64_t              Result;
};

auto parse(const std::vector<std::string_view>& input) {
    auto toEquation = [](std::string_view line) {
        Equation ret;
        auto     colon = line.find(':');
        ret.Result     = convert(line.substr(0, colon));
        ret.Operands   = splitString(line.substr(colon + 1), ' ') | std::views::transform(convert<10>) |
                       std::ranges::to<std::vector>();
        return ret;
    };
    return input | std::views::transform(toEquation) | std::ranges::to<std::vector>();
}

template<bool WithConcatenation>
bool isEquationValidImpl(const std::int64_t expectedResult, const std::int64_t resultSoFar,
                         std::span<const std::int64_t> remainingOperands) noexcept {
    if ( remainingOperands.empty() ) {
        return expectedResult == resultSoFar;
    } //if ( remainingOperands.empty() )

    if ( resultSoFar > expectedResult ) {
        return false;
    } //if ( resultSoFar > expectedResult )

    const auto front  = remainingOperands.front();
    remainingOperands = remainingOperands.subspan(1);

    if constexpr ( WithConcatenation ) {
        static constexpr auto logTable = [](void) noexcept {
            std::array<std::int64_t, 12> ret;
            ret[0] = 1;
            std::ranges::generate(ret | std::views::drop(1), [log = std::int64_t{1}](void) mutable noexcept {
                log *= 10;
                return log;
            });
            return ret;
        }();
        const auto log = std::ranges::upper_bound(logTable, front);
        if ( isEquationValidImpl<true>(expectedResult, resultSoFar * *log + front, remainingOperands) ) {
            return true;
        } //if ( isEquationValidImpl<true>(expectedResult, resultSoFar * *log + front, remainingOperands) )
    } //if constexpr ( WithConcatenation )

    if ( isEquationValidImpl<WithConcatenation>(expectedResult, resultSoFar + front, remainingOperands) ) {
        return true;
    } //if ( isEquationValidImpl<WithConcatenation>(expectedResult, resultSoFar + front, remainingOperands) )

    return isEquationValidImpl<WithConcatenation>(expectedResult, resultSoFar * front, remainingOperands);
}

template<bool WithConcatenation>
bool isEquationValid(const Equation& equation) noexcept {
    return isEquationValidImpl<WithConcatenation>(equation.Result, equation.Operands.front(),
                                                  std::span{equation.Operands}.subspan(1));
}
} //namespace

bool challenge7(const std::vector<std::string_view>& input) {
    auto equations = parse(input);
    const auto sum1      = std::ranges::fold_left(equations | std::views::filter(isEquationValid<false>) |
                                                      std::views::transform(&Equation::Result),
                                                  0, std::plus<>{});
    myPrint(" == Result of Part 1: {:d} ==\n", sum1);

    const auto sum2      = std::ranges::fold_left(equations | std::views::filter(isEquationValid<true>) |
                                                 std::views::transform(&Equation::Result),
                                             0, std::plus<>{});
    myPrint(" == Result of Part 2: {:d} ==\n", sum2);

    return sum1 == 6'083'020'304'036 && sum2 == 59'002'246'504'791;
}
