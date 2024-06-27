#include "yacht.h"
#include "ustl.h"

#include <iostream>
#include <atomic>
#include <regex>
#include <vector>
#include <string>
#include <utility>
#include <exception>
#include <optional>

struct Dummy {
	int i, j;

	Dummy() : i(0), j(0) {
		std::cout << "[Construct] Dummy default construct" << "\n";
	};

	Dummy(int i, int j) : i(i), j(j) {
		std::cout << "[Construct] Dummy construct" << "\n";
	}

	Dummy(const Dummy& other) : i(other.i), j(other.j) {
		std::cout << "[Construct] Dummy copy construct" << "\n";
	}

	Dummy(Dummy&& other) noexcept : i(other.i), j(other.j) {
		std::cout << "[Construct] Dummy move construct" << "\n";
	}

	~Dummy() {
		i = {};
		j = {};
		std::cout << "[de-Construct] Dummy de-construct" << "\n";
	}

	bool operator==(const Dummy& other) {
		return i == other.i && j == other.j;
	}

	bool operator!=(const Dummy& other) {
		return i != other.i || j != other.j;
	}

	void speak() { 
		std::cout << "Dummy speak i: " << i << " j: " << j << "\n";
	}
};

int main() {
	auto t = MakeHandyThreadPtr();
	auto dummy = ustl::make_optional<Dummy>(1, 2);
	t->Run(Run_Once_Task, &Dummy::speak, &dummy.value());
}


