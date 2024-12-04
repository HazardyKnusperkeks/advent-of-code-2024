#include "challenge4.hpp"

#include "helper.hpp"
#include "print.hpp"

#include <algorithm>
#include <ranges>

using namespace std::string_view_literals;

namespace {
class Column {
    public:
    class Field {
        public:
        using value_type        = char;
        using iterator_category = std::random_access_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using reference_type    = const char&;
        using pointer_type      = const char*;

        Field(void) noexcept : Base{nullptr}, Index{0} {
            return;
        }

        reference_type operator*(void) const noexcept {
            return (*(*Base).Base)[static_cast<std::size_t>(Index)][static_cast<std::size_t>(Base->Index)];
        }

        Field& operator++(void) noexcept {
            ++Index;
            return *this;
        }

        Field operator++(int) noexcept {
            Field ret{*this};
            operator++();
            return ret;
        }

        Field& operator--(void) noexcept {
            --Index;
            return *this;
        }

        Field operator--(int) noexcept {
            Field ret{*this};
            operator--();
            return ret;
        }

        difference_type operator-(const Field& that) const noexcept {
            return Index - that.Index;
        }

        Field& operator+=(difference_type offset) noexcept {
            Index += offset;
            return *this;
        }

        Field operator+(difference_type offset) const noexcept {
            return Field{*this} += offset;
        }

        friend Field operator+(difference_type offset, Field iter) noexcept {
            return iter += offset;
        }

        Field& operator-=(difference_type offset) noexcept {
            return operator+=(-offset);
        }

        Field operator-(difference_type offset) const noexcept {
            return Field{*this} -= offset;
        }

        reference_type operator[](difference_type offset) const noexcept {
            return *(Field{*this} += offset);
        }

        pointer_type operator->(void) const noexcept {
            return std::addressof(operator*());
        }

        auto operator<=>(const Field& that) const noexcept = default;

        private:
        const Column*  Base;
        std::ptrdiff_t Index;

        Field(const Column* base, std::ptrdiff_t index) noexcept : Base{base}, Index{index} {
            return;
        }

        friend class Column;
    };

    using value_type        = Column;
    using iterator_category = std::contiguous_iterator_tag;
    using difference_type   = std::ptrdiff_t;
    using reference_type    = const Column;
    using pointer_type      = const Column*;

    Column(void) noexcept : Column{nullptr, 0} {
        return;
    }

    Column(const std::vector<std::string_view>* base, std::ptrdiff_t index) noexcept : Base{base}, Index{index} {
        return;
    }

    Field begin(void) const noexcept {
        return Field{this, 0};
    }

    Field end(void) const noexcept {
        return Field{this, static_cast<std::ptrdiff_t>(Base->size())};
    }

    reference_type operator*(void) const noexcept {
        return *this;
    }

    Column& operator++(void) noexcept {
        ++Index;
        return *this;
    }

    Column operator++(int) noexcept {
        Column ret{*this};
        operator++();
        return ret;
    }

    Column& operator--(void) noexcept {
        --Index;
        return *this;
    }

    Column operator--(int) noexcept {
        Column ret{*this};
        operator--();
        return ret;
    }

    difference_type operator-(const Column& that) const noexcept {
        return Index - that.Index;
    }

    Column& operator+=(difference_type offset) noexcept {
        Index += offset;
        return *this;
    }

    Column operator+(difference_type offset) const noexcept {
        return Column{*this} += offset;
    }

    friend Column operator+(difference_type offset, Column iter) noexcept {
        return iter += offset;
    }

    Column& operator-=(difference_type offset) noexcept {
        return operator+=(-offset);
    }

    Column operator-(difference_type offset) const noexcept {
        return Column{*this} -= offset;
    }

    reference_type operator[](difference_type offset) const noexcept {
        return *(Column{*this} += offset);
    }

    pointer_type operator->(void) const noexcept {
        return this;
    }

    auto operator<=>(const Column& that) const noexcept = default;

    private:
    const std::vector<std::string_view>* Base;
    std::ptrdiff_t                       Index;
};

static_assert(std::random_access_iterator<Column::Field>);
static_assert(std::ranges::input_range<Column>);
static_assert(std::ranges::viewable_range<Column>);

class TransposedVector {
    public:
    explicit TransposedVector(const std::vector<std::string_view>& base) noexcept : Base{std::addressof(base)} {
        return;
    }

    Column begin(void) const noexcept {
        return {Base, 0};
    }

    Column end(void) const noexcept {
        return {Base, static_cast<std::ptrdiff_t>(Base->front().size())};
    }

    private:
    const std::vector<std::string_view>* Base;
};

static_assert(std::random_access_iterator<Column>);
static_assert(std::ranges::input_range<TransposedVector>);
static_assert(std::ranges::viewable_range<TransposedVector>);

template<bool DownAndRight>
class Diagonal {
    public:
    class Field {
        public:
        using value_type        = char;
        using iterator_category = std::random_access_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using reference_type    = const char&;
        using pointer_type      = const char*;

        Field(void) noexcept : Base{nullptr}, Offset{0} {
            return;
        }

        reference_type operator*(void) const noexcept {
            if constexpr ( DownAndRight ) {
                return (*(*Base).Base)[static_cast<std::size_t>(Base->StartRow + Offset)]
                                      [static_cast<std::size_t>(Base->StartColumn + Offset)];
            } //if constexpr ( DownAndRight )
            else {
                return (*(*Base).Base)[static_cast<std::size_t>(Base->StartRow + Offset)]
                                      [static_cast<std::size_t>(Base->StartColumn - Offset)];
            } //else -> if constexpr( DownAndRight )
        }

        Field& operator++(void) noexcept {
            ++Offset;
            return *this;
        }

        Field operator++(int) noexcept {
            Field ret{*this};
            operator++();
            return ret;
        }

        Field& operator--(void) noexcept {
            --Offset;
            return *this;
        }

        Field operator--(int) noexcept {
            Field ret{*this};
            operator--();
            return ret;
        }

        difference_type operator-(const Field& that) const noexcept {
            return Offset - that.Offset;
        }

        Field& operator+=(difference_type offset) noexcept {
            Offset += offset;
            return *this;
        }

        Field operator+(difference_type offset) const noexcept {
            return Field{*this} += offset;
        }

        friend Field operator+(difference_type offset, Field iter) noexcept {
            return iter += offset;
        }

        Field& operator-=(difference_type offset) noexcept {
            return operator+=(-offset);
        }

        Field operator-(difference_type offset) const noexcept {
            return Field{*this} -= offset;
        }

        reference_type operator[](difference_type offset) const noexcept {
            return *(Field{*this} += offset);
        }

        pointer_type operator->(void) const noexcept {
            return std::addressof(operator*());
        }

        auto operator<=>(const Field& that) const noexcept = default;

        private:
        const Diagonal* Base;
        std::ptrdiff_t  Offset;

        Field(const Diagonal* base, std::ptrdiff_t offset) noexcept : Base{base}, Offset{offset} {
            return;
        }

        friend class Diagonal;
    };

    using value_type        = Diagonal;
    using iterator_category = std::contiguous_iterator_tag;
    using difference_type   = std::ptrdiff_t;
    using reference_type    = const Diagonal;
    using pointer_type      = const Diagonal*;

    Diagonal(void) noexcept : Diagonal{nullptr, 0} {
        return;
    }

    Diagonal(const std::vector<std::string_view>* base, std::ptrdiff_t index) noexcept : Base{base}, Index{index} {
        updateStart();
        return;
    }

    Field begin(void) const noexcept {
        return Field{this, 0};
    }

    Field end(void) const noexcept {
        const auto size = std::ssize(*Base);
        return Field{this, size - std::abs(size - Index - 1)};
    }

    reference_type operator*(void) const noexcept {
        return *this;
    }

    Diagonal& operator++(void) noexcept {
        ++Index;
        updateStart();
        return *this;
    }

    Diagonal operator++(int) noexcept {
        Diagonal ret{*this};
        operator++();
        return ret;
    }

    Diagonal& operator--(void) noexcept {
        --Index;
        updateStart();
        return *this;
    }

    Diagonal operator--(int) noexcept {
        Diagonal ret{*this};
        operator--();
        return ret;
    }

    difference_type operator-(const Diagonal& that) const noexcept {
        return Index - that.Index;
    }

    Diagonal& operator+=(difference_type offset) noexcept {
        Index += offset;
        updateStart();
        return *this;
    }

    Diagonal operator+(difference_type offset) const noexcept {
        return Diagonal{*this} += offset;
    }

    friend Diagonal operator+(difference_type offset, Diagonal iter) noexcept {
        return iter += offset;
    }

    Diagonal& operator-=(difference_type offset) noexcept {
        return operator+=(-offset);
    }

    Diagonal operator-(difference_type offset) const noexcept {
        return Diagonal{*this} -= offset;
    }

    reference_type operator[](difference_type offset) const noexcept {
        return *(Diagonal{*this} += offset);
    }

    pointer_type operator->(void) const noexcept {
        return this;
    }

    auto operator<=>(const Diagonal& that) const noexcept = default;

    private:
    const std::vector<std::string_view>* Base;
    std::ptrdiff_t                       Index;
    std::ptrdiff_t                       StartRow;
    std::ptrdiff_t                       StartColumn;

    void updateStart(void) noexcept {
        StartRow = std::max(std::ssize(*Base) - Index - 1, 0z);
        if constexpr ( DownAndRight ) {
            if ( StartRow == 0 ) {
                StartColumn = Index - std::ssize(*Base) + 1;
            } //if ( StartRow == 0 )
            else {
                StartColumn = 0;
            } //else -> if ( StartRow == 0 )
        } //if constexpr ( DownAndRight )
        else {
            if ( StartRow == 0 ) {
                StartColumn = 2 * std::ssize(*Base) - Index - 2;
            } //if ( StartRow == 0 )
            else {
                StartColumn = std::ssize(*Base) - 1;
            } //else -> if ( StartRow == 0 )
        } //else -> if constexpr( DownAndRight )
        return;
    }
};

static_assert(std::random_access_iterator<Diagonal<true>::Field>);
static_assert(std::ranges::input_range<Diagonal<true>>);
static_assert(std::ranges::viewable_range<Diagonal<true>>);
static_assert(std::random_access_iterator<Diagonal<false>::Field>);
static_assert(std::ranges::input_range<Diagonal<false>>);
static_assert(std::ranges::viewable_range<Diagonal<false>>);

template<bool DownAndRight>
class DiagonalVector {
    public:
    explicit DiagonalVector(const std::vector<std::string_view>& base) noexcept : Base{std::addressof(base)} {
        return;
    }

    Diagonal<DownAndRight> begin(void) const noexcept {
        return {Base, 0};
    }

    Diagonal<DownAndRight> end(void) const noexcept {
        return {Base, static_cast<std::ptrdiff_t>(Base->front().size() * 2) - 1};
    }

    private:
    const std::vector<std::string_view>* Base;
};

static_assert(std::random_access_iterator<Diagonal<true>>);
static_assert(std::ranges::input_range<DiagonalVector<true>>);
static_assert(std::ranges::viewable_range<DiagonalVector<true>>);
static_assert(std::random_access_iterator<Diagonal<false>>);
static_assert(std::ranges::input_range<DiagonalVector<false>>);
static_assert(std::ranges::viewable_range<DiagonalVector<false>>);

template<typename Input>
std::int64_t count(const Input& input, std::string_view needle) noexcept {
    std::int64_t ret               = 0;

    std::ranges::subrange haystack = input;

    while ( true ) {
        auto found = std::ranges::search(haystack, needle);
        if ( found.begin() == haystack.end() ) {
            return ret;
        } //if ( found.begin() == haystack.end() )

        ++ret;
        haystack = {found.end(), haystack.end()};
    } //while ( true )
}
} //namespace

bool challenge4(const std::vector<std::string_view>& input) {
    auto countXmas   = [](const auto& x) noexcept { return count(x, "XMAS"sv); };
    auto countSamx   = [](const auto& x) noexcept { return count(x, "SAMX"sv); };

    auto leftToRight = std::ranges::fold_left(input | std::views::transform(countXmas), 0, std::plus<>{});
    auto rightToLeft = std::ranges::fold_left(input | std::views::transform(countSamx), 0, std::plus<>{});

    TransposedVector transposed{input};

    auto topToBottom = std::ranges::fold_left(transposed | std::views::transform(countXmas), 0, std::plus<>{});
    auto bottomToTop = std::ranges::fold_left(transposed | std::views::transform(countSamx), 0, std::plus<>{});

    DiagonalVector<true>  diagonalRight{input};
    DiagonalVector<false> diagonalLeft{input};

    auto downAndRight = std::ranges::fold_left(diagonalRight | std::views::transform(countXmas), 0, std::plus<>{});
    auto upAndLeft    = std::ranges::fold_left(diagonalRight | std::views::transform(countSamx), 0, std::plus<>{});

    auto downAndLeft  = std::ranges::fold_left(diagonalLeft | std::views::transform(countXmas), 0, std::plus<>{});
    auto upAndRight   = std::ranges::fold_left(diagonalLeft | std::views::transform(countSamx), 0, std::plus<>{});

    const auto sum1 =
        leftToRight + rightToLeft + topToBottom + bottomToTop + downAndRight + upAndLeft + downAndLeft + upAndRight;
    myPrint(" == Result of Part 1: {:d} ==\n", sum1);

    auto countCrossMas = [&input](auto lineAndNumber) noexcept {
        auto [line, rowNumber] = lineAndNumber;
        auto isCross           = [&input, rowNumber](auto charAndColumn) noexcept {
            auto [c, columnNumber] = charAndColumn;
            if ( c != 'A' ) {
                return false;
            } //if ( c != 'A' )

            const Coordinate a{rowNumber, columnNumber};
            const auto       ul = a.up().left();
            const auto       ur = a.up().right();
            const auto       dl = a.down().left();
            const auto       dr = a.down().right();

            auto at = [&input](Coordinate<std::size_t> coord) noexcept { return input[coord.Row][coord.Column]; };

            return ((at(ul) == 'M' && at(dr) == 'S') || (at(ul) == 'S' && at(dr) == 'M')) &&
                   ((at(ur) == 'M' && at(dl) == 'S') || (at(ur) == 'S' && at(dl) == 'M'));
        };
        return std::ranges::count_if(
            std::views::zip(line | std::views::drop(1), std::views::iota(1zu, line.size() - 1)), isCross);
    };

    const auto sum2 =
        std::ranges::fold_left(std::views::zip(input | std::views::drop(1), std::views::iota(1zu, input.size() - 1)) |
                                   std::views::transform(countCrossMas),
                               0, std::plus<>{});
    myPrint(" == Result of Part 2: {:d} ==\n", sum2);

    return sum1 == 2557 && sum2 == 1854;
}
