#include <u/utilities.h>

// #include <u/expected.h>
#include <u/diagnostics/result.h>

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
	u::result<bool, bool> result;
	if (int value = *result; result.has_value()) {
		u::discard(value);
	} else {
		bool error = result.error();
		result.error_or(true);
	}

	u::discard(argc, argv);
	return 0;
}
