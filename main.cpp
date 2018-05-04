#include "views.hpp"
#include <string>

int main() {
	std::vector<int> a = {1, 2, 3};

	auto rs = a | view::remove_if([](int i) {return false;})
				| view::remove_if([](int i) {return i % 2 == 0;})
				| view::transform([](int i) {return std::to_string(i);});

	for (auto &el : rs) {
		std::cout << el << ' ';
	}

	std::cout << std::endl;
}