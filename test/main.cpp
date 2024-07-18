#include "yacht.h"
#include "ustl.h"

#include "yacht_test.h"
#include "ustl_test.h"

int main(int argc, char** argv) {
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS(); 
}