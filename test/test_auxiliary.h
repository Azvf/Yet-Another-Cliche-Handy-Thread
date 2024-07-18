#pragma once

#include <iostream>

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
		std::cout << "Dummy speak, i: " << i << " j: " << j << "\n";
	}

	void speak2() {
		std::cout << "Dummy speak2, i: " << i << " j: " << j << "\n";
	}

	void speak3(int x, int y) {
		std::cout << "Dummy speak3, x: " << i << " y: " << j << "\n";
	}
};

struct Handler {
	bool Handle(int i) {
		std::cout << "Handle: " << i << "\n";
		return 1;
	}
};

struct Foo {
	int val;
	Foo(int val) : val(val) {}
};