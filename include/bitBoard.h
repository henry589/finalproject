#ifndef BITBOARD_H
#define BITBOARD_H
#include <bitset>
#include <cassert>
#include <random>
#include <iostream>
namespace bitboard{

enum Square : int {
    SQ_A1, SQ_B1, SQ_C1, SQ_D1, SQ_E1, SQ_F1, SQ_G1, SQ_H1,
    SQ_A2, SQ_B2, SQ_C2, SQ_D2, SQ_E2, SQ_F2, SQ_G2, SQ_H2,
    SQ_A3, SQ_B3, SQ_C3, SQ_D3, SQ_E3, SQ_F3, SQ_G3, SQ_H3,
    SQ_A4, SQ_B4, SQ_C4, SQ_D4, SQ_E4, SQ_F4, SQ_G4, SQ_H4,
    SQ_A5, SQ_B5, SQ_C5, SQ_D5, SQ_E5, SQ_F5, SQ_G5, SQ_H5,
    SQ_A6, SQ_B6, SQ_C6, SQ_D6, SQ_E6, SQ_F6, SQ_G6, SQ_H6,
    SQ_A7, SQ_B7, SQ_C7, SQ_D7, SQ_E7, SQ_F7, SQ_G7, SQ_H7,
    SQ_A8, SQ_B8, SQ_C8, SQ_D8, SQ_E8, SQ_F8, SQ_G8, SQ_H8,
    SQ_NONE,

    SQUARE_ZERO = 0,
    SQUARE_NB   = 64
};
enum Rank : int {
    RANK_1,
    RANK_2,
    RANK_3,
    RANK_4,
    RANK_5,
    RANK_6,
    RANK_7,
    RANK_8,
    RANK_NB
};

enum File : int {
    FILE_A,
    FILE_B,
    FILE_C,
    FILE_D,
    FILE_E,
    FILE_F,
    FILE_G,
    FILE_H,
    FILE_NB
};

using Bitboard = uint64_t;
constexpr Bitboard Rank1BB = 0xFF;
constexpr Bitboard Rank8BB = Rank1BB << (8 * 7);
constexpr Bitboard FileABB = 0x0101010101010101ULL;
constexpr Bitboard FileHBB = FileABB << 7;

constexpr Bitboard rank_bb(Rank r) { return Rank1BB << (8 * r); }
constexpr Rank rank_of(Square s) { return Rank(s >> 3); }
constexpr File file_of(Square s) { return File(s & 7); }

constexpr Bitboard rank_bb(Square s) { return rank_bb(rank_of(s)); }
constexpr Bitboard file_bb(File f) { return FileABB << f; }

constexpr Bitboard file_bb(Square s) { return file_bb(file_of(s)); }

#define ENABLE_INCR_OPERATORS_ON(T) \
inline T& operator++(T& d) { return d = T(int(d) + 1); } \
inline T& operator--(T& d) { return d = T(int(d) - 1); }

ENABLE_INCR_OPERATORS_ON(Square)
ENABLE_INCR_OPERATORS_ON(Rank)

struct Magic {
    Bitboard  mask;
    Bitboard* attacks;
    Bitboard magic;
    unsigned shift;

    // Compute the attack's index using the 'magic bitboards' approach
    unsigned index(Bitboard occupied) const {


            return unsigned(((occupied & mask) * magic) >> shift);

     }

    Bitboard attacks_bb(Bitboard occupied) const { return attacks[index(occupied)]; }
};
alignas(64) Magic magics[SQUARE_NB][2];
enum Direction : int {
    NORTH = 8,
    EAST  = 1,
    SOUTH = -NORTH,
    WEST  = -EAST,

    NORTH_EAST = NORTH + EAST,
    SOUTH_EAST = SOUTH + EAST,
    SOUTH_WEST = SOUTH + WEST,
    NORTH_WEST = NORTH + WEST
};
enum DirectionType {
    DIAGO, 
    ORTHO
};
constexpr bool is_ok(Square s) { return s >= SQ_A1 && s <= SQ_H8; }

constexpr Bitboard square_bb(Square s) {
    assert(is_ok(s));
    return (1ULL << s);
}
inline Bitboard  operator&(Bitboard b, Square s) { return b & square_bb(s); }
inline Bitboard  operator|(Bitboard b, Square s) { return b | square_bb(s); }
inline Bitboard  operator^(Bitboard b, Square s) { return b ^ square_bb(s); }
inline Bitboard& operator|=(Bitboard& b, Square s) { return b |= square_bb(s); }
inline Bitboard& operator^=(Bitboard& b, Square s) { return b ^= square_bb(s); }

inline Bitboard operator&(Square s, Bitboard b) { return b & s; }
inline Bitboard operator|(Square s, Bitboard b) { return b | s; }
inline Bitboard operator^(Square s, Bitboard b) { return b ^ s; }
inline Bitboard operator|(Square s1, Square s2) { return square_bb(s1) | s2; }


constexpr Square operator+(Square s, Direction d) { return Square(int(s) + int(d)); }
constexpr Square operator-(Square s, Direction d) { return Square(int(s) - int(d)); }
inline Square&   operator+=(Square& s, Direction d) { return s = s + d; }
uint8_t SquareDistance[SQUARE_NB][SQUARE_NB];
Bitboard OrthoTable[0x19000];  // to store orthogonal attacks
Bitboard DiagoTable[0x1480];  // To store diagonal attacks


template<typename T1 = Square>
inline int distance(Square x, Square y);

template<>
inline int distance<File>(Square x, Square y) {
    return std::abs(file_of(x) - file_of(y));
}

template<>
inline int distance<Rank>(Square x, Square y) {
    return std::abs(rank_of(x) - rank_of(y));
}

template<>
inline int distance<Square>(Square x, Square y) {
    return SquareDistance[x][y];
}



Bitboard safe_destination(Square s, int step) {
    Square to = Square(s + step);
    return is_ok(to) && distance(s, to) <= 2 ? square_bb(to) : Bitboard(0);
}

void boardViewer(uint64_t &boardB, uint64_t &boardW)
{
    //decode the bitboard
    uint64_t boardBtmp = boardB;
    uint64_t boardWtmp = boardW;
    int n = 0;
    char board_char_arr[64] = {};
    for(int m =0; m < 64; ++m)
    {
        board_char_arr[n++] = boardBtmp & 1 ? 'x' : boardWtmp & 1 ? 'o' : '-';
        boardBtmp >>= 1;
        boardWtmp >>= 1;
    }
    std::cout<<"\npretty board:\n";
    for (int m = 7; m >= 0; --m)
    {
        for(int i = (8 * m) + 7; i >= 8 * m; --i)
        {

            std::cout<<board_char_arr[i];
            if(i % 8 == 0)
            std::cout<<std::endl;
        }
    }
    std::cout<<"\nend board\n";

}
// potential flipping, checking the first end of the same piece, edges are also considered, edges only not considered for occupancies
Bitboard expected_flips(DirectionType dr, Square sq, Bitboard occupied, bool singleDirection=false, int direction_selected=0) {

    Bitboard  attacks             = 0;
    Direction OrthoDirections[4]   = {NORTH, SOUTH, EAST, WEST};
    Direction DiagoDirections[4] = {NORTH_EAST, SOUTH_EAST, SOUTH_WEST, NORTH_WEST};

    if(singleDirection == false)
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
    }else
    {
        Square s = sq;
        Direction d;
        if(dr == ORTHO)
            d = OrthoDirections[direction_selected];
        else if(dr == DIAGO)
            d = DiagoDirections[direction_selected];

        while (safe_destination(s, d))
        {
            attacks |= (s += d);
        }
    }

    return attacks;
}
Bitboard connectivityMaskOrtho[SQUARE_NB][4];
Bitboard connectivityMaskDiago[SQUARE_NB][4];

void buildConnectivityMask(){
    for(Square sq = SQ_A1; sq <= SQ_H8; ++sq)
    {
        for(int n=0; n < 4; ++n)
        {
            connectivityMaskOrtho[sq][n] = expected_flips(ORTHO, sq, 0, true, n);
        }
        for(int n=0; n < 4; ++n)
        {
            connectivityMaskDiago[sq][n] = expected_flips(DIAGO, sq, 0, true, n);
        }
    }
}

// this is the actual flips
inline Bitboard actual_flips(Square & sq, const Bitboard & Player_occupied, const Bitboard & Opponent_occupied) {

    Bitboard  board             = 0;

    Bitboard ortho_expected_flips = magics[sq][ORTHO - DIAGO].attacks_bb(Player_occupied);
    Bitboard ortho_outmost_coin = Player_occupied & ortho_expected_flips;

    Bitboard diago_expected_flips = magics[sq][DIAGO - DIAGO].attacks_bb(Player_occupied);
    Bitboard diago_outmost_coin = Player_occupied & diago_expected_flips;
    Bitboard dummy = 0;

    boardViewer(ortho_expected_flips, dummy);
    for (int d = 0; d < 4; ++d)
    {
        const Bitboard & cMask = connectivityMaskOrtho[sq][d];
        const Bitboard & x = Opponent_occupied & cMask;
        const Bitboard & outmost_tmp = ortho_outmost_coin & cMask;
        const Bitboard & exp_tmp = ortho_expected_flips & cMask;
        const Bitboard & x_or_outmost_tmp = x | outmost_tmp;
        const Bitboard & flip = exp_tmp ^ x_or_outmost_tmp ? 0 : x;
        board |= flip;
    }
    for (int d = 0; d < 4; ++d)
    {
        const Bitboard & cMask = connectivityMaskDiago[sq][d];
        const Bitboard & x = Opponent_occupied & cMask;
        const Bitboard & outmost_tmp = diago_outmost_coin & cMask;
        const Bitboard & exp_tmp = diago_expected_flips & cMask;
        const Bitboard & x_or_outmost_tmp = x | outmost_tmp;
        const Bitboard & flip = exp_tmp ^ x_or_outmost_tmp ? 0 : x;
        board |= flip;
    }
    boardViewer(board, dummy);

    return board;
}

class PRNG {

    uint64_t s;

    uint64_t rand64() {

        s ^= s >> 12, s ^= s << 25, s ^= s >> 27;
        return s * 2685821657736338717LL;
    }

   public:
    PRNG(uint64_t seed) :
        s(seed) {
        assert(seed);
    }

    template<typename T>
    T rand() {
        return T(rand64());
    }

    // Special generator used to fast init magic numbers.
    // Output values only have 1/8th of their bits set on average.
    template<typename T>
    T sparse_rand() {
        return T(rand64() & rand64() & rand64());
    }
};
inline int popcount(Bitboard b) {
  
    return __builtin_popcountll(b);
    
}
void magicBitboard(DirectionType dr, Bitboard table[])
{
    // std::random_device rd;
    // std::mt19937_64 generator(rd()); // Randomly seeded 64-bit Mersenne Twister
    // std::uniform_int_distribution<uint64_t> dist(0, UINT64_MAX);


    for (Square s1 = SQ_A1; s1 <= SQ_H8; ++s1)
        for (Square s2 = SQ_A1; s2 <= SQ_H8; ++s2)
            SquareDistance[s1][s2] = std::max(distance<File>(s1, s2), distance<Rank>(s1, s2));
    
    int      size = 0;
    Bitboard occupancy[4096];
    int      epoch[4096] = {}, cnt = 0;
    Bitboard reference[4096];
    int seeds[][RANK_NB] = {{8977, 44560, 54343, 38998, 5731, 95205, 104912, 17020},
    {728, 10316, 55013, 32803, 12281, 15100, 16645, 255}};
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
        m.mask   = expected_flips(dr, s, 0) & ~edges;
        m.shift = 64 - popcount(m.mask);
        // Set the offset for the attacks table of the square. We have individual
        // table sizes for each square with "Fancy Magic Bitboards".
        m.attacks = s == SQ_A1 ? table : magics[s - 1][dr - DIAGO].attacks + size;
        size      = 0;

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
                    epoch[idx]     = cnt;
                    m.attacks[idx] = reference[i];
                }
                else if (m.attacks[idx] != reference[i])
                    break;
            }
        }

    }
}


};

#endif