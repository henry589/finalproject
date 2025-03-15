#include <algorithm>  // for std::max
#include <cstdlib>    // for std::malloc, std::free
#include <mutex>      // for std::mutex, std::lock_guard
#include <vector>     // for std::vector
#include <stdexcept>  // for std::bad_alloc
#include <cassert>
#include <iostream>
#include <thread>
#include <cstring> // For memset and memcpy
#include <sstream>
#include "../include/memoryPool.h"
#include "../include/nodeManager.h"
#include "../include/mcts.h"
#include "../include/bitBoard.h"
#include "../include/misc.h"

using namespace bitboard;

MemoryPool vnode::pool = MemoryPool(sizeof(vnode), 100000);
std::stringstream vnode::ss;

void test()
{
	vnode* list = NULL;

	Side s = CURR;
	vnode* parent_node = new vnode();
	parent_node->boardB = 0;
	std::cout << "start:" << std::endl;
	// this is a routine to simulate addition of sibling to the children head
	for (int i = 1; i < 6; ++i)
	{
		parent_node->append_child(i, i, bool(s), 0);
	}
	std::cout << "done:" << std::endl;

	// to simulate the rollout of the children
	vnode* asd = parent_node->get_children();
	int dummycounter = 0;
	while (asd != nullptr)
	{
		std::cout << asd->boardB << std::endl;
		asd->sim_visits = 2;
		asd->sim_reward = 11;

		dummycounter++;
		asd = asd->get_next_sibling();
	}

	vnode* asd2 = parent_node->get_children();
	std::cout << "second:" << std::endl;
	int n = 0;
	while (asd2 != nullptr)
	{
		std::cout << asd2->boardB << std::endl;
		asd2 = asd2->get_next_sibling();

		n++;
	}
	// vnode::BFS(parent_node);
	std::cout << "third:" << std::endl;
	asd2 = parent_node->get_children();
	parent_node->sim_visits = 2;
	vnode* selected_node = nullptr;
	vnode* nextGen = nullptr;
	vnode* nextnextGen = nullptr;
	vnode* record = nullptr;
	n = 0;
	while (asd2 != nullptr)
	{
		if (n == 3)
		{
			selected_node = asd2;
			selected_node->append_child(6, 0x76, s, 0);
			selected_node->append_child(7, 0x76, s, 0);
			selected_node->append_child(8, 0x76, s, 0);
			selected_node->sim_visits = 200;
			selected_node->sim_reward = 18;

			nextGen = selected_node->get_children();
			nextGen->sim_visits = 5;
			nextGen->sim_reward = 20;
			nextGen->get_next_sibling()->sim_visits = 3;
			nextGen->get_next_sibling()->sim_reward = 3;
			nextGen->get_next_sibling()->get_next_sibling()->sim_visits = 4;
			nextGen->get_next_sibling()->get_next_sibling()->sim_reward = 6;
			nextGen->append_child(9, 0x92, s, 0);
			nextGen->append_child(10, 0x93, s, 0);

			nextnextGen = nextGen->get_children();
			// record = nextnextGen->get_next_sibling();
			nextnextGen->get_next_sibling()->append_child(11, 0x00, s, 0);
			nextnextGen->append_child(12, 0x00, s, 0);
			nextnextGen->append_child(13, 0x00, s, 0);
			nextnextGen->sim_visits = 2;
			nextnextGen->sim_reward = 20;
			nextnextGen->get_next_sibling()->sim_visits = 1;
			nextnextGen->get_next_sibling()->sim_reward = 30;
		}
		if (n == 1)
		{
			record = asd2;
		}
		std::cout << asd2->boardB << ",p:" << asd2->get_parent()->boardB << std::endl;
		asd2 = asd2->get_next_sibling();

		n++;
	}

	//generate dot format
	vnode::BFS(parent_node, vnode::OpType::TRAVERSE, false);
	std::string x = vnode::get_dot_formatted();
	std::cout << "\nsearch:" << x;
	std::cout << "size of vnode:" << sizeof(vnode);

	mcts* mc = new mcts();
	mc->selection(selected_node);

	bool win_side = CURR;
	mc->backup(nextnextGen, win_side, true);

	std::cout << "score mod:" << nextGen->sim_reward;
	int childCount = 0;
	vnode* test_node = new vnode();
	// test_node->boardB = 0x4a000a200080060;
	// test_node->boardW = 0x8101c30002000;
	//test_node->boardW = 0x81402400d0492a44;
	//test_node->boardB = 0x4200000821940028;
	test_node->boardW = 0x1008000000;
	test_node->boardB = 0x810000000;

	//test_node->boardW = 0xf3f3f3f3f3f3f3f3;
	//test_node->boardB = 0xc0c0c0c0c0c0c0c;
	/*test_node->turn = BLACK;
	vnode* children = mc->createValidChildren(test_node, childCount);
	std::cout << "\nchild count:" << childCount;
	vnode* tmpChildren = children;
	while (tmpChildren != nullptr)
	{
		std::cout << "\nmove:" << tmpChildren->action_taken;
		mc->boardViewer(tmpChildren->boardB, tmpChildren->boardW);
		tmpChildren = tmpChildren->get_next_sibling();
	}*/

	vnode* result = mc->expansion(test_node, mcts::exp_mode::EXPANSION_SINGLE);
	vnode::BFS(test_node, vnode::OpType::TRAVERSE, false);
	std::string x2 = vnode::get_dot_formatted();
	std::cout << "\nsearch:" << x2;
	// uint64_t boardB = 0x89240a904394248a;
	// uint64_t boardW = 0x2218012d20008014;
	if(result != nullptr)
		mc->boardViewer(result->boardB, result->boardW);

	for (int n = 0; n <= 1;++n)
	{	
		mc->simulation(test_node);
	}
	for (int m = 0; m < 5; m++)
	{
		std::cout << "\nrandom number:" << getRandomNumber(0, 5);
		//std::cout << ",random number no bound:" << fast_rand();
	}

	//for (int m = 0; m <= 100; ++m)
	//{
	//	std::cout <<std::endl<< rand()%2;
	//}
}

void singleThreadTest() {
	std::cout << "Running single-threaded test...\n";

	// Create a memory pool with block size 32 bytes and chunk size 10 blocks
	MemoryPool pool(32, 10);

	// Allocate some blocks
	void* block1 = pool.allocate();
	void* block2 = pool.allocate();
	void* block3 = pool.allocate();

	// Deallocate a block and allocate again (to check reuse)
	pool.deallocate(block2);
	void* block4 = pool.allocate();

	// Ensure the same block was reused
	assert(block2 == block4);

	std::cout << "Single-threaded test passed.\n";
}

//void multiThreadTest() {
//    std::cout << "Running multi-threaded test...\n";
//
//    MemoryPool pool(64, 20); // Create a memory pool with block size 64 bytes and chunk size 20 blocks
//
//    const int numThreads = 4;
//    const int numAllocationsPerThread = 100;
//
//    auto allocateAndDeallocate = [&pool]() {
//        std::vector<void*> blocks;
//
//        // Allocate multiple blocks
//        for (int i = 0; i < numAllocationsPerThread; ++i) {
//            blocks.push_back(pool.allocate());
//        }
//
//        // Deallocate them
//        for (void* block : blocks) {
//            pool.deallocate(block);
//        }
//    };
//
//    // Create and run multiple threads
//    std::vector<std::thread> threads;
//    for (int i = 0; i < numThreads; ++i) {
//        threads.emplace_back(allocateAndDeallocate);
//    }
//
//    for (auto& t : threads) {
//        t.join(); // Wait for all threads to finish
//    }
//
//    std::cout << "Multi-threaded test passed.\n";
//}

//void stressTest() {
//    std::cout << "Running stress test...\n";
//
//    MemoryPool pool(128, 50); // Create a memory pool with block size 128 bytes and chunk size 50 blocks
//
//    const int numThreads = 8;
//    const int numAllocationsPerThread = 1000;
//
//    auto allocateAndDeallocate = [&pool]() {
//        for (int i = 0; i < numAllocationsPerThread; ++i) {
//            void* block = pool.allocate();
//            pool.deallocate(block); // Allocate and immediately deallocate
//        }
//    };
//
//    // Create and run multiple threads
//    std::vector<std::thread> threads;
//    for (int i = 0; i < numThreads; ++i) {
//        threads.emplace_back(allocateAndDeallocate);
//    }
//
//    for (auto& t : threads) {
//        t.join(); // Wait for all threads to finish
//    }
//
//    std::cout << "Stress test passed.\n";
//}

void valueVerificationTest() {
	std::cout << "Running value verification test...\n";

	const size_t blockSize = 32;
	const size_t chunkSize = 10;

	MemoryPool pool(blockSize, chunkSize);

	// Allocate blocks and write unique data to each
	void* blocks[chunkSize];
	for (size_t i = 0; i < chunkSize; ++i) {
		blocks[i] = pool.allocate();

		// Write data to the block
		int value = static_cast<int>(i); // Example unique value
		std::memset(blocks[i], value, blockSize); // Fill block with the value
	}

	// Verify that data is preserved
	for (size_t i = 0; i < chunkSize; ++i) {
		int value = static_cast<int>(i); // Expected value
		char expected[blockSize];
		std::memset(expected, value, blockSize); // Create expected pattern

		// Compare memory content
		assert(std::memcmp(blocks[i], expected, blockSize) == 0);
	}

	// Deallocate all blocks
	for (size_t i = 0; i < chunkSize; ++i) {
		pool.deallocate(blocks[i]);
	}

	std::cout << "Value verification test passed.\n";
}

void test_uct_formula()
{
	Side s = CURR;
	vnode* parent_node = new vnode();
	parent_node->sim_visits = 5;
	parent_node->append_child(13, 14, s, 0);
	vnode* testnode = parent_node->get_children();
	testnode->sim_visits = 100;
	testnode->sim_reward = 3;
	// testnode->explorationConstant = 1.414;
	std::cout << "\nuct calculated:" << testnode->calc_uct() << std::endl;
}

int main() {
	// try {
	//     valueVerificationTest();

	//     std::cout << "All tests passed successfully!\n";
	// } catch (const std::exception& e) {
	//     std::cerr << "Test failed: " << e.what() << '\n';
	//     return 1;
	// }
	init_Bitboards();

	test();
	test_uct_formula();
	std::cout << "hello";
	// std::cout<<"\nindexed:"<<magics[SQ_F5][ORTHO - DIAGO].rays_bb(0x2000008400000020);

	// uint64_t boardx = 0;
	// mcts *dummy = new mcts();
	// std::cout << "\n\nconnectivity mask 0:\n";
	// dummy->boardViewer(connectivityMaskOrtho[SQ_F5][0], boardx);
	// std::cout << "\n\nconnectivity mask 1:\n";
	// dummy->boardViewer(connectivityMaskOrtho[SQ_F5][1], boardx);
	// std::cout << "\n\nconnectivity mask 2:\n";
	// dummy->boardViewer(connectivityMaskOrtho[SQ_F5][2], boardx);
	// std::cout << "\n\nconnectivity mask 3:\n";
	// dummy->boardViewer(connectivityMaskOrtho[SQ_F5][3], boardx);
	// // 0x101430002000
	// // 0x8505c30002000

	// // 0x4a0008200080020

	// Bitboard playerBoard = 0x100000000000;
	// Bitboard oppBoard = 0xa0000000000000;
	//     std::cout<<"\nstart 2333333333333333\n";

	// Square testSquare = SQ_G8;
	// const Bitboard & future_flips = actual_flips(testSquare, playerBoard, oppBoard);
	// bool validFlip = future_flips ^ 0 ? true : false;
	// std::cout<<"\nthe answer:\n";

	// dummy->boardViewer(future_flips, boardx);

	// // dummy->boardViewer(connectivityMaskDiago[SQ_C2][0], boardx);
	// // dummy->boardViewer(connectivityMaskDiago[SQ_C2][1], boardx);
	// // dummy->boardViewer(connectivityMaskDiago[SQ_C2][2], boardx);
	// // dummy->boardViewer(connectivityMaskDiago[SQ_C2][3], boardx);
	// std::cout<<"\nvalid is move:\n";
	// std::cout<<validFlip;

//   const Bitboard & future_flips = actual_flips(sq, Black_occupied, White_occupied);
//     bool validFlip = future_flips ^ 0 ? true : false; // means got possible flips

//     Bitboard diagoray = magics[SQ_D4][DIAGO - DIAGO].rays_bb(0x220000100001);
//     dummy->boardViewer(diagoray, boardx);

//     std::cout<<":"<<nb<<"\n";
//     dummy->boardViewer(nb, boardx);
	return 0;
}