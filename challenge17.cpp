#include "challenge17.hpp"

#include "helper.hpp"
#include "print.hpp"

#include <iterator>
#include <ranges>
#include <set>
#include <unordered_map>

using namespace std::string_view_literals;

namespace {
enum class ComboOperand : std::uint8_t {
    Zero      = 0,
    One       = 1,
    Two       = 2,
    Three     = 3,
    RegisterA = 4,
    RegisterB = 5,
    RegisterC = 6,
    Invalid   = 7
};

enum class Operation : std::uint8_t { adv = 0, bxl = 1, bst = 2, jnz = 3, bxc = 4, out = 5, bdv = 6, cdv = 7 };

struct Computer {
    std::int64_t A;
    std::int64_t B;
    std::int64_t C;

    std::string_view                         ProgramString;
    std::vector<std::int8_t>                 Program;
    std::vector<std::int8_t>::const_iterator Pc;
    std::vector<std::int8_t>                 Output;

    bool performNextOperation(void) noexcept {
        return perform(static_cast<Operation>(*Pc), static_cast<std::uint8_t>(*std::next(Pc)));
    }

    std::string compute(void) noexcept {
        Pc = Program.begin();
        Output.clear();
        while ( Pc != Program.end() ) {
            performNextOperation();
        } //while ( Pc != Program.end() )

        auto output = Output | std::views::transform([](std::int8_t c) {
                          std::string ret{"0,"};
                          ret[0] += c;
                          return ret;
                      }) |
                      std::views::join | std::ranges::to<std::string>();
        if ( !output.empty() ) {
            output.pop_back();
        } //if ( !output.empty() )
        return output;
    }

    void reset(std::int64_t a) noexcept {
        A = a;
        B = 0;
        C = 0;
        Output.clear();
        Pc = Program.begin();
        return;
    }

    bool computeToOut(void) noexcept {
        for ( auto keepGoing = true; keepGoing && Pc != Program.end(); ) {
            keepGoing = !performNextOperation();
        } //for ( auto keepGoing = true; keepGoing && Pc != Program.end(); )
        return Pc != Program.end();
    }

    void tryToMatchProgram(void) noexcept {
        for ( auto index = 0zu; index < Program.size(); ++index ) {
            if ( !computeToOut() ) {
                break;
            }
            if ( Output[index] != Program[index] ) {
                break;
            }
        }
    }

    std::int64_t getValue(ComboOperand operand) const noexcept {
        switch ( operand ) {
            using enum ComboOperand;
            case Zero      : break;
            case One       : return 1;
            case Two       : return 2;
            case Three     : return 3;
            case RegisterA : return A;
            case RegisterB : return B;
            case RegisterC : return C;
            case Invalid   : throwIfInvalid(false);
        } //switch ( operand )
        return 0;
    }

    bool perform(Operation op, std::uint8_t operand) noexcept {
        auto getCombo = [this, &operand](void) noexcept { return getValue(static_cast<ComboOperand>(operand)); };
        auto div      = [this, &getCombo](void) noexcept { return A / (1 << getCombo()); };

        switch ( op ) {
            using enum Operation;
            case adv : A = div(); break;
            case bdv : B = div(); break;
            case cdv : C = div(); break;
            case bxl : B ^= operand; break;
            case bst : B = getCombo() & 0x7; break;
            case bxc : B ^= C; break;
            case out : Output.push_back(getCombo() & 0x7); break;
            case jnz : {
                if ( A != 0 ) {
                    Pc = std::next(Program.begin(), operand);
                    return false;
                } //if ( A != 0 )
                break;
            } //case jnz
        } //switch ( op )

        std::advance(Pc, 2);
        return op == Operation::out;
    }
};

Computer parse(std::span<const std::string_view> input) {
    throwIfInvalid(input.size() == 5);
    auto aLine        = input[0];
    auto bLine        = input[1];
    auto cLine        = input[2];

    auto readRegister = [](std::string_view line, std::int64_t& reg) {
        throwIfInvalid(line.starts_with("Register "));
        reg = convert(line.substr("Register A: "sv.size()));
        return;
    };

    Computer ret;
    readRegister(aLine, ret.A);
    readRegister(bLine, ret.B);
    readRegister(cLine, ret.C);

    auto code = input[4];
    throwIfInvalid(code.starts_with("Program: "));
    ret.ProgramString = code.substr("Program: "sv.size());
    ret.Program       = splitString(ret.ProgramString, ',') | std::views::transform(convert<10>) |
                  std::views::transform([](auto x) noexcept { return static_cast<std::int8_t>(x); }) |
                  std::ranges::to<std::vector>();

    return ret;
}

std::int64_t findCopyA(Computer& computer) {
    auto shift = 99u;
    for ( auto index = 0zu; index < computer.Program.size(); index += 2 ) {
        if ( computer.Program[index] == std::to_underlying(Operation::adv) ) {
            shift = static_cast<std::uint32_t>(computer.Program[index + 1]);
            break;
        } //if ( computer.Program[index] == std::to_underlying(Operation::adv) )
    } //for ( auto index = 0zu; index < computer.Program.size(); index += 2 )
    throwIfInvalid(shift <= 3);

    constexpr auto magic7    = 7u;
    const auto     inputBits = magic7 + shift;

    const auto cache         = std::views::iota(0, 1 << inputBits) | std::views::transform([&computer](std::int64_t a) {
                           computer.reset(a);
                           throwIfInvalid(computer.computeToOut());
                           return std::pair<std::int64_t, std::int8_t>{a, computer.Output.front()};
                       }) |
                       std::ranges::to<std::unordered_map>();

    std::set<std::int64_t> results;

    auto recurse = [&results, &cache, &computer, &shift, &inputBits](this auto& self, const std::int64_t a,
                                                                     const std::size_t index) noexcept {
        if ( index == computer.Program.size() ) {
            results.insert(a);
            return;
        } //if ( index == computer.Program.size() )

        for ( std::int64_t bits : std::views::iota(0, 1 << shift) ) {
            const auto nextShift = inputBits + (index - 1) * shift;
            const auto nextA     = a | (bits << nextShift);
            const auto input     = nextA >> (index * shift);

            if ( cache.at(input) == computer.Program[index] ) {
                self(nextA, index + 1);
            } //if ( cache.at(input) == computer.Program[index] )
        } //for ( auto bits : std::views::iota(0, 1 << shift) )
        return;
    };

    for ( auto [a, _] : cache | std::views::filter([&computer](const auto& pair) noexcept {
                            return pair.second == computer.Program.front();
                        }) ) {
        recurse(a, 1);
    } //for ( auto a : cache | std::views::filter() )

    throwIfInvalid(!results.empty());
    return *results.begin();
}
} //namespace

bool challenge17(const std::vector<std::string_view>& input) {
    auto computer     = parse(input);

    const auto output = computer.compute();
    myPrint(" == Result of Part 1: {:s} ==\n", output);

    const auto copyA = findCopyA(computer);
    myPrint(" == Result of Part 2: {:d} ==\n", copyA);

    return output == "1,5,0,1,7,4,1,0,3"sv && copyA == 47'910'079'998'866;
}
