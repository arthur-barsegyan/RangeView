#include "views.hpp"
#include <string>

int main() {
	std::vector<int> a = {1, 2, 3};
	std::function<bool(int)> f = [](int i) {return false;}; 
	std::function<std::string(int)> f1 = [](int i) {return std::to_string(i);};

	auto rs = a | view::remove_if(f)
				| view::transform(f1);

	for (auto &el : rs) {
		std::cout << el << ' ';
	}

	std::cout << std::endl;
}