#pragma once

#include "yacht.h"
#include "test_auxiliary.h"
#include <gtest/gtest.h>

class YachtTest : public testing::Test {
protected:
	// You can remove any or all of the following functions if their bodies would
	// be empty.

	YachtTest() {
		// You can do set-up work for each test here.
	}

	~YachtTest() override {
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

	// Class members declared here can be used by all tests in the test suite
	// for Foo.
};


TEST_F(YachtTest, Yacht_VoidRet_MemFn_RunOnce_Task_With_MemFn_Callback) {
	auto t = MakeHandyThreadPtr();
	auto dummy = std::make_shared<Dummy>(1, 2);
	auto handler = std::make_shared<Handler>();
	t->SetCallbackCtx([dummy, handler] { handler->Handle(dummy->j); })
		->Run(Run_Once_Task, [dummy] { dummy->speak(); });
	std::this_thread::sleep_for(std::chrono::milliseconds(300));
}

TEST_F(YachtTest, Yacht_IntRet_MemFn_RunOnce_Task_With_MemFn_Callback) {
	auto yacht = MakeHandyThreadPtr();
	auto dummy = std::make_shared<Dummy>(1, 2);
	auto handler = std::make_shared<Handler>();
	yacht->SetCallbackCtx(
		[dummy, handler] {
			handler->Handle(dummy->j);
			return 1;
		})
		->Run(Run_Once_Task,
			[dummy] {
				dummy->speak();
				return 1;
			});
	std::this_thread::sleep_for(std::chrono::milliseconds(300));
}

TEST_F(YachtTest, Yacht_VoidRet_MemFn_RunTimer_Task_With_MemFn_Callback) {
	auto t = MakeHandyThreadPtr();
	auto dummy = std::make_shared<Dummy>(1, 2);
	auto handler = std::make_shared<Handler>();
	t->SetCallbackCtx([dummy, handler] { handler->Handle(dummy->j); })
		->Run(Timer_Task, [dummy] { dummy->speak(); });
	Sleep(300);
}

TEST_F(YachtTest, Yacht_IntRet_MemFn_RunTimer_Task_With_MemFn_Callback) {
	auto yacht = MakeHandyThreadPtr();
	auto dummy = std::make_shared<Dummy>(1, 2);
	auto handler = std::make_shared<Handler>();
	yacht->SetCallbackCtx(
		[dummy, handler] {
			handler->Handle(dummy->j);
			return 1;
		})
		->Run(Timer_Task,
			[dummy] {
				dummy->speak();
				return 1;
			});
	std::this_thread::sleep_for(std::chrono::milliseconds(300));
}

TEST_F(YachtTest, Yacht_Args_MemFn_RunOnce_Task_With_MemFn_Callback) {
	auto yacht = MakeHandyThreadPtr();
	auto dummy = std::make_shared<Dummy>(1, 2);
	auto handler = std::make_shared<Handler>();

	int x = 10, y = 20;
	yacht->SetCallbackCtx(
		[dummy, handler] {
			handler->Handle(dummy->j);
			return 1;
		})
		->Run(Run_Once_Task, &Dummy::speak3, &(*dummy), std::ref(x), std::ref(y));
	std::this_thread::sleep_for(std::chrono::milliseconds(300));
}

TEST_F(YachtTest, Yacht_Args_MemFn_RunTimer_Task_With_MemFn_Callback) {
	auto yacht = MakeHandyThreadPtr();
	auto dummy = std::make_shared<Dummy>(1, 2);
	auto handler = std::make_shared<Handler>();

	int x = 10, y = 20;
	yacht->SetCallbackCtx(
		[dummy, handler] {
			handler->Handle(dummy->j);
			return 1;
		})
		->Run(Timer_Task, &Dummy::speak3, &(*dummy), std::ref(x), std::ref(y));
	std::this_thread::sleep_for(std::chrono::milliseconds(300));
}


TEST_F(YachtTest, test_case) {
	auto t = MakeHandyThreadPtr();
	auto dummy = std::make_shared<Dummy>(1, 2);
	t->Run(Run_Once_Task, [dummy] { dummy->speak(); });
}