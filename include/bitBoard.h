#ifndef BITBOARD_H
#define BITBOARD_H
#include <bitset>
#include <cassert>
#include <random>
#include <iostream>
#include <immintrin.h>
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
enum Direction : int {
    NORTH = 8,
    EAST = 1,
    SOUTH = -NORTH,
    WEST = -EAST,

    NORTH_EAST = NORTH + EAST,
    SOUTH_EAST = SOUTH + EAST,
    SOUTH_WEST = SOUTH + WEST,
    NORTH_WEST = NORTH + WEST
};
using Bitboard = uint64_t;

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

//inline Bitboard operator&(Square s, Bitboard b) { return b & s; }
inline Bitboard operator|(Square s, Bitboard b) { return b | s; }
inline Bitboard operator^(Square s, Bitboard b) { return b ^ s; }
inline Bitboard operator|(Square s1, Square s2) { return square_bb(s1) | s2; }


constexpr Square operator+(Square s, Direction d) { return Square(int(s) + int(d)); }
constexpr Square operator-(Square s, Direction d) { return Square(int(s) - int(d)); }
inline Square& operator+=(Square& s, Direction d) { return s = s + d; }

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

    Bitboard rays_bb(Bitboard occupied) const { return attacks[index(occupied)]; }
};
extern uint8_t SquareDistance[SQUARE_NB][SQUARE_NB];
extern Magic magics[SQUARE_NB][2];
extern Bitboard connectivityMaskOrtho[SQUARE_NB][4];
extern Bitboard connectivityMaskDiago[SQUARE_NB][4];

enum DirectionType {
    DIAGO, 
    ORTHO
};

enum Side : bool{
    BLACK = false,
    WHITE = true
};




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

Bitboard safe_destination(Square s, int step);



void init_Bitboards();
void buildConnectivityMask();
void boardViewer(const Bitboard &boardB, const Bitboard &boardW);
Bitboard actual_flips(const Square & sq, const Bitboard & Black_occupied, const Bitboard & White_occupied);
Bitboard expected_flips(DirectionType dr, Square sq, Bitboard occupied, bool singleDirection=false, int direction_selected=0);

// Bitboard braceMask[SQUARE_NB];
// void surroundingChecker()
// {
//     Direction MixedDirections[]   = {NORTH, SOUTH, EAST, WEST, NORTH_EAST, SOUTH_EAST, SOUTH_WEST, NORTH_WEST};
//     const int max = 1;
//     Bitboard mtest= 0;
//     for(Square sq = SQ_A1; sq <= SQ_H8; ++sq)
//     {
//         int n = 0;
//         for(Direction d : MixedDirections)
//         {

//             int count = 0;
//             Square s = sq;
//             while (safe_destination(s, d))
//             {
//                 braceMask[sq]  |= (s += d);
//                 count++;
//                 if(count >= max)
//                 {
//                     break;
//                 }
//             }            
//         }
//     }
    
//     // Bitboard boardx = 0;
//     // boardViewer(braceMask[SQ_F5], boardx);
// }



// inline bool validMove(const Square & sq, const Bitboard & Black_occupied, const Bitboard & White_occupied)
// {
//     // assume the square sq is empty
//     const Bitboard & future_flips = actual_flips(sq, Black_occupied, White_occupied);
//     return  future_flips ^ 0; // means got possible flips
// }

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
  
    return int(_mm_popcnt_u64(b));
    
}


};

#endif