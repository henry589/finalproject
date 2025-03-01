#include <iostream>
#include "../include/bitBoard.h"
#include "../include/misc.h"

namespace bitboard {
	alignas(64) Magic magics[SQUARE_NB][2];
	uint8_t SquareDistance[SQUARE_NB][SQUARE_NB];
	Bitboard OrthoTable[0x19000];  // to store orthogonal attacks
	Bitboard DiagoTable[0x1480];  // To store diagonal attacks

	Bitboard connectivityMaskOrtho[SQUARE_NB][4];
	Bitboard connectivityMaskDiago[SQUARE_NB][4];
	Bitboard safe_destination(Square s, int step) {
		Square to = Square(s + step);
		return is_ok(to) && distance(s, to) <= 2 ? square_bb(to) : Bitboard(0);
	}

	void init_Bitboards()
	{
		for (Square s1 = SQ_A1; s1 <= SQ_H8; ++s1)
			for (Square s2 = SQ_A1; s2 <= SQ_H8; ++s2)
				SquareDistance[s1][s2] = std::max(distance<File>(s1, s2), distance<Rank>(s1, s2));

		int      size = 0;
		Bitboard occupancy[4096];
		int      epoch[4096] = {}, cnt = 0;
		Bitboard reference[4096];
		int seeds[][RANK_NB] = { {8977, 44560, 54343, 38998, 5731, 95205, 104912, 17020},
		{728, 10316, 55013, 32803, 12281, 15100, 16645, 255} };
		DirectionType direction_type[] = { ORTHO, DIAGO };
		Bitboard* table;
		for (DirectionType dr : direction_type)
		{
			table = dr == ORTHO ? OrthoTable : DiagoTable;
			for (Square s = SQ_A1; s <= SQ_H8; ++s)
			{
				// Board edges are not considered in the relevant occupancies
				Bitboard edges = ((Rank1BB | Rank8BB) & ~rank_bb(s)) | ((FileABB | FileHBB) & ~file_bb(s));

				// Given a square 's', the mask is the bitboard of sliding attacks from
				// 's' computed on an empty board. The index must be big enough to contain
				// all the attacks for each possible subset of the mask and so is 2 power
				// the number of 1s of the mask. Hence we deduce the size of the shift to
				// apply to the 64 or 32 bits word to get the index.
				Magic& m = magics[s][dr - DIAGO];
				m.mask = expected_flips(dr, s, 0) & ~edges;
				m.shift = 64 - popcount(m.mask);
				// Set the offset for the attacks table of the square. We have individual
				// table sizes for each square with "Fancy Magic Bitboards".
				m.attacks = s == SQ_A1 ? table : magics[s - 1][dr - DIAGO].attacks + size;
				size = 0;

				// Use Carry-Rippler trick to enumerate all subsets of masks[s] and
				// store the corresponding sliding attack bitboard in reference[].
				Bitboard b = 0;
				do
				{
					occupancy[size] = b;
					reference[size] = expected_flips(dr, s, b);

					size++;
					b = (b - m.mask) & m.mask;
				} while (b);

				PRNG rng(seeds[true][rank_of(s)]);

				// Find a magic for square 's' picking up an (almost) random number
				// until we find the one that passes the verification test.
				for (int i = 0; i < size;)
				{
					for (m.magic = 0; popcount((m.magic * m.mask) >> 56) < 6;)
						m.magic = rng.sparse_rand<Bitboard>();

					// A good magic must map every possible occupancy to an index that
					// looks up the correct sliding attack in the attacks[s] database.
					// Note that we build up the database for square 's' as a side
					// effect of verifying the magic. Keep track of the attempt count
					// and save it in epoch[], little speed-up trick to avoid resetting
					// m.attacks[] after every failed attempt.
					for (++cnt, i = 0; i < size; ++i)
					{
						unsigned idx = m.index(occupancy[i]);

						if (epoch[idx] < cnt)
						{
							epoch[idx] = cnt;
							m.attacks[idx] = reference[i];
						}
						else if (m.attacks[idx] != reference[i])
							break;
					}
				}
			}
		}
		// prepare the connectivity mask here to check the connection of coins
		bitboard::buildConnectivityMask();
	}

	void buildConnectivityMask() {
		for (Square sq = SQ_A1; sq <= SQ_H8; ++sq)
		{
			for (int n = 0; n < 4; ++n)
			{
				connectivityMaskOrtho[sq][n] = expected_flips(ORTHO, sq, 0, true, n);
			}
			for (int n = 0; n < 4; ++n)
			{
				connectivityMaskDiago[sq][n] = expected_flips(DIAGO, sq, 0, true, n);
			}
		}
	}

	// potential flipping, checking the first end of the same piece, edges are also considered, edges only not considered for occupancies
	Bitboard expected_flips(DirectionType dr, Square sq, Bitboard occupied, bool singleDirection, int direction_selected) {
		Bitboard  attacks = 0;
		Direction OrthoDirections[4] = { NORTH, SOUTH, EAST, WEST };
		Direction DiagoDirections[4] = { NORTH_EAST, SOUTH_EAST, SOUTH_WEST, NORTH_WEST };

		if (singleDirection == false)
		{
			for (Direction d : (dr == ORTHO ? OrthoDirections : DiagoDirections))
			{
				Square s = sq;
				while (safe_destination(s, d))
				{
					attacks |= (s += d);
					if (occupied & s)
					{
						break;
					}
				}
			}
		}
		else
		{
			Square s = sq;
			Direction d;
			if (dr == ORTHO)
				d = OrthoDirections[direction_selected];
			else if (dr == DIAGO)
				d = DiagoDirections[direction_selected];

			while (safe_destination(s, d))
			{
				attacks |= (s += d);
			}
		}

		return attacks;
	}

	void boardViewer(const Bitboard& boardB, const Bitboard& boardW)
	{
		//decode the bitboard
		Bitboard boardBtmp = boardB;
		Bitboard boardWtmp = boardW;
		int n = 0;
		char board_char_arr[64] = {};
		for (int m = 0; m < 64; ++m)
		{
			board_char_arr[n++] = boardBtmp & 1 ? 'x' : boardWtmp & 1 ? 'o' : '-';
			boardBtmp >>= 1;
			boardWtmp >>= 1;
		}
		std::cout << "\npretty board:\n";
		for (int m = 7; m >= 0; --m)
		{
			for (int i = (8 * m) + 7; i >= 8 * m; --i)
			{
				std::cout << board_char_arr[i];
				if (i % 8 == 0)
					std::cout << std::endl;
			}
		}
		std::cout << "\nend board\n";
	}

	// this is the actual flips
	Bitboard actual_flips(const Square& sq, const Bitboard& Black_occupied, const Bitboard& White_occupied) {
		Bitboard flipped(0);

		Bitboard ortho_rays = magics[sq][ORTHO - DIAGO].rays_bb(Black_occupied);
		Bitboard ortho_rayBlockers = Black_occupied & ortho_rays;

		Bitboard diago_rays = magics[sq][DIAGO - DIAGO].rays_bb(Black_occupied);
		Bitboard diago_rayBlockers = Black_occupied & diago_rays;
		Bitboard dummy = 0;

		for (int d = 0; d < 4; ++d)
		{
			const Bitboard& cMask = connectivityMaskOrtho[sq][d];
			const Bitboard& rb = ortho_rayBlockers & cMask;

			Bitboard flip(0);
			if (rb ^ 0) {
				const Bitboard& oRay = ortho_rays & cMask;
				const Bitboard& oOcc = White_occupied & cMask;
				const Bitboard& comb = (oOcc & oRay) | rb;
				if (oRay == comb)
					flip = oOcc & oRay;
			}
			flipped |= flip;
		}
		for (int d = 0; d < 4; ++d)
		{
			const Bitboard& cMask = connectivityMaskDiago[sq][d];
			const Bitboard& rb = diago_rayBlockers & cMask;

			Bitboard flip(0);
			if (rb ^ 0) {
				const Bitboard& oRay = diago_rays & cMask;
				const Bitboard& oOcc = White_occupied & cMask;
				const Bitboard& comb = (oOcc & oRay) | rb;
				if (oRay == comb)
					flip = oOcc & oRay;
			}
			flipped |= flip;
		}
		// boardViewer(flipped, dummy);

		return flipped;
	}
}