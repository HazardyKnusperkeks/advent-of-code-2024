#include "challenge17.hpp"

#include "helper.hpp"
#include "print.hpp"

#include <algorithm>
#include <iterator>
#include <numeric>
#include <ranges>

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

    std::string_view                          ProgramString;
    std::vector<std::int64_t>                 Program;
    std::vector<std::int64_t>::const_iterator Pc;
    std::string                               Output;

    void compute(void) noexcept {
        Pc = Program.begin();
        Output.clear();
        while ( Pc != Program.end() ) {
            perform(static_cast<Operation>(*Pc), static_cast<std::uint8_t>(*std::next(Pc)));
        } //while ( Pc != Program.end() )
        if ( !Output.empty() ) {
            Output.pop_back();
        } //if ( !Output.empty() )
        return;
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

    void perform(Operation op, std::uint8_t operand) noexcept {
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
            case out : {
                Output += static_cast<char>('0' + (getCombo() & 0x7));
                Output += ',';
                break;
            } //case out
            case jnz : {
                if ( A != 0 ) {
                    Pc = std::next(Program.begin(), operand);
                    return;
                } //if ( A != 0 )
                break;
            } //case jnz
        } //switch ( op )

        std::advance(Pc, 2);
        return;
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
    ret.Program =
        splitString(ret.ProgramString, ',') | std::views::transform(convert<10>) | std::ranges::to<std::vector>();

    return ret;
}

struct MyIota {
    std::int64_t Begin;
    std::int64_t End;

    MyIota(std::int64_t begin, std::int64_t end) noexcept : Begin{begin}, End{end} {
        return;
    }

    struct iterator {
        using value_type        = std::int64_t;
        using reference_type    = value_type;
        using pointer_type      = const value_type*;
        using iterator_category = std::random_access_iterator_tag;
        using difference_type   = value_type;

        std::int64_t Current;

        auto operator<=>(const iterator&) const noexcept = default;

        reference_type operator*(void) const noexcept {
            return Current;
        }

        iterator& operator++(void) noexcept {
            ++Current;
            return *this;
        }

        iterator operator++(int) noexcept {
            auto ret{*this};
            operator++();
            return ret;
        }

        iterator& operator--(void) noexcept {
            --Current;
            return *this;
        }

        iterator operator--(int) noexcept {
            auto ret{*this};
            operator--();
            return ret;
        }

        difference_type operator-(const iterator& that) const noexcept {
            return Current - that.Current;
        }

        iterator& operator+=(difference_type d) noexcept {
            Current += d;
            return *this;
        }

        iterator operator+(difference_type d) const noexcept {
            auto ret{*this};
            return ret += d;
        }

        friend iterator operator+(difference_type d, iterator i) noexcept {
            return i += d;
        }

        iterator& operator-=(difference_type d) noexcept {
            Current -= d;
            return *this;
        }

        iterator operator-(difference_type d) const noexcept {
            auto ret{*this};
            return ret -= d;
        }

        reference_type operator[](difference_type d) const noexcept;
    };

    iterator begin(void) const noexcept {
        return {Begin};
    }

    iterator end(void) const noexcept {
        return {End};
    }
};

static_assert(std::random_access_iterator<MyIota::iterator>);

std::int64_t findCopyA(Computer& computer) noexcept {
    //Does not work, I'm out of ideas.
    return 0;
    const auto programLength = computer.ProgramString.size();

    int  prefix              = 127;
    auto run                 = [&computer, &prefix](std::int64_t a) noexcept {
        computer.A = a;
        computer.B = 0;
        computer.C = 0;
        computer.compute();
        //myPrint("{:2d} Running {:16d} => {:s}\n", prefix, a, computer.Output);
        return;
    };

    auto mapToLength = [&run, &computer](std::int64_t a) noexcept {
        run(a);
        return computer.Output.size();
    };

    auto fullRange = MyIota{100, 100'000'000'000'000'000};
    auto sizeRange = std::ranges::equal_range(fullRange, programLength, {}, mapToLength);
    myPrint("\n\nFound Range: {:d}, {:d}\n\n", sizeRange.front(), sizeRange.back() - 1);

    struct Range {
        std::int64_t Lower;
        std::int64_t Upper;

        auto length(void) const noexcept {
            return Upper - Lower;
        }

        auto midpoint(void) const noexcept {
            return std::midpoint(Lower, Upper);
        }
    };

    std::vector<Range> rangesToSearch{{sizeRange.front(), sizeRange.back()}};

    for ( auto searchForIndex = programLength - 1; searchForIndex < programLength; searchForIndex -= 2 ) {
        //prefix                   = searchForIndex;
        //Looking for the range with the correct index.
        const auto digitToSearch = computer.ProgramString[searchForIndex];
        run(rangesToSearch.front().Lower);
        auto lowerResult = computer.Output;
        run(rangesToSearch.front().Upper - 1);
        myPrint("\nLooking for Index {:d}: {:c} in\n{:16c} {:>{}c}\n{:16d} {:s}\n{:16d} {:s}\n", searchForIndex,
                digitToSearch, ' ', 'v', searchForIndex + 1, rangesToSearch.front().Lower, lowerResult,
                rangesToSearch.front().Upper - 1, computer.Output);

        do { //while ( rangesToSearch.size() > 1 )
            for ( auto index = rangesToSearch.size() - 1; index <= rangesToSearch.size(); --index ) {
                auto range    = rangesToSearch[index];
                auto midpoint = range.midpoint();
                run(midpoint);

                if ( computer.Output[searchForIndex] == digitToSearch ) {
                    myPrint("Matched Midpoint:\n{:16d} {:s}\n", midpoint, computer.Output);
                    //The correct value is in this range.
                    const bool addLeft = index > 0 && (run(rangesToSearch[index - 1].Upper - 1),
                                                       computer.Output[searchForIndex] == digitToSearch);
                    const bool addRight =
                        index < rangesToSearch.size() - 1 &&
                        (run(rangesToSearch[index + 1].Lower), computer.Output[searchForIndex] == digitToSearch);

                    if ( addLeft ) {
                        range.Lower = rangesToSearch[index - 1].Lower;
                    } //if ( addLeft )

                    if ( addRight ) {
                        range.Upper = rangesToSearch[index + 1].Upper;
                    } //if ( addRight )

                    rangesToSearch.clear();
                    rangesToSearch.push_back(range);

                    if ( addLeft || addRight ) {
                        index = rangesToSearch.size();
                        continue;
                    } //if ( addLeft || addRight )
                    break;
                } //if ( computer.Output[searchForIndex] == digitToSearch )

                rangesToSearch[index].Upper = midpoint;
                rangesToSearch.insert(std::next(rangesToSearch.begin(), index + 1), {midpoint + 1, range.Upper});
            } //for ( auto index = rangesToSearch.size() - 1; index <= rangesToSearch.size(); --index )
        } while ( rangesToSearch.size() > 1 );

        auto& theRange = rangesToSearch.front();

        run(rangesToSearch.front().Lower);
        lowerResult = computer.Output;
        run(rangesToSearch.front().Upper - 1);
        myPrint("Limited Range #1 to {:d} {:d} ({:d})\n", theRange.Lower, theRange.Upper - 1, theRange.length());
        myPrint("{:16d} {:s}\n{:16d} {:s}\n", theRange.Lower, lowerResult, theRange.Upper - 1, computer.Output);
        //Move lower up.
        MyIota lowerRange{theRange.Lower, theRange.midpoint()};
        auto   lowerBorder = std::ranges::lower_bound(
            lowerRange, true, {}, [&run, &computer, &digitToSearch, &searchForIndex](std::int64_t a) noexcept {
                run(a);
                return computer.Output[searchForIndex] == digitToSearch;
            });

        //myPrint("Found Lower\n");

        //Move upper down.
        MyIota upperRange{theRange.midpoint(), theRange.Upper};
        auto   upperBorder = std::ranges::lower_bound(
            upperRange, true, {}, [&run, &computer, &digitToSearch, &searchForIndex](std::int64_t a) noexcept {
                run(a);
                return computer.Output[searchForIndex] != digitToSearch;
            });
        theRange.Lower = *lowerBorder;
        theRange.Upper = *std::prev(upperBorder);

        //myPrint("Found Upper\n");
        run(rangesToSearch.front().Lower);
        lowerResult = computer.Output;
        run(rangesToSearch.front().Upper - 1);
        const auto upperResult = computer.Output;
        myPrint("Limited Range #2 to {:d} {:d} ({:d})\n", theRange.Lower, theRange.Upper - 1, theRange.length());
        myPrint("{:16d} {:s}\n{:16d} {:s}\n", theRange.Lower, lowerResult, theRange.Upper - 1, computer.Output);
        myFlush();

        if ( theRange.length() < 50000 && false ) {
            //Go for linear.
            for ( ; computer.Output != computer.ProgramString; ++theRange.Lower ) {
                run(theRange.Lower);
            } //for ( ; computer.Output != computer.ProgramString; ++theRange.Lower )

            return theRange.Lower;
        } //if ( theRange.length() < 50000 )
    } //for ( auto searchForIndex = programLength - 1; rangesToSearch.front().length() != 0; searchForIndex -= 2 )

    return 0;
}
} //namespace

bool challenge17(const std::vector<std::string_view>& input) {
    auto computer = parse(input);

    computer.compute();
    myPrint(" == Result of Part 1: {:s} ==\n", computer.Output);

    const auto copyA = findCopyA(computer);
    myPrint(" == Result of Part 2: {:d} ==\n", copyA);

    return computer.Output == "1,5,0,1,7,4,1,0,3"sv && copyA == 1171;
}
