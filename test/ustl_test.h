#pragma once

#include "ustl.h"
#include "test_auxiliary.h"
#include <gtest/gtest.h>

class ustl_test : public testing::Test {
protected:
	// You can remove any or all of the following functions if their bodies would
	// be empty.

	ustl_test() {
		// You can do set-up work for each test here.
	}

	~ustl_test() override {
		// You can do clean-up work that doesn't throw exceptions here.
	}

	// If the constructor and destructor are not enough for setting up
	// and cleaning up each test, you can define the following methods:

	void SetUp() override {
		// Code here will be called immediately after the constructor (right
		// before each test).
	}

	void TearDown() override {
		// Code here will be called immediately after each test (right
		// before the destructor).
	}
};

TEST_F(ustl_test, ustl_vector_emplace_back) {
	ustl::Vector<Dummy> vec;
	vec.reserve(10);

	Dummy dummy(1, 2);
	vec.emplace_back(dummy);
	vec.emplace_back(Dummy(1, 2));

	std::cout << "vec: " << vec.back().j << "\n";
	std::cout << "vec size: " << vec.size() << "\n";
	std::cout << "vec capacity: " << vec.capacity() << "\n";
}

TEST_F(ustl_test, ustl_shared_ptr_test) {
	ustl::shared_ptr<Dummy> ptr(new Dummy(1, 2));
	std::cout << "ptr: " << ptr->j << "\n";
	ustl::shared_ptr<Dummy> ptr3(new Dummy(3, 4));
	ptr = ptr3;
	ustl::shared_ptr<Dummy> ptr5(std::move(ptr));
	
	ustl::shared_ptr<int> ptr2(new int(10));
	std::cout << "ptr2: " << *ptr2 << "\n";
	
	auto ptr4 = ustl::make_shared<Dummy>(5, 6);
	std::cout << "ptr4: " << ptr4->i << " " << ptr4->j << "\n";
}

TEST_F(ustl_test, ustl_optional_none_default_construct_struct) {
	ustl::optional<Foo> foo;
	std::cout << "foo has_value: " << foo.has_value() << "\n";
	foo.emplace(1);
	std::cout << "foo emplace: " << foo.value().val << "\n";
	foo.reset();
	std::cout << "foo reset has_value: " << foo.has_value() << "\n";
	foo = ustl::optional<Foo>(Foo(2));
	std::cout << "foo operator=: " << foo.value().val << "\n";
	foo = ustl::nullopt;
	std::cout << "foo nullopt has_value: " << foo.has_value() << "\n";
	foo = ustl::make_optional<Foo>(3);
	std::cout << "foo make_optional: " << foo.value().val << "\n";
}

TEST_F(ustl_test, ustl_optional_with_default_construct_struct) {
	ustl::optional<Dummy> dummy;
	std::cout << "dummy has_value: " << dummy.has_value() << "\n";
	dummy.emplace(1, 2);
	std::cout << "dummy emplace: " << dummy.value().i << "\n";
	dummy.reset();
	std::cout << "dummy reset has_value: " << dummy.has_value() << "\n";
	dummy = ustl::optional<Dummy>(Dummy(3, 4));
	std::cout << "dummy operator=: " << dummy.value().i << "\n";
	dummy = ustl::nullopt;
	std::cout << "dummy nullopt has_value: " << dummy.has_value() << "\n";
	dummy = ustl::make_optional<Dummy>(5, 6);
	std::cout << "dummy make_optional: " << dummy.value().i << "\n";
}

TEST_F(ustl_test, ustl_optional_func_test) {
	using optional_func = ustl::optional<std::function<void()>>;

	auto test_util_func = [](const optional_func& f) {
		std::invoke(f.value_or([] { std::cout << "default call" << std::endl; }));
	};

	test_util_func(ustl::nullopt);
	test_util_func([] {});
	test_util_func(optional_func{});
	test_util_func({});
}

TEST_F(ustl_test, ustl_optional_copy_construct_test) {
	using optional_func = ustl::optional<std::function<void()>>;

	auto get_func = []() {
		return ustl::make_optional<std::function<void()>>([] {
			std::cout << "optional(optional<Ty2>&& other) test" << std::endl;
		});
	};

	auto func = get_func();
	std::invoke(func.value());
}

TEST_F(ustl_test, ustl_optional_operator_override_test) {
	auto optional_int = ustl::make_optional<int>(1);
	std::cout << "*optional_int: " << *optional_int << "\n";
}