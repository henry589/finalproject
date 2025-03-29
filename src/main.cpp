

// this is multithread brute force mcts search system
#if 0

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
// Example function that each thread will run
void mctsWorker(mcts* engine, vnode* root, int numSimulations) {
	for (int i = 0; i < numSimulations; i++) {
		// Standard MCTS steps
		vnode* leaf = engine->selection(root);
		vnode* expanded = engine->expansion(leaf, mcts::EXPANSION_FULL);
		Won   result = engine->simulation(expanded);
		engine->backup(expanded, result);
	}
}

int main() {
	// 1) Initialize bitboard environment, etc.
	init_Bitboards();

	// 2) Create root node
	vnode* root = new vnode();
	// Initialize root->boardB / root->boardW as needed
	// root->boardB = ...
	// root->boardW = ...
	// root->turn   = WHITE;  // or BLACK
	root->boardW = 0x20083c1a102800;
	root->boardB = 0x106240806e0400;
	//boardViewer(root_node->boardB, root_node->boardW);
	//Won winner_iss = mc->simulation(root_node);

	//system("pause");
	root->turn = WHITE; //default as white first
	// 3) Create an MCTS engine
	mcts engine;

	// 4) Decide how many total simulations want and how many threads
	const int totalSimulations = 1000000;
	const int numThreads = 16;

	// 5) Distribute simulations among threads. 
	//    E.g. each thread does totalSimulations / numThreads:
	const int simsPerThread = totalSimulations / numThreads;
	 
	// 6) Launch threads
	std::vector<std::thread> threadPool;
	threadPool.reserve(numThreads);

	for (int t = 0; t < numThreads; t++) {
		// Capture 'engine' and 'root' by pointer to share them
		threadPool.emplace_back(mctsWorker, &engine, root, simsPerThread);
	}

	// 7) Wait for all threads to finish
	for (auto& thr : threadPool) {
		thr.join();
	}

	// 8) After all simulations are done, retrieve the best move
	vnode* bestMove = engine.get_best_move(root);

	// 9) Print some info about the best move, or visualize
	if (bestMove) {
		std::cout << "\nBest move's visits: " << bestMove->sim_visits
			<< ", reward: " << bestMove->sim_reward
			<< ", action: " << (int)bestMove->action_taken << "\n";
		engine.boardViewer(bestMove->boardB, bestMove->boardW);
	}

	return 0;
}
#endif
 
#if 1
#include <torch/torch.h>
#include <vector>
#include <iostream>
#include <algorithm>
#include <random>
#include <fstream>

// Include network, MCTS, and board/node utilities:
#include "../include/othelloNet.h"  // Contains OthelloNetImpl and encode_board
#include "../include/mcts.h"        // Contains mcts class and related methods
#include "../include/bitBoard.h"     // For Bitboard types and popcount
#include "../include/nodeManager.h"  // For vnode and tree management
#include "../include/misc.h"
#include "../include/fsmHeuristic.h"

// Structure for one training sample.
struct TrainingSample {
	torch::Tensor board;   // Tensor of shape [2, 8, 8] (board state)
	torch::Tensor policy;  // Tensor of shape [64] (target probability distribution from MCTS)
	double outcome;        // Game outcome (+1 win for Black, -1 win for White, 0 draw)
};

// Helper: initialize the Othello board to the standard starting configuration.
void initOthelloBoard(uint64_t& boardB, uint64_t& boardW) {
	// Standard Othello initial state: four central pieces.
	// (This assumes a bitboard numbering from SQ_A1=0 to SQ_H8=63.)
	boardB = 0;
	boardW = 0;
	// For example, let Black have pieces at D5 (35) and E4 (28)
	// and White at D4 (27) and E5 (36). Adjust indices if indexing differs.
	boardB |= (1ULL << 28) | (1ULL << 35);
	boardW |= (1ULL << 27) | (1ULL << 36);
}

std::pair<Bitboard, Bitboard> FSM_makeMove(const Bitboard& boardB, const Bitboard& boardW, const Side turn) {

	// Suppose it's black's turn:
	Bitboard cur_side = turn == BLACK ? boardW : boardB;
	Bitboard alt_side = turn == WHITE ? boardW : boardB;
	Bitboard mod_cur_boardW;
	Bitboard mod_cur_boardB;
	int move = FSM::pickMove_KaneFSM(boardB, boardW, turn);
	if (move < 0) {
		std::cout << "FSM: No move => pass.\n";
		return { -1, -1 };
	}
	else {
		int r = FSM::row_of(move);
		int c = FSM::col_of(move);
		//std::cout << "FSM: final chosen move = " << move << " (row=" << r << ", col=" << c << ")\n";
		const Bitboard future_flips = actual_flips(Square(move), cur_side, alt_side);
		mod_cur_boardW = turn == BLACK ? future_flips | boardW | (1ULL << move) : ~future_flips & boardW;
		mod_cur_boardB = turn == WHITE ? future_flips | boardB | (1ULL << move) : ~future_flips & boardB;
		//boardViewer(mod_cur_boardB, mod_cur_boardW);
	}
	return { mod_cur_boardB, mod_cur_boardW };
}

void FSMtestPlay(int& countBlackWin, int& countWhiteWin)
{
	init_Bitboards();
	// Device setup.

	const int simulationsPerMove = 100;
	const double c_puct = 1.5;

	// Start from the standard board

	uint64_t cur_boardB = 0x0000000810000000;
	uint64_t cur_boardW = 0x0000001008000000;
	Side modeMadeTurn = WHITE;

	mcts engine;


	int passCount = 0;

	while (true) {
		//std::cout << "\nTurn: " << (modeMadeTurn == BLACK ? "BLACK" : "WHITE") << std::endl;
		//boardViewer(cur_boardB, cur_boardW);

		// End game if board is full or both players pass
		if (popcount(cur_boardB | cur_boardW) == 64 || passCount >= 2) {
			break;
		}

		// Check if current player has any valid moves
		std::vector<Square> valids = engine.getValidMoves(cur_boardB, cur_boardW, modeMadeTurn);
		if (valids.empty()) {
			//std::cout << "[INFO] No valid moves. Passing turn.\n";

			modeMadeTurn = Side(!modeMadeTurn);


			passCount++;
			continue;
		}

		passCount = 0;

		if ((modeMadeTurn == BLACK)) { //
			// ----- Human Turn -----
			std::vector<Square> valids = engine.getValidMoves(cur_boardB, cur_boardW, modeMadeTurn);

			//std::cout << "Valid moves: ";
			//for (int mv : valids) std::cout << mv << " ";
			//std::cout << "\nEnter move （0-63）: ";

			int move_int;
			//if (!(std::cin >> move_int)) {
			//	std::cin.clear();
			//	std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			//	std::cout << "Invalid input. Try again.\n";
			//	continue;
			//}
			int chosenRandom = getRandomNumber(0, valids.size() - 1); //simulate random player
			Square move = valids[chosenRandom];
			//std::cout << "\nmove taken:" << move;
			if (std::find(valids.begin(), valids.end(), move) == valids.end()) {
				//std::cout << "Invalid move. Try again.\n";
				continue;
			}
			Bitboard  cur_side = modeMadeTurn == BLACK ? cur_boardW : cur_boardB;
			Bitboard  alt_side = modeMadeTurn == WHITE ? cur_boardW : cur_boardB;
			const Bitboard future_flips = actual_flips(move, cur_side, alt_side);
			cur_boardW = modeMadeTurn == BLACK ? future_flips | cur_boardW | (1ULL << move) : ~future_flips & cur_boardW;
			cur_boardB = modeMadeTurn == WHITE ? future_flips | cur_boardB | (1ULL << move) : ~future_flips & cur_boardB;
			modeMadeTurn = Side(!modeMadeTurn);

		}
		else {
			// ----- FSM Turn -----

			//std::cout << "[FSM is choosing move...]\n";

			auto results = FSM_makeMove(cur_boardB, cur_boardW, modeMadeTurn);
			if (std::get<0>(results) == -1) {
				//std::cout << "[ERROR] FSM failed to select move. Passing.\n";
				modeMadeTurn = Side(!modeMadeTurn);


				continue;
			}

			cur_boardB = std::get<0>(results);
			cur_boardW = std::get<1>(results);
			modeMadeTurn = Side(!modeMadeTurn);

		}
	}

	// ----- Game Over -----
	//std::cout << "\nFinal board:\n";
	//boardViewer(cur_boardB, cur_boardW);

	Won winner = engine.check_winner(cur_boardB, cur_boardW);
	if (winner == Won::BLACK_PLAYER)
	{
		countBlackWin += 1;
		//std::cout << "\nWinner: BLACK\n";
	}
	else if (winner == Won::WHITE_PLAYER)
	{
		countWhiteWin += 1;
		//std::cout << "\nWinner: WHITE\n";
	}
	else std::cout << "\nDraw!\n";
}


void testFSM() {
	int countBlackWin = 0;
	int countWhiteWin = 0;
	
	for (int m = 0; m < 1000; ++m)
	{
		FSMtestPlay(countBlackWin, countWhiteWin);
	}
	std::cout << "\n total black win:" << countBlackWin;
	std::cout << "\n total white win:" << countWhiteWin;
	return;
}


MemoryPool vnode::pool = MemoryPool(sizeof(vnode), 100000);
std::stringstream vnode::ss;
// Function to run one self-play episode using MCTS.
// It uses network to guide the search and collects training samples.
std::vector<TrainingSample> runSelfPlayEpisode(OthelloNet net, mcts engine, double c_puct, const int numSimulations) {

	std::vector<TrainingSample> episodeData;

	// Create the root node for the game.
	vnode* root = new vnode();
	uint64_t boardB, boardW;
	initOthelloBoard(boardB, boardW);
	root->boardB = boardB;
	root->boardW = boardW;
	// Convention: set root->turn = WHITE so that the first move (child) gets turn = BLACK.
	root->turn = WHITE;

	bool gameOver = false;
	while (!gameOver) {
		// Run a fixed number of MCTS simulations for the current move.
		for (int sim = 0; sim < numSimulations; sim++) {
			vnode* leaf = engine.selection_ai(root, c_puct);
			vnode* expanded = engine.expansion_ai(leaf, mcts::EXPANSION_FULL);
			Won outcome = engine.simulation_ai(expanded);
			engine.backup(expanded, outcome);
			engine.update_prior(expanded);
		}

		// At the current root, compute the MCTS visit counts for each move (0-63).
		std::vector<double> visits(64, 0.0);
		double totalVisits = 0.0;
		// Iterate over the children of the root.
		vnode* child = root->get_children();
		while (child) {
			// Use the move index stored in action_taken as the move.
			visits[child->action_taken] = static_cast<double>(child->sim_visits);
			totalVisits += visits[child->action_taken];
			child = child->get_next_sibling();
		}
		// Build the target policy: if no moves visited, use a zero vector.
		torch::Tensor policyTarget = torch::zeros({ 64 });
		if (totalVisits > 0) {
			policyTarget = torch::tensor(visits) / totalVisits;
		}
		// Record this state as a training sample.
		// Encode the board state to a tensor of shape [1, 2, 8, 8] then squeeze to [2,8,8].
		torch::Tensor boardTensor = OthelloNetImpl::encode_board(root->boardB, root->boardW).squeeze(0);
		episodeData.push_back({ boardTensor, policyTarget, 0.0 });

		// Select the move with the highest visit count.
		int bestMove = -1;
		double bestVisit = -1;
		for (int i = 0; i < 64; i++) {
			if (visits[i] > bestVisit) {
				bestVisit = visits[i];
				bestMove = i;
			}
		}
		if (bestMove == -1) {
			// No valid move found => game over.
			gameOver = true;
			break;
		}

		// Move to the child corresponding to bestMove.
		vnode* nextNode = nullptr;
		child = root->get_children();
		while (child) {
			if (child->action_taken == bestMove) {
				nextNode = child;
				break;
			}
			child = child->get_next_sibling();
		}
		if (!nextNode) {
			gameOver = true;
			break;
		}
		// Set the new root to be this node.
		root = nextNode;

		// Termination check: if board is nearly full, or no valid moves for both sides.
		if (popcount(root->boardB | root->boardW) >= 60) {
			gameOver = true;
		}
	}

	// Determine the final outcomes using check_winner function.
	Won finalOutcome = engine.check_winner(root->boardB, root->boardW);
	double outcomeVal = 0.0;
	if (finalOutcome == Won::BLACK_PLAYER)
		outcomeVal = 1.0;
	else if (finalOutcome == Won::WHITE_PLAYER)
		outcomeVal = -1.0;
	else
		outcomeVal = 0.0;
	// Update all training samples with the final outcome.
	for (auto& sample : episodeData) {
		sample.outcome = outcomeVal;
	}

	// Clean up the MCTS tree:
	vnode::BFS(root, vnode::OpType::PRUNE, true);

	return episodeData;
}

OthelloNet model = OthelloNet(128, 10);

// Function that runs one generation of self-play + training
void main_training_loop(OthelloNet& model, mcts& engine, torch::Device device,
	int totalGenerations, int numEpisodesPerGen, int simulationsPerMove, int epochsPerGen)
{
	double c_puct = 1.5;

	// Open loss log file
	std::ofstream lossLog("training_loss_log.csv");
	lossLog << "Generation,Epoch,AvgTotalLoss,AvgPolicyLoss,AvgValueLoss\n";

	for (int gen = 0; gen < totalGenerations; gen++) {
		std::cout << "\n=== Generation " << gen << " ===" << std::endl;

		// ===== Step 1: Self-play data generation =====
		std::vector<TrainingSample> trainingData;
		for (int ep = 0; ep < numEpisodesPerGen; ep++) {
			std::vector<TrainingSample> episodeData = runSelfPlayEpisode(model, engine, c_puct, simulationsPerMove);
			trainingData.insert(trainingData.end(), episodeData.begin(), episodeData.end());
			std::cout << "Episode " << ep << " produced " << episodeData.size() << " samples." << std::endl;
		}
		std::cout << "Collected a total of " << trainingData.size() << " samples.\n" << std::endl;

		// ===== Step 2: Training =====
		model->train();
		torch::optim::Adam optimizer(model->parameters(), torch::optim::AdamOptions(1e-3));
		const int batchSize = 64;

		for (int epoch = 0; epoch < epochsPerGen; epoch++) {
			std::shuffle(trainingData.begin(), trainingData.end(), std::default_random_engine());

			float epochTotalLoss = 0.0f;
			float epochPolicyLoss = 0.0f;
			float epochValueLoss = 0.0f;
			int batchCount = 0;

			for (size_t i = 0; i < trainingData.size(); i += batchSize) {
				size_t end = std::min(trainingData.size(), i + batchSize);
				std::vector<torch::Tensor> boards, policies, outcomes;
				for (size_t j = i; j < end; j++) {
					boards.push_back(trainingData[j].board.unsqueeze(0));   // [1,2,8,8]
					policies.push_back(trainingData[j].policy.unsqueeze(0)); // [1,64]
					outcomes.push_back(torch::tensor({ trainingData[j].outcome }));
				}

				auto batchBoards = torch::cat(boards, 0).to(device);
				auto batchPolicies = torch::cat(policies, 0).to(device);
				auto batchOutcomes = torch::cat(outcomes, 0).to(device);

				optimizer.zero_grad();
				auto outputs = model->forward(batchBoards);
				auto policy_logits = std::get<0>(outputs);  // [B,64]
				auto value_pred = std::get<1>(outputs).squeeze(1); // [B]

				auto log_probs = torch::log_softmax(policy_logits, 1);
				auto policy_loss = -(batchPolicies * log_probs).sum(1).mean();
				auto value_loss = torch::mse_loss(value_pred, batchOutcomes);
				auto total_loss = policy_loss + value_loss;

				total_loss.backward();
				optimizer.step();

				epochTotalLoss += total_loss.item<float>();
				epochPolicyLoss += policy_loss.item<float>();
				epochValueLoss += value_loss.item<float>();
				batchCount++;

				// Batch-level print
				std::cout << "Gen " << gen << " Epoch " << epoch
					<< " Batch " << (i / batchSize)
					<< " Loss: " << total_loss.item<float>()
					<< " (Policy: " << policy_loss.item<float>()
					<< ", Value: " << value_loss.item<float>() << ")" << std::endl;
			}

			// Epoch-level summary
			float avgTotal = epochTotalLoss / batchCount;
			float avgPolicy = epochPolicyLoss / batchCount;
			float avgValue = epochValueLoss / batchCount;

			std::cout << "=== Generation " << gen << " Epoch " << epoch
				<< " Averages: TotalLoss=" << avgTotal
				<< " PolicyLoss=" << avgPolicy
				<< " ValueLoss=" << avgValue << " ===" << std::endl;

			// Log to file
			lossLog << gen << "," << epoch << "," << avgTotal << "," << avgPolicy << "," << avgValue << "\n";
		}

		// ===== Step 3: Save checkpoint =====
		std::string checkpointName = "othello_model_gen_" + std::to_string(gen) + ".pt";
		torch::save(model, checkpointName);
		std::cout << "Checkpoint saved: " << checkpointName << std::endl;
	}

	lossLog.close();
	std::cout << "\n=== Training Completed! ===" << std::endl;
}

void testAI(const int & playcount, int & blackwins, int & whitewins)
{
	init_Bitboards();
	// Device setup.
	torch::Device device(torch::kCUDA);
	if (!torch::cuda::is_available())
		device = torch::Device(torch::kCPU);

	//load the model here
	try {
		torch::load(model, "othello_model_gen_35.pt");
		model->eval();
	}
	catch (const c10::Error& e) {
		std::cerr << "Error loading model: " << e.msg() << std::endl;
		return;
	}
	// Create network instance.
	model->to(device);

	const int simulationsPerMove = 100;
	const double c_puct = 1.5;

	// Start from the standard board
	for (int m = 0; m < playcount; ++m)
	{
		std::cout << "\ncurrent play:" << m<<",prev wins white:"<< whitewins<<",prev wins black:"<<blackwins;
		uint64_t cur_boardB = 0x0000000810000000;
		uint64_t cur_boardW = 0x0000001008000000;
		Side modeMadeTurn = WHITE;

		mcts engine;


		int passCount = 0;

		while (true) {
			//std::cout << "\nTurn: " << (modeMadeTurn == BLACK ? "BLACK" : "WHITE") << std::endl;
			//boardViewer(cur_boardB, cur_boardW);

			// End game if board is full or both players pass
			if (popcount(cur_boardB | cur_boardW) == 64 || passCount >= 2) {
				break;
			}

			// Check if current player has any valid moves
			std::vector<Square> valids = engine.getValidMoves(cur_boardB, cur_boardW, modeMadeTurn);
			if (valids.empty()) {
				//std::cout << "[INFO] No valid moves. Passing turn.\n";

				modeMadeTurn = Side(!modeMadeTurn);


				passCount++;
				continue;
			}

			passCount = 0;

			if ((modeMadeTurn == BLACK)) { //
				// ----- Human Turn -----
				std::vector<Square> valids = engine.getValidMoves(cur_boardB, cur_boardW, modeMadeTurn);

				//std::cout << "Valid moves: ";
				//for (int mv : valids) std::cout << mv << " ";
				//std::cout << "\nEnter move （0-63）: ";

				int move_int;
				//if (!(std::cin >> move_int)) {
				//	std::cin.clear();
				//	std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
				//	std::cout << "Invalid input. Try again.\n";
				//	continue;
				//}
				int chosenRandom = getRandomNumber(0, valids.size() - 1); //simulate random player
				Square move = valids[chosenRandom];
				//std::cout << "\nmove taken:" << move;
				if (std::find(valids.begin(), valids.end(), move) == valids.end()) {
					//std::cout << "Invalid move. Try again.\n";
					continue;
				}
				Bitboard  cur_side = modeMadeTurn == BLACK ? cur_boardW : cur_boardB;
				Bitboard  alt_side = modeMadeTurn == WHITE ? cur_boardW : cur_boardB;
				const Bitboard future_flips = actual_flips(move, cur_side, alt_side);
				cur_boardW = modeMadeTurn == BLACK ? future_flips | cur_boardW | (1ULL << move) : ~future_flips & cur_boardW;
				cur_boardB = modeMadeTurn == WHITE ? future_flips | cur_boardB | (1ULL << move) : ~future_flips & cur_boardB;
				modeMadeTurn = Side(!modeMadeTurn);

			}
			else {
				// ----- AI Turn -----
				vnode* root = new vnode();
				root->boardB = cur_boardB;
				root->boardW = cur_boardW;
				root->turn = WHITE; //human is white, this is the move made side
				//std::cout << "[AI is thinking...]\n";
				// get the first prior here
				for (int sim = 0; sim < simulationsPerMove; ++sim) {
					vnode* leaf = engine.selection_ai(root, c_puct);
					vnode* expanded = engine.expansion_ai(leaf, mcts::EXPANSION_FULL);
					Won outcome = engine.simulation_ai(expanded);
					engine.backup(expanded, outcome);
					engine.update_prior(expanded);
				}

				vnode* best = engine.get_best_move(root);
				if (!best) {
					//std::cout << "[ERROR] AI failed to select move. Passing.\n";
					modeMadeTurn = Side(!modeMadeTurn);					  
					continue;
				}

				//std::cout << "\n[AI played move : " << best->action_taken << "]\n";
				cur_boardB = best->boardB;
				cur_boardW = best->boardW;
				vnode::BFS(root, vnode::OpType::PRUNE, true);
				modeMadeTurn = Side(!modeMadeTurn);

			}
		}

		// ----- Game Over -----
		//std::cout << "\nFinal board:\n";
		//boardViewer(cur_boardB, cur_boardW);

		Won winner = engine.check_winner(cur_boardB, cur_boardW);
		if (winner == Won::BLACK_PLAYER) {
			blackwins++;
			//std::cout << "\nWinner: BLACK\n";
		}
		else if (winner == Won::WHITE_PLAYER) {
			whitewins++;
			//std::cout << "\nWinner: WHITE\n";
		}
		else std::cout << "\nDraw!\n";
	}
}

void trainSetup() {
	init_Bitboards();
	// Device setup.
	torch::Device device(torch::kCUDA);
	if (!torch::cuda::is_available())
	{
		device = torch::Device(torch::kCPU);
	}
	else {
		std::cout << "\nusing CUDA";
	
	}

	try {
		torch::load(model, "C:/Users/User/source/repos/finalproject/x64-Release_20deepres/othello_model_gen_1.pt");
	}
	catch (const c10::Error& e) {
		std::cerr << "Error loading model: " << e.msg() << std::endl;
		return;
	}
	// Create network instance.
	model->train();
	model->to(device);

	mcts engine;

	// Train for 50 generations, each generation:
	// - run 20 self-play episodes, with 100 simulations per move
	// - train for 5 epochs per generation
	main_training_loop(model, engine, device,
		50,    // totalGenerations
		20,    // numEpisodesPerGen
		400,   // simulationsPerMove
		5);    // epochsPerGen
}
int main() {
	int whitewins;
	int blackwins;

	testAI(1000, blackwins, whitewins);
	std::cout << "\nblackwins:" << blackwins;
	std::cout << "\nwhitewins:" << whitewins;
	//trainSetup();
	return 0;
}

#endif
