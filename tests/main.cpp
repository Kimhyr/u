#include <u/utilities.h>

#include <u/expected.h>
// #include <core/diagnostics/result.h>

template<typename T>
class foo
{
public:
    auto bar() -> void
    {
        float* baz;
        const_cast<foo*>(baz);
    }
};

auto main(int argc, char** argv) -> int
{
    //std::expected<int, int> e;
    //e.value();

    core::result<int, int> hello{};
    hello.value();

    hello.value_or(14);

    core::discard(argc, argv);
    return 0;
}
