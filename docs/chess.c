// This is a chess engine called Fissure

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>
#include <stdlib.h>
#include <time.h>

#define MAX_MOVES 					256
#define MOVE_NORMAL					0
#define MOVE_EN_PASSANT 			1
#define MOVE_PROMOTE_QUEEN  		2
#define MOVE_PROMOTE_ROOK   		3
#define MOVE_PROMOTE_BISHOP 		4
#define MOVE_PROMOTE_KNIGHT 		5
#define MOVE_CASTLE_KINGSIDE  		6
#define MOVE_CASTLE_QUEENSIDE 		7
#define MAX_GAME_PLIES				512
#define CHALLENGERS_PER_GENERATION	4

// File Masks
const uint64_t FILE_A = 0x0101010101010101ULL;
const uint64_t FILE_B = 0x0202020202020202ULL;
const uint64_t FILE_C = 0x0404040404040404ULL;
const uint64_t FILE_D = 0x0808080808080808ULL;
const uint64_t FILE_E = 0x1010101010101010ULL;
const uint64_t FILE_F = 0x2020202020202020ULL;
const uint64_t FILE_G = 0x4040404040404040ULL;
const uint64_t FILE_H = 0x8080808080808080ULL;

// Rank Masks
const uint64_t RANK_1 = 0x00000000000000FFULL;
const uint64_t RANK_2 = 0x000000000000FF00ULL;
const uint64_t RANK_3 = 0x0000000000FF0000ULL;
const uint64_t RANK_4 = 0x00000000FF000000ULL;
const uint64_t RANK_5 = 0x000000FF00000000ULL;
const uint64_t RANK_6 = 0x0000FF0000000000ULL;
const uint64_t RANK_7 = 0x00FF000000000000ULL;
const uint64_t RANK_8 = 0xFF00000000000000ULL;

// Color Masks
const uint64_t DARK_SQUARES  = 0xAA55AA55AA55AA55ULL;
const uint64_t LIGHT_SQUARES = 0x55AA55AA55AA55AAULL;

typedef struct {
    const char *name;
    const char *fen;
} TrainingPosition;

TrainingPosition training_positions[] =
{
    {
        "Setup 1",
        "rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq e6 0 2"
    },
    {
        "Setup 2",
        "rnbqkbnr/ppp1pppp/8/3p4/3P4/8/PPP1PPPP/RNBQKBNR w KQkq d6 0 2"
    },
    {
        "Setup 3",
        "1r3rk1/p4pp1/4pn1p/8/PqN5/4Q1P1/1P2PP1P/R1R3K1 w - - 9 27"
    },
    {
        "Setup 4",
        "r1bqr1k1/1p1n1pbp/p2p1np1/2pP2B1/4P3/2N2PN1/PP1Q2PP/R3KB1R w KQ - 0 12"
    },
    {
        "Setup 5",
        "rnb1kb1r/p4p1p/1qp1pp2/1p6/P1pPP3/2N2N2/1P2BPPP/R2QK2R b KQkq - 1 9"
    },
    {
        "Setup 6",
        "r1q2nk1/2bb2np/p2p2p1/2pPpp2/2P1P3/PQN2NPP/1R1B1P1K/5B2 w - - 7 27"
    },
    {
        "Setup 7",
        "2rbn1k1/Brqb1p1p/R2p2p1/1p1Pp3/1Q2P3/3B3P/1P1N1PPK/R7 b - - 1 30"
    },
    {
        "Setup 8",
        "r2q1rk1/ppp1bpp1/2n4p/3np3/N1B3b1/P2PBN2/1PP1QPPP/R4RK1 b - - 3 11"
    },
    {
        "Setup 9",
        "r1bbqrk1/ppp3pp/3pp3/1N1P1p2/1nP1n3/5NP1/PPQ1PPBP/R1BR2K1 w - - 1 13"
    },
    {
        "Setup 10",
        "rnbqk1nr/pp3ppp/4p3/2ppP3/1b1P4/2N5/PPP2PPP/R1BQKBNR w KQkq c6 0 5"
    }
};

int training_position_count =
    sizeof(training_positions) / sizeof(training_positions[0]);

uint64_t white_pawns;
uint64_t white_knights;
uint64_t white_bishops;
uint64_t white_rooks;
uint64_t white_queens;
uint64_t white_king;

uint64_t black_pawns;
uint64_t black_knights;
uint64_t black_bishops;
uint64_t black_rooks;
uint64_t black_queens;
uint64_t black_king;

uint64_t white_occupation;
uint64_t black_occupation;
uint64_t total_occupation;
uint64_t empty_occupation;

int en_passant_square = -1;
int white_can_castle_kingside = 1;
int white_can_castle_queenside = 1;
int black_can_castle_kingside = 1;
int black_can_castle_queenside = 1;

void generate_castling_moves();

void print_bitboard(uint64_t bb)
{
    printf("\n");

    for(int rank = 7; rank >= 0; rank--)
    {
        printf("%d ", rank + 1);

        for(int file = 0; file < 8; file++)
        {
            int square = rank * 8 + file;

            if(bb & (1ULL << square))
                printf("1 ");
            else
                printf(". ");
        }

        printf("\n");
    }

    printf("  a b c d e f g h\n\n");
}

void setup()
{
	white_pawns = 0x000000000000FF00ULL;
	black_pawns = 0x00FF000000000000ULL;

	white_knights = 0x0000000000000042ULL;
	black_knights = 0x4200000000000000ULL;

	white_bishops = 0x0000000000000024ULL;
	black_bishops = 0x2400000000000000ULL;

	white_rooks = 0x0000000000000081ULL;
	black_rooks = 0x8100000000000000ULL;

	white_queens = 0x0000000000000008ULL;
	black_queens = 0x0800000000000000ULL;

	white_king = 0x0000000000000010ULL;
	black_king = 0x1000000000000000ULL;
}

void test_setup()
{
	white_pawns   = 0x0000008010066100ULL;
	black_pawns   = 0x0004A15200000000ULL;

	white_knights = 0x0000002000000000ULL;
	black_knights = 0x0400000000000000ULL;

	white_bishops = 0x0000000000100008ULL;
	black_bishops = 0x2000100000000000ULL;

	white_rooks   = 0x0000000000000011ULL;
	black_rooks   = 0x1008000000000000ULL;

	white_queens  = 0x0000000000800000ULL;
	black_queens  = 0x0020000000000000ULL;

	white_king    = 0x0000000000000040ULL;
	black_king    = 0x0080000000000000ULL;	
}

void occupation()
{
	white_occupation = white_pawns | white_knights | white_bishops | white_rooks | white_queens | white_king;
	black_occupation = black_pawns | black_knights | black_bishops | black_rooks | black_queens | black_king;
	total_occupation = white_occupation | black_occupation;
}

// Move and Position Functions
enum Position {
	a1, b1, c1, d1, e1, f1, g1, h1,
	a2, b2, c2, d2, e2, f2, g2, h2,
	a3, b3, c3, d3, e3, f3, g3, h3,
	a4, b4, c4, d4, e4, f4, g4, h4,
	a5, b5, c5, d5, e5, f5, g5, h5,
	a6, b6, c6, d6, e6, f6, g6, h6,
	a7, b7, c7, d7, e7, f7, g7, h7,
	a8, b8, c8, d8, e8, f8, g8, h8
};

typedef struct
{
	enum Position from;
	enum Position to;
	int flags;
} Move;

Move move_list[MAX_MOVES];
int move_count = 0;

void add_move(Move move_list[], int *move_count, enum Position from, enum Position to, int flags)
{
    move_list[*move_count].from = from;
    move_list[*move_count].to = to;
    move_list[*move_count].flags = flags;

    (*move_count)++;
}

void add_promotion_moves(enum Position from, enum Position to)
{
    add_move(move_list, &move_count, from, to, MOVE_PROMOTE_QUEEN);
    add_move(move_list, &move_count, from, to, MOVE_PROMOTE_ROOK);
    add_move(move_list, &move_count, from, to, MOVE_PROMOTE_BISHOP);
    add_move(move_list, &move_count, from, to, MOVE_PROMOTE_KNIGHT);
}

const char *piece_on_square(enum Position square)
{
    uint64_t mask = 1ULL << square;

    if(white_pawns   & mask) return "White Pawn";
    if(white_knights & mask) return "White Knight";
    if(white_bishops & mask) return "White Bishop";
    if(white_rooks   & mask) return "White Rook";
    if(white_queens  & mask) return "White Queen";
    if(white_king    & mask) return "White King";

    if(black_pawns   & mask) return "Black Pawn";
    if(black_knights & mask) return "Black Knight";
    if(black_bishops & mask) return "Black Bishop";
    if(black_rooks   & mask) return "Black Rook";
    if(black_queens  & mask) return "Black Queen";
    if(black_king    & mask) return "Black King";

    return "Unknown";
}

void print_move(Move move)
{
    char from_file = 'a' + (move.from % 8);
    int from_rank = (move.from / 8) + 1;

    char to_file = 'a' + (move.to % 8);
    int to_rank = (move.to / 8) + 1;

    printf("%s: %c%d -> %c%d",
           piece_on_square(move.from),
           from_file, from_rank,
           to_file, to_rank);

    switch(move.flags)
    {
        case MOVE_EN_PASSANT:
            printf(" e.p.");
            break;
        case MOVE_PROMOTE_QUEEN:
            printf("=Q");
            break;
        case MOVE_PROMOTE_ROOK:
            printf("=R");
            break;
        case MOVE_PROMOTE_BISHOP:
            printf("=B");
            break;
        case MOVE_PROMOTE_KNIGHT:
            printf("=N");
            break;
        case MOVE_CASTLE_KINGSIDE:
            printf(" O-O");
            break;
        case MOVE_CASTLE_QUEENSIDE:
            printf(" O-O-O");
            break;
    }

    printf("\n");
}

void print_move_list(Move move_list[], int move_count)
{
    for(int i = 0; i < move_count; i++)
    {
        print_move(move_list[i]);
    }
}

void print_square(enum Position pos)
{
    printf("%c%d\n", 'a' + (pos % 8), 1 + (pos / 8));
}

void find_rank(enum Position pos)
{
	int rank = pos / 8;
	printf("Rank: %d\n", rank + 1);
}

void find_file(enum Position pos)
{
    char file = 'a' + (pos % 8);
    printf("File: %c\n", file);
}

// Move and Capture Functions
// Pawns
uint64_t white_pawn_moves()
{
    uint64_t empty = ~total_occupation;
    uint64_t single = (white_pawns << 8) & empty;

    uint64_t rank3 = 0x0000000000FF0000ULL;

    uint64_t double_push = ((single & rank3) << 8) & empty;

    return single | double_push;
}

uint64_t white_pawn_captures()
{
    uint64_t captures_left =
        ((white_pawns & ~FILE_A) << 7) & black_occupation;

    uint64_t captures_right =
        ((white_pawns & ~FILE_H) << 9) & black_occupation;

    return captures_left | captures_right;
}

uint64_t black_pawn_moves()
{
    uint64_t empty = ~total_occupation;
    uint64_t single = (black_pawns >> 8) & empty;

    uint64_t rank6 = 0x0000FF0000000000ULL;

    uint64_t double_push = ((single & rank6) >> 8) & empty;

    return single | double_push;
}

uint64_t black_pawn_captures()
{
    uint64_t captures_left =
        ((black_pawns & ~FILE_A) >> 9) & white_occupation;

    uint64_t captures_right =
        ((black_pawns & ~FILE_H) >> 7) & white_occupation;

    return captures_left | captures_right;
}

uint64_t white_pawn_attacks()
{
    return ((white_pawns & ~FILE_A) << 7) | ((white_pawns & ~FILE_H) << 9);
}

uint64_t black_pawn_attacks()
{
    return ((black_pawns & ~FILE_A) >> 9) | ((black_pawns & ~FILE_H) >> 7);
}

// Knights
uint64_t knight_attacks(uint64_t knights)
{
    uint64_t l1 = (knights >> 1) & ~FILE_H;
    uint64_t l2 = (knights >> 2) & ~(FILE_H | FILE_G);

    uint64_t r1 = (knights << 1) & ~FILE_A;
    uint64_t r2 = (knights << 2) & ~(FILE_A | FILE_B);

    uint64_t h1 = l1 | r1;
    uint64_t h2 = l2 | r2;

    return (h1 << 16) | (h1 >> 16) | (h2 << 8) | (h2 >> 8);
}

uint64_t white_knight_moves()
{
    uint64_t attacks = knight_attacks(white_knights);

    return attacks & ~total_occupation;
}

uint64_t white_knight_captures()
{
    uint64_t attacks = knight_attacks(white_knights);

    return attacks & black_occupation;
}

uint64_t black_knight_moves()
{
    uint64_t attacks = knight_attacks(black_knights);

    return attacks & ~total_occupation;
}

uint64_t black_knight_captures()
{
    uint64_t attacks = knight_attacks(black_knights);

    return attacks & white_occupation;
}

// Bishops
uint64_t bishop_attacks(int square)
{
    uint64_t attacks = 0;

    int rank = square / 8;
    int file = square % 8;

    // Northeast
    int r = rank + 1;
    int f = file + 1;

    while(r < 8 && f < 8)
    {
        int target = r * 8 + f;

        attacks |= (1ULL << target);

        if(total_occupation & (1ULL << target))
            break;

        r++;
        f++;
    }

    // Northwest
    r = rank + 1;
    f = file - 1;

    while(r < 8 && f >= 0)
    {
        int target = r * 8 + f;

        attacks |= (1ULL << target);

        if(total_occupation & (1ULL << target))
            break;

        r++;
        f--;
    }

    // Southeast
    r = rank - 1;
    f = file + 1;

    while(r >= 0 && f < 8)
    {
        int target = r * 8 + f;

        attacks |= (1ULL << target);

        if(total_occupation & (1ULL << target))
            break;

        r--;
        f++;
    }

    // Southwest
    r = rank - 1;
    f = file - 1;

    while(r >= 0 && f >= 0)
    {
        int target = r * 8 + f;

        attacks |= (1ULL << target);

        if(total_occupation & (1ULL << target))
            break;

        r--;
        f--;
    }

    return attacks;
}

uint64_t white_bishop_attacks()
{
    uint64_t attacks = 0;

    for(int square = 0; square < 64; square++)
    {
        if(white_bishops & (1ULL << square))
        {
            attacks |= bishop_attacks(square);
        }
    }

    return attacks;
}

uint64_t black_bishop_attacks()
{
    uint64_t attacks = 0;

    for(int square = 0; square < 64; square++)
    {
        if(black_bishops & (1ULL << square))
        {
            attacks |= bishop_attacks(square);
        }
    }

    return attacks;
}

uint64_t white_bishop_moves()
{
    return white_bishop_attacks() & ~total_occupation;
}

uint64_t white_bishop_captures()
{
    return white_bishop_attacks() & black_occupation;
}

uint64_t black_bishop_moves()
{
    return black_bishop_attacks() & ~total_occupation;
}

uint64_t black_bishop_captures()
{
    return black_bishop_attacks() & white_occupation;
}

// Rooks
uint64_t rook_attacks(int square)
{
    uint64_t attacks = 0;

    int rank = square / 8;
    int file = square % 8;

    // North
    int r = rank + 1;

    while(r < 8)
    {
        int target = r * 8 + file;

        attacks |= (1ULL << target);

        if(total_occupation & (1ULL << target))
            break;

        r++;
    }

    // South
    r = rank - 1;

    while(r >= 0)
    {
        int target = r * 8 + file;

        attacks |= (1ULL << target);

        if(total_occupation & (1ULL << target))
            break;

        r--;
    }

    // East
    int f = file + 1;

    while(f < 8)
    {
        int target = rank * 8 + f;

        attacks |= (1ULL << target);

        if(total_occupation & (1ULL << target))
            break;

        f++;
    }

    // West
    f = file - 1;

    while(f >= 0)
    {
        int target = rank * 8 + f;

        attacks |= (1ULL << target);

        if(total_occupation & (1ULL << target))
            break;

        f--;
    }

    return attacks;
}

uint64_t white_rook_attacks()
{
    uint64_t attacks = 0;

    for(int square = 0; square < 64; square++)
    {
        if(white_rooks & (1ULL << square))
        {
            attacks |= rook_attacks(square);
        }
    }

    return attacks;
}

uint64_t black_rook_attacks()
{
    uint64_t attacks = 0;

    for(int square = 0; square < 64; square++)
    {
        if(black_rooks & (1ULL << square))
        {
            attacks |= rook_attacks(square);
        }
    }

    return attacks;
}

uint64_t white_rook_moves()
{
    return white_rook_attacks() & ~total_occupation;
}

uint64_t white_rook_captures()
{
    return white_rook_attacks() & black_occupation;
}

uint64_t black_rook_moves()
{
    return black_rook_attacks() & ~total_occupation;
}

uint64_t black_rook_captures()
{
    return black_rook_attacks() & white_occupation;
}

// Queens
uint64_t queen_attacks(int square)
{
    return bishop_attacks(square) | rook_attacks(square);
}

uint64_t white_queen_attacks()
{
    uint64_t attacks = 0;

    for(int square = 0; square < 64; square++)
    {
        if(white_queens & (1ULL << square))
        {
            attacks |= queen_attacks(square);
        }
    }

    return attacks;
}

uint64_t black_queen_attacks()
{
    uint64_t attacks = 0;

    for(int square = 0; square < 64; square++)
    {
        if(black_queens & (1ULL << square))
        {
            attacks |= queen_attacks(square);
        }
    }

    return attacks;
}

uint64_t white_queen_moves()
{
    return white_queen_attacks() & ~total_occupation;
}

uint64_t white_queen_captures()
{
    return white_queen_attacks() & black_occupation;
}

uint64_t black_queen_moves()
{
    return black_queen_attacks() & ~total_occupation;
}

uint64_t black_queen_captures()
{
    return black_queen_attacks() & white_occupation;
}

// Kings
uint64_t king_attacks(uint64_t king)
{
    uint64_t attacks = 0;

    attacks |= king << 8;                          // North
    attacks |= king >> 8;                          // South

    attacks |= (king << 1) & ~FILE_A;              // East
    attacks |= (king >> 1) & ~FILE_H;              // West

    attacks |= (king << 9) & ~FILE_A;              // NE
    attacks |= (king << 7) & ~FILE_H;              // NW

    attacks |= (king >> 7) & ~FILE_A;              // SE
    attacks |= (king >> 9) & ~FILE_H;              // SW

    return attacks;
}

uint64_t white_king_attacks()
{
    return king_attacks(white_king);
}

uint64_t black_king_attacks()
{
    return king_attacks(black_king);
}

uint64_t white_king_moves()
{
    return white_king_attacks() & ~total_occupation;
}

uint64_t white_king_captures()
{
    return white_king_attacks() & black_occupation;
}

uint64_t black_king_moves()
{
    return black_king_attacks() & ~total_occupation;
}

uint64_t black_king_captures()
{
    return black_king_attacks() & white_occupation;
}

// conversion from bitboard
void add_moves_from_bitboard(enum Position from, uint64_t moves)
{
    for(int to = 0; to < 64; to++)
    {
        if(moves & (1ULL << to))
        {
            add_move(move_list, &move_count, from, (enum Position)to, MOVE_NORMAL);
        }
    }
}

void generate_white_pawn_moves()
{
    for(int from = 0; from < 64; from++)
    {
        if(white_pawns & (1ULL << from))
        {
            uint64_t from_bit = 1ULL << from;

            // single push
            uint64_t single = (from_bit << 8) & ~total_occupation;
            for(int to = 0; to < 64; to++)
            {
                if(single & (1ULL << to))
                {
                    if((1ULL << to) & RANK_8)
                        add_promotion_moves((enum Position)from, (enum Position)to);
                    else
                        add_move(move_list, &move_count, (enum Position)from, (enum Position)to, MOVE_NORMAL);
                }
            }

            // double push from rank 2 only
            if(from_bit & RANK_2)
            {
                uint64_t double_push = ((single & RANK_3) << 8) & ~total_occupation;
                add_moves_from_bitboard((enum Position)from, double_push);
            }

            // captures
            uint64_t capture_left = ((from_bit & ~FILE_A) << 7) & black_occupation;
            uint64_t capture_right = ((from_bit & ~FILE_H) << 9) & black_occupation;
            uint64_t captures = capture_left | capture_right;
            
            for(int to = 0; to < 64; to++)
            {
                if(captures & (1ULL << to))
                {
                    if((1ULL << to) & RANK_8)
                        add_promotion_moves((enum Position)from, (enum Position)to);
                    else
                        add_move(move_list, &move_count,
                                 (enum Position)from,
                                 (enum Position)to,
                                 MOVE_NORMAL);
                }
            }

            // en passant
            if(en_passant_square != -1)
            {
                uint64_t ep = 1ULL << en_passant_square;
                uint64_t ep_left = ((from_bit & ~FILE_A) << 7) & ep;
                uint64_t ep_right =((from_bit & ~FILE_H) << 9) & ep;
                uint64_t ep_moves = ep_left | ep_right;
            
                for(int to = 0; to < 64; to++)
                {
                    if(ep_moves & (1ULL << to))
                        add_move(move_list, &move_count, (enum Position)from, (enum Position)to, MOVE_EN_PASSANT);
                }
            }
        }
    }
}

void generate_white_knight_moves()
{
    for(int from = 0; from < 64; from++)
    {
        if(white_knights & (1ULL << from))
        {
            uint64_t moves = knight_attacks(1ULL << from) & ~white_occupation;
            add_moves_from_bitboard((enum Position)from, moves);
        }
    }
}

void generate_white_bishop_moves()
{
    for(int from = 0; from < 64; from++)
    {
        if(white_bishops & (1ULL << from))
        {
            uint64_t moves = bishop_attacks(from) & ~white_occupation;
            add_moves_from_bitboard((enum Position)from, moves);
        }
    }
}

void generate_white_rook_moves()
{
    for(int from = 0; from < 64; from++)
    {
        if(white_rooks & (1ULL << from))
        {
            uint64_t moves = rook_attacks(from) & ~white_occupation;
            add_moves_from_bitboard( (enum Position)from, moves);
        }
    }
}

void generate_white_queen_moves()
{
    for(int from = 0; from < 64; from++)
    {
        if(white_queens & (1ULL << from))
        {
            uint64_t moves = queen_attacks(from) & ~white_occupation;
            add_moves_from_bitboard( (enum Position)from, moves);
        }
    }
}

void generate_white_king_moves()
{
    for(int from = 0; from < 64; from++)
    {
        if(white_king & (1ULL << from))
        {
            uint64_t moves = king_attacks(1ULL << from) & ~white_occupation;
            add_moves_from_bitboard( (enum Position)from, moves);
        }
    }
}

void generate_white_moves()
{
    move_count = 0;

	generate_white_pawn_moves();
    generate_white_knight_moves();
    generate_white_bishop_moves();
    generate_white_rook_moves();
    generate_white_queen_moves();
    generate_white_king_moves();
    generate_castling_moves();
}

void generate_black_pawn_moves()
{
    for(int from = 0; from < 64; from++)
    {
        if(black_pawns & (1ULL << from))
        {
            uint64_t from_bit = 1ULL << from;

            // single push
            uint64_t single = (from_bit >> 8) & ~total_occupation;
            for(int to = 0; to < 64; to++)
            {
                if(single & (1ULL << to))
                {
                    if((1ULL << to) & RANK_1)
                        add_promotion_moves((enum Position)from, (enum Position)to);
                    else
                        add_move(move_list, &move_count, (enum Position)from, (enum Position)to, MOVE_NORMAL);
                }
            }

            // double push from rank 7 only
            if(from_bit & RANK_7)
            {
                uint64_t double_push = ((single & RANK_6) >> 8) & ~total_occupation;
                add_moves_from_bitboard((enum Position)from, double_push);
            }

            // captures
            uint64_t capture_left = ((from_bit & ~FILE_A) >> 9) & white_occupation;
            uint64_t capture_right = ((from_bit & ~FILE_H) >> 7) & white_occupation;
            uint64_t captures = capture_left | capture_right;
            
            for(int to = 0; to < 64; to++)
            {
                if(captures & (1ULL << to))
                {
                    if((1ULL << to) & RANK_1)
                        add_promotion_moves((enum Position)from, (enum Position)to);
                    else
                        add_move(move_list, &move_count,
                                 (enum Position)from,
                                 (enum Position)to,
                                 MOVE_NORMAL);
                }
            }

            // en passant
            if(en_passant_square != -1)
            {
                uint64_t ep = 1ULL << en_passant_square;
                uint64_t ep_left = ((from_bit & ~FILE_A) >> 9) & ep;
                uint64_t ep_right = ((from_bit & ~FILE_H) >> 7) & ep;
                uint64_t ep_moves = ep_left | ep_right;
            
                for(int to = 0; to < 64; to++)
                {
                    if(ep_moves & (1ULL << to))
                        add_move(move_list, &move_count, (enum Position)from, (enum Position)to, MOVE_EN_PASSANT);
                }
            }
        }
    }
}

void generate_black_knight_moves()
{
    for(int from = 0; from < 64; from++)
    {
        if(black_knights & (1ULL << from))
        {
            uint64_t moves = knight_attacks(1ULL << from) & ~black_occupation;
            add_moves_from_bitboard((enum Position)from, moves);
        }
    }
}

void generate_black_bishop_moves()
{
    for(int from = 0; from < 64; from++)
    {
        if(black_bishops & (1ULL << from))
        {
            uint64_t moves = bishop_attacks(from) & ~black_occupation;
            add_moves_from_bitboard((enum Position)from, moves);
        }
    }
}

void generate_black_rook_moves()
{
    for(int from = 0; from < 64; from++)
    {
        if(black_rooks & (1ULL << from))
        {
            uint64_t moves = rook_attacks(from) & ~black_occupation;
            add_moves_from_bitboard((enum Position)from, moves);
        }
    }
}

void generate_black_queen_moves()
{
    for(int from = 0; from < 64; from++)
    {
        if(black_queens & (1ULL << from))
        {
            uint64_t moves = queen_attacks(from) & ~black_occupation;
            add_moves_from_bitboard((enum Position)from, moves);
        }
    }
}

void generate_black_king_moves()
{
    for(int from = 0; from < 64; from++)
    {
        if(black_king & (1ULL << from))
        {
            uint64_t moves = king_attacks(1ULL << from) & ~black_occupation;
            add_moves_from_bitboard((enum Position)from, moves);
        }
    }
}

void generate_black_moves()
{
    move_count = 0;

    generate_black_pawn_moves();
    generate_black_knight_moves();
    generate_black_bishop_moves();
    generate_black_rook_moves();
    generate_black_queen_moves();
    generate_black_king_moves();
    generate_castling_moves();
}

void clear_move_list()
{
    move_count = 0;
}

enum Side {
    WHITE,
    BLACK
};

enum Side side_to_move = WHITE;

void generate_moves()
{
    if(side_to_move == WHITE)
        generate_white_moves();
    else
        generate_black_moves();
}

void switch_side()
{
    if(side_to_move == WHITE)
        side_to_move = BLACK;
    else
        side_to_move = WHITE;
}

void make_move(Move move)
{
    en_passant_square = -1;

    uint64_t from_mask = 1ULL << move.from;
    uint64_t to_mask   = 1ULL << move.to;

    if(move.from == e1) {
        white_can_castle_kingside = 0;
        white_can_castle_queenside = 0;
    }
    if(move.from == e8) {
        black_can_castle_kingside = 0;
        black_can_castle_queenside = 0;
    }
    
    if(move.from == h1 || move.to == h1) white_can_castle_kingside = 0;
    if(move.from == a1 || move.to == a1) white_can_castle_queenside = 0;
    if(move.from == h8 || move.to == h8) black_can_castle_kingside = 0;
    if(move.from == a8 || move.to == a8) black_can_castle_queenside = 0;

    // En passant capture removes pawn behind target square
    if(move.flags == MOVE_EN_PASSANT)
    {
        if(side_to_move == WHITE)
            black_pawns &= ~(1ULL << (move.to - 8));
        else
            white_pawns &= ~(1ULL << (move.to + 8));
    }
    else
    {
        // Normal capture: remove any enemy piece on destination
        if(side_to_move == WHITE)
        {
            black_pawns   &= ~to_mask;
            black_knights &= ~to_mask;
            black_bishops &= ~to_mask;
            black_rooks   &= ~to_mask;
            black_queens  &= ~to_mask;
            black_king    &= ~to_mask;
        }
        else
        {
            white_pawns   &= ~to_mask;
            white_knights &= ~to_mask;
            white_bishops &= ~to_mask;
            white_rooks   &= ~to_mask;
            white_queens  &= ~to_mask;
            white_king    &= ~to_mask;
        }
    }

    // Move white pieces
    if(white_pawns & from_mask)
    {
        white_pawns &= ~from_mask;

        if(move.to - move.from == 16)
            en_passant_square = move.from + 8;

        if(move.flags == MOVE_PROMOTE_QUEEN)
            white_queens |= to_mask;
        else if(move.flags == MOVE_PROMOTE_ROOK)
            white_rooks |= to_mask;
        else if(move.flags == MOVE_PROMOTE_BISHOP)
            white_bishops |= to_mask;
        else if(move.flags == MOVE_PROMOTE_KNIGHT)
            white_knights |= to_mask;
        else
            white_pawns |= to_mask;
    }
    else if(white_knights & from_mask) {
        white_knights &= ~from_mask;
        white_knights |= to_mask;
    }
    else if(white_bishops & from_mask) {
        white_bishops &= ~from_mask;
        white_bishops |= to_mask;
    }
    else if(white_rooks & from_mask) {
        white_rooks &= ~from_mask;
        white_rooks |= to_mask;
    }
    else if(white_queens & from_mask) {
        white_queens &= ~from_mask;
        white_queens |= to_mask;
    }
    else if(white_king & from_mask) {
        white_king &= ~from_mask;
        white_king |= to_mask;
    }

    // Move black pieces
    else if(black_pawns & from_mask)
    {
        black_pawns &= ~from_mask;

        if(move.from - move.to == 16)
            en_passant_square = move.from - 8;

        if(move.flags == MOVE_PROMOTE_QUEEN)
            black_queens |= to_mask;
        else if(move.flags == MOVE_PROMOTE_ROOK)
            black_rooks |= to_mask;
        else if(move.flags == MOVE_PROMOTE_BISHOP)
            black_bishops |= to_mask;
        else if(move.flags == MOVE_PROMOTE_KNIGHT)
            black_knights |= to_mask;
        else
            black_pawns |= to_mask;
    }
    else if(black_knights & from_mask) {
        black_knights &= ~from_mask;
        black_knights |= to_mask;
    }
    else if(black_bishops & from_mask) {
        black_bishops &= ~from_mask;
        black_bishops |= to_mask;
    }
    else if(black_rooks & from_mask) {
        black_rooks &= ~from_mask;
        black_rooks |= to_mask;
    }
    else if(black_queens & from_mask) {
        black_queens &= ~from_mask;
        black_queens |= to_mask;
    }
    else if(black_king & from_mask) {
        black_king &= ~from_mask;
        black_king |= to_mask;
    }

    if(move.flags == MOVE_CASTLE_KINGSIDE)
    {
        if(side_to_move == WHITE)
        {
            white_rooks &= ~(1ULL << h1);
            white_rooks |=  (1ULL << f1);
        }
        else
        {
            black_rooks &= ~(1ULL << h8);
            black_rooks |=  (1ULL << f8);
        }
    }
    else if(move.flags == MOVE_CASTLE_QUEENSIDE)
    {
        if(side_to_move == WHITE)
        {
            white_rooks &= ~(1ULL << a1);
            white_rooks |=  (1ULL << d1);
        }
        else
        {
            black_rooks &= ~(1ULL << a8);
            black_rooks |=  (1ULL << d8);
        }
    }

    occupation();
    switch_side();
}

uint64_t all_white_attacks()
{
    return white_pawn_attacks()
         | knight_attacks(white_knights)
         | white_bishop_attacks()
         | white_rook_attacks()
         | white_queen_attacks()
         | white_king_attacks();
}

uint64_t all_black_attacks()
{
    return black_pawn_attacks()
         | knight_attacks(black_knights)
         | black_bishop_attacks()
         | black_rook_attacks()
         | black_queen_attacks()
         | black_king_attacks();
}

int white_king_in_check()
{
    return (white_king & all_black_attacks()) != 0;
}

int black_king_in_check()
{
    return (black_king & all_white_attacks()) != 0;
}

int king_in_check(enum Side side)
{
    if(side == WHITE)
        return white_king_in_check();
    else
        return black_king_in_check();
}

typedef struct {
    uint64_t white_pawns, white_knights, white_bishops, white_rooks, white_queens, white_king;
    uint64_t black_pawns, black_knights, black_bishops, black_rooks, black_queens, black_king;
    enum Side side_to_move;
    int en_passant_square;
    int white_can_castle_kingside;
    int white_can_castle_queenside;
    int black_can_castle_kingside;
    int black_can_castle_queenside;
} BoardState;

BoardState save_board()
{
    return (BoardState){
        white_pawns, white_knights, white_bishops, white_rooks, white_queens, white_king,
        black_pawns, black_knights, black_bishops, black_rooks, black_queens, black_king,
        side_to_move, en_passant_square, white_can_castle_kingside, white_can_castle_queenside,
        black_can_castle_kingside, black_can_castle_queenside
    };
}

void restore_board(BoardState s)
{
    white_pawns = s.white_pawns;
    white_knights = s.white_knights;
    white_bishops = s.white_bishops;
    white_rooks = s.white_rooks;
    white_queens = s.white_queens;
    white_king = s.white_king;

    black_pawns = s.black_pawns;
    black_knights = s.black_knights;
    black_bishops = s.black_bishops;
    black_rooks = s.black_rooks;
    black_queens = s.black_queens;
    black_king = s.black_king;

    side_to_move = s.side_to_move;
    en_passant_square = s.en_passant_square;
    white_can_castle_kingside = s.white_can_castle_kingside;
    white_can_castle_queenside = s.white_can_castle_queenside;
    black_can_castle_kingside = s.black_can_castle_kingside;
    black_can_castle_queenside = s.black_can_castle_queenside;

    occupation();
}

void clear_board()
{
    white_pawns = white_knights = white_bishops = white_rooks = white_queens = white_king = 0;
    black_pawns = black_knights = black_bishops = black_rooks = black_queens = black_king = 0;

    en_passant_square = -1;

    white_can_castle_kingside = 0;
    white_can_castle_queenside = 0;
    black_can_castle_kingside = 0;
    black_can_castle_queenside = 0;

    side_to_move = WHITE;

    occupation();
}

int move_is_legal(Move move)
{
    enum Side moving_side = side_to_move;
    BoardState saved = save_board();
    make_move(move);
    int legal = !king_in_check(moving_side);
    restore_board(saved);

    return legal;
}

void generate_legal_moves()
{
    Move pseudo_moves[MAX_MOVES];
    int pseudo_count;

    generate_moves();
    pseudo_count = move_count;

    for(int i = 0; i < pseudo_count; i++)
    {
        pseudo_moves[i] = move_list[i];
    }

    move_count = 0;

    for(int i = 0; i < pseudo_count; i++)
    {
        if(move_is_legal(pseudo_moves[i]))
        {
            add_move(move_list, &move_count, pseudo_moves[i].from, pseudo_moves[i].to, pseudo_moves[i].flags);
        }
    }
}

void generate_castling_moves()
{
    if(side_to_move == WHITE)
    {
        if(white_can_castle_kingside &&
           (white_rooks & (1ULL << h1)) &&
           !(total_occupation & ((1ULL << f1) | (1ULL << g1))) &&
           !king_in_check(WHITE) &&
           !(all_black_attacks() & ((1ULL << f1) | (1ULL << g1))))
        {
            add_move(move_list, &move_count, e1, g1, MOVE_CASTLE_KINGSIDE);
        }

        if(white_can_castle_queenside &&
           (white_rooks & (1ULL << a1)) &&
           !(total_occupation & ((1ULL << b1) | (1ULL << c1) | (1ULL << d1))) &&
           !king_in_check(WHITE) &&
           !(all_black_attacks() & ((1ULL << c1) | (1ULL << d1))))
        {
            add_move(move_list, &move_count, e1, c1, MOVE_CASTLE_QUEENSIDE);
        }
    }
    else
    {
        if(black_can_castle_kingside &&
           (black_rooks & (1ULL << h8)) &&
           !(total_occupation & ((1ULL << f8) | (1ULL << g8))) &&
           !king_in_check(BLACK) &&
           !(all_white_attacks() & ((1ULL << f8) | (1ULL << g8))))
        {
            add_move(move_list, &move_count, e8, g8, MOVE_CASTLE_KINGSIDE);
        }

        if(black_can_castle_queenside &&
           (black_rooks & (1ULL << a8)) &&
           !(total_occupation & ((1ULL << b8) | (1ULL << c8) | (1ULL << d8))) &&
           !king_in_check(BLACK) &&
           !(all_white_attacks() & ((1ULL << c8) | (1ULL << d8))))
        {
            add_move(move_list, &move_count, e8, c8, MOVE_CASTLE_QUEENSIDE);
        }
    }
}

void load_fen(const char *fen)
{
    clear_board();

    int rank = 7;
    int file = 0;
    int i = 0;

    // Piece placement
    while(fen[i] && fen[i] != ' ')
    {
        char c = fen[i];

        if(c == '/')
        {
            rank--;
            file = 0;
        }
        else if(isdigit(c))
        {
            file += c - '0';
        }
        else
        {
            int square = rank * 8 + file;
            uint64_t bit = 1ULL << square;

            switch(c)
            {
                case 'P': white_pawns   |= bit; break;
                case 'N': white_knights |= bit; break;
                case 'B': white_bishops |= bit; break;
                case 'R': white_rooks   |= bit; break;
                case 'Q': white_queens  |= bit; break;
                case 'K': white_king    |= bit; break;

                case 'p': black_pawns   |= bit; break;
                case 'n': black_knights |= bit; break;
                case 'b': black_bishops |= bit; break;
                case 'r': black_rooks   |= bit; break;
                case 'q': black_queens  |= bit; break;
                case 'k': black_king    |= bit; break;
            }

            file++;
        }

        i++;
    }

    // Skip space
    if(fen[i] == ' ') i++;

    // Side to move
    if(fen[i] == 'w')
        side_to_move = WHITE;
    else if(fen[i] == 'b')
        side_to_move = BLACK;

    while(fen[i] && fen[i] != ' ') i++;
    if(fen[i] == ' ') i++;

    // Castling rights
    while(fen[i] && fen[i] != ' ')
    {
        switch(fen[i])
        {
            case 'K': white_can_castle_kingside = 1; break;
            case 'Q': white_can_castle_queenside = 1; break;
            case 'k': black_can_castle_kingside = 1; break;
            case 'q': black_can_castle_queenside = 1; break;
        }

        i++;
    }

    if(fen[i] == ' ') i++;

    // En passant square
    if(fen[i] != '-')
    {
        int ep_file = fen[i] - 'a';
        int ep_rank = fen[i + 1] - '1';

        en_passant_square = ep_rank * 8 + ep_file;
    }
    else
    {
        en_passant_square = -1;
    }

    occupation();
}

uint64_t perft(int depth)
{
    if(depth == 0)
        return 1;

    generate_legal_moves();

    uint64_t nodes = 0;

    int count = move_count;

    Move moves[MAX_MOVES];

    for(int i = 0; i < count; i++)
        moves[i] = move_list[i];

    for(int i = 0; i < count; i++)
    {
        BoardState saved = save_board();

        make_move(moves[i]);

        nodes += perft(depth - 1);

        restore_board(saved);
    }

    return nodes;
}

void perft_divide(int depth)
{
    generate_legal_moves();

    Move moves[MAX_MOVES];
    int count = move_count;

    for(int i = 0; i < count; i++)
        moves[i] = move_list[i];

    uint64_t total = 0;

    for(int i = 0; i < count; i++)
    {
        BoardState saved = save_board();

        print_move(moves[i]);

        make_move(moves[i]);

        uint64_t nodes = perft(depth - 1);

        restore_board(saved);

        printf("Nodes: %" PRIu64 "\n\n", nodes);

        total += nodes;
    }

    printf("Total: %" PRIu64 "\n", total);
}

// Evaluation and Search Processes

typedef struct {
	int pawn_value;
	int knight_value;
	int bishop_value;
	int rook_value;
	int queen_value;
	//int king value

	int doubled_pawn_penalty;
	int rook_open_file_bonus;
	int passed_pawn_weight;
	int bad_bishop_pawn_penalty;
	int castle_bonus;
	int center_pawn_bonus;
	int knight_development_bonus;
	int bishop_development_bonus;
	
} EvalParams;

EvalParams default_params = {100, 300, 330, 500, 900, 10, 10, 25, 3, 25, 10, 10, 10};

int count_bits(uint64_t bb)
{
    return __builtin_popcountll(bb);
}

int white_has_castled()
{
    return (white_king & (1ULL << g1)) ||
           (white_king & (1ULL << c1));
}

int black_has_castled()
{
    return (black_king & (1ULL << g8)) ||
           (black_king & (1ULL << c8));
}

int center_pawn_score(EvalParams *params)
{
    int score = 0;

    uint64_t center = (1ULL << d4) | (1ULL << e4) |
                      (1ULL << d5) | (1ULL << e5);

    score += count_bits(white_pawns & center) * params->center_pawn_bonus;
    score -= count_bits(black_pawns & center) * params->center_pawn_bonus;

    return score;
}

int development_score(EvalParams *params)
{
    int score = 0;

    uint64_t white_knight_start = (1ULL << b1) | (1ULL << g1);
    uint64_t black_knight_start = (1ULL << b8) | (1ULL << g8);

    uint64_t white_bishop_start = (1ULL << c1) | (1ULL << f1);
    uint64_t black_bishop_start = (1ULL << c8) | (1ULL << f8);

    int white_developed_knights = 2 - count_bits(white_knights & white_knight_start);
    int black_developed_knights = 2 - count_bits(black_knights & black_knight_start);

    int white_developed_bishops = 2 - count_bits(white_bishops & white_bishop_start);
    int black_developed_bishops = 2 - count_bits(black_bishops & black_bishop_start);

    score += white_developed_knights * params->knight_development_bonus;
    score -= black_developed_knights * params->knight_development_bonus;

    score += white_developed_bishops * params->bishop_development_bonus;
    score -= black_developed_bishops * params->bishop_development_bonus;

    return score;
}

int castle_score(EvalParams *params)
{
    int score = 0;

    if(white_has_castled())
        score += params->castle_bonus;

    if(black_has_castled())
        score -= params->castle_bonus;

    return score;
}

int doubled_pawn_score(EvalParams *params)
{
    int score = 0;

    const uint64_t files[8] = {
        FILE_A, FILE_B, FILE_C, FILE_D,
        FILE_E, FILE_F, FILE_G, FILE_H
    };

    for(int i = 0; i < 8; i++)
    {
        int white_count = count_bits(white_pawns & files[i]);
        int black_count = count_bits(black_pawns & files[i]);

        if(white_count > 1)
            score -= (white_count - 1) * params->doubled_pawn_penalty;

        if(black_count > 1)
            score += (black_count - 1) * params->doubled_pawn_penalty;
    }

    return score;
}

int rook_open_file_score(EvalParams *params)
{
    int score = 0;

    const uint64_t files[8] = {
        FILE_A, FILE_B, FILE_C, FILE_D,
        FILE_E, FILE_F, FILE_G, FILE_H
    };

    for(int i = 0; i < 8; i++)
    {
        int file_has_white_pawn = (white_pawns & files[i]) != 0;
        int file_has_black_pawn = (black_pawns & files[i]) != 0;
        int file_is_open = !file_has_white_pawn && !file_has_black_pawn;

        if(file_is_open)
        {
            if(white_rooks & files[i])
                score += params->rook_open_file_bonus;

            if(black_rooks & files[i])
                score -= params->rook_open_file_bonus;
        }
    }

    return score;
}

int bad_bishop_score(EvalParams *params)
{
    int score = 0;

    if(white_bishops & DARK_SQUARES)
        score -= count_bits(white_pawns & DARK_SQUARES) * params->bad_bishop_pawn_penalty;

    if(white_bishops & LIGHT_SQUARES)
        score -= count_bits(white_pawns & LIGHT_SQUARES) * params->bad_bishop_pawn_penalty;

    if(black_bishops & DARK_SQUARES)
        score += count_bits(black_pawns & DARK_SQUARES) * params->bad_bishop_pawn_penalty;

    if(black_bishops & LIGHT_SQUARES)
        score += count_bits(black_pawns & LIGHT_SQUARES) * params->bad_bishop_pawn_penalty;

    return score;
}

uint64_t adjacent_files_mask(int file)
{
    uint64_t mask = 0;

    const uint64_t files[8] = {
        FILE_A, FILE_B, FILE_C, FILE_D,
        FILE_E, FILE_F, FILE_G, FILE_H
    };

    if(file > 0)
        mask |= files[file - 1];

    mask |= files[file];

    if(file < 7)
        mask |= files[file + 1];

    return mask;
}

int passed_pawn_score(EvalParams *params)
{
    int score = 0;

    for(int square = 0; square < 64; square++)
    {
        uint64_t pawn = 1ULL << square;

        int rank = square / 8;
        int file = square % 8;

        uint64_t nearby_files = adjacent_files_mask(file);

        if(white_pawns & pawn)
        {
            uint64_t squares_ahead = 0;

            for(int r = rank + 1; r < 8; r++)
                squares_ahead |= RANK_1 << (8 * r);

            if((black_pawns & nearby_files & squares_ahead) == 0)
                score += params->passed_pawn_weight * (rank + 1);
        }

        if(black_pawns & pawn)
        {
            uint64_t squares_ahead = 0;

            for(int r = rank - 1; r >= 0; r--)
                squares_ahead |= RANK_1 << (8 * r);

            if((white_pawns & nearby_files & squares_ahead) == 0)
                score -= params->passed_pawn_weight * (8 - rank);
        }
    }

    return score;
}

int evaluate_position(EvalParams *params)
{
    int white_score = 0;
    int black_score = 0;

    white_score += count_bits(white_pawns)   * params->pawn_value;
    white_score += count_bits(white_knights) * params->knight_value;
    white_score += count_bits(white_bishops) * params->bishop_value;
    white_score += count_bits(white_rooks)   * params->rook_value;
    white_score += count_bits(white_queens)  * params->queen_value;

    black_score += count_bits(black_pawns)   * params->pawn_value;
    black_score += count_bits(black_knights) * params->knight_value;
    black_score += count_bits(black_bishops) * params->bishop_value;
    black_score += count_bits(black_rooks)   * params->rook_value;
    black_score += count_bits(black_queens)  * params->queen_value;

    int score = white_score - black_score;
    
    score += doubled_pawn_score(params);
    score += rook_open_file_score(params);
    score += bad_bishop_score(params);
    score += passed_pawn_score(params);
    score += castle_score(params);
    score += center_pawn_score(params);
    score += development_score(params);
    
    return score;
}

int evaluate(EvalParams *params)
{
    int score = evaluate_position(params);

    if(side_to_move == WHITE)
        return score;
    else
        return -score;
}

int move_is_capture(Move move)
{
    uint64_t to_mask = 1ULL << move.to;

    if(side_to_move == WHITE)
        return (black_occupation & to_mask) != 0 || move.flags == MOVE_EN_PASSANT;
    else
        return (white_occupation & to_mask) != 0 || move.flags == MOVE_EN_PASSANT;
}

void order_moves(Move moves[], int count)
{
    for(int i = 0; i < count - 1; i++)
    {
        for(int j = i + 1; j < count; j++)
        {
            int a = move_is_capture(moves[i]);
            int b = move_is_capture(moves[j]);

            if(b > a)
            {
                Move temp = moves[i];
                moves[i] = moves[j];
                moves[j] = temp;
            }
        }
    }
}

int negamax(int depth, int alpha, int beta, EvalParams *params, int ply)
{
    if(depth == 0)
        return evaluate(params);

    generate_legal_moves();
    
    if(move_count == 0)
    {
        if(king_in_check(side_to_move))
            return -100000 + ply; // checkmate
    
        return 0; // stalemate
    }

    int best_score = -1000000;

    Move moves[MAX_MOVES];
    int count = move_count;

    for(int i = 0; i < count; i++)
        moves[i] = move_list[i];

    order_moves(moves, count);

    for(int i = 0; i < count; i++)
    {
        BoardState saved = save_board();

        make_move(moves[i]);

        int score = -negamax(depth - 1, -beta, -alpha, params, ply + 1);

        restore_board(saved);

        if(score > best_score)
            best_score = score;

        if(score > alpha)
            alpha = score;

        if(alpha >= beta)
            break;
    }

    return best_score;
}

Move find_best_move(int depth, EvalParams *params)
{
    generate_legal_moves();

	if(move_count == 0)
	{
		Move null_move = {0};
		
	    return null_move;
	}
	
    Move best_move = move_list[0];
    int best_score = -1000000;

    Move moves[MAX_MOVES];
    int count = move_count;

    for(int i = 0; i < count; i++)
        moves[i] = move_list[i];

    order_moves(moves, count);

    for(int i = 0; i < count; i++)
    {
        BoardState saved = save_board();
		int is_capture = move_is_capture(moves[i]);
        make_move(moves[i]);

        int score = -negamax( depth - 1, -1000000, 1000000, params, 1);

        restore_board(saved);

        if(is_capture)
            score += 1;

        if(score > best_score)
        {
            best_score = score;
            best_move = moves[i];
        }
        else if(score == best_score)
        {
            if(rand() % 2 == 0)
                best_move = moves[i];
        }
    }

    return best_move;
}

// Engine v. Engine Games

int game_is_over()
{
    generate_legal_moves();
    return move_count == 0;
}

uint64_t board_key()
{
    uint64_t key = 0;

    key ^= white_pawns;
    key ^= white_knights << 1;
    key ^= white_bishops << 2;
    key ^= white_rooks << 3;
    key ^= white_queens << 4;
    key ^= white_king << 5;

    key ^= black_pawns << 6;
    key ^= black_knights << 7;
    key ^= black_bishops << 8;
    key ^= black_rooks << 9;
    key ^= black_queens << 10;
    key ^= black_king << 11;

    key ^= (uint64_t)side_to_move << 63;

    key ^= (uint64_t)white_can_castle_kingside << 55;
    key ^= (uint64_t)white_can_castle_queenside << 56;
    key ^= (uint64_t)black_can_castle_kingside << 57;
    key ^= (uint64_t)black_can_castle_queenside << 58;

    if(en_passant_square != -1)
        key ^= (uint64_t)en_passant_square << 48;

    return key;
}

// 1 is white wins, 0 is stalemate, -1 is black wins
int play_engine_game(EvalParams *white_params,
                     EvalParams *black_params,
                     int depth,
                     int max_plies,
                     int verbose)
{
    uint64_t history[MAX_GAME_PLIES];
    int history_count = 0;

    if(max_plies > MAX_GAME_PLIES)
        max_plies = MAX_GAME_PLIES;

    for(int ply = 0; ply < max_plies; ply++)
    {
        uint64_t key = board_key();

        int repetitions = 0;

        for(int i = 0; i < history_count; i++)
        {
            if(history[i] == key)
                repetitions++;
        }

        if(repetitions >= 2)
        {
//            printf("Draw by threefold repetition\n");
            return 0;
        }

        history[history_count++] = key;

        generate_legal_moves();

        if(move_count == 0)
        {
            if(king_in_check(side_to_move))
            {
                if(side_to_move == WHITE)
                {
//                    printf("Black wins by checkmate\n");
                    return -1;
                }
                else
                {
//                    printf("White wins by checkmate\n");
                    return 1;
                }
            }

//            printf("Draw by stalemate\n");
            return 0;
        }

		if(verbose)
		{
			printf("\nPly %d\n", ply + 1);
			print_bitboard(total_occupation);
		}
        

        Move best;

        if(side_to_move == WHITE)
            best = find_best_move(depth, white_params);
        else
            best = find_best_move(depth, black_params);

		if(verbose)
		{
			printf("Move played: ");
	    	print_move(best);	
		}

        make_move(best);
    }

//    printf("Draw by max ply limit\n");
    return 0;
}

typedef struct{
	EvalParams params;

	int wins;
	int losses;
	int draws;
	int score;
} Candidate;

void reset_candidate_score(Candidate *c)
{
    c->wins = 0;
    c->losses = 0;
    c->draws = 0;
    c->score = 0;
}

void play_match(Candidate *a,
                Candidate *b,
                int depth,
                int games_per_fen)
{
    reset_candidate_score(a);
    reset_candidate_score(b);

    for(int fen_index = 0; fen_index < training_position_count; fen_index++)
    {

//	printf("\n%s\n", training_positions[fen_index].name);
    
        for(int game = 0; game < games_per_fen; game++)
        {
            int result;

            // A as White
            load_fen(training_positions[fen_index].fen);

            result = play_engine_game(
                &a->params,
                &b->params,
                depth,
                250,
                0
            );

            if(result == 1) {
                a->wins++;
                b->losses++;
            }
            else if(result == -1) {
                b->wins++;
                a->losses++;
            }
            else {
                a->draws++;
                b->draws++;
            }

            // B as White
            load_fen(training_positions[fen_index].fen);

            result = play_engine_game(
                &b->params,
                &a->params,
                depth,
                250,
                0
            );

            if(result == 1) {
                b->wins++;
                a->losses++;
            }
            else if(result == -1) {
                a->wins++;
                b->losses++;
            }
            else {
                a->draws++;
                b->draws++;
            }
        }
    }

    a->score = (a->wins * 10) + a->draws;
    b->score = (b->wins * 10) + b->draws;
}

EvalParams make_balanced_params()
{
    return (EvalParams){100, 320, 330, 500, 900, 10, 10, 25, 3, 5, 10, 10, 10};
}


int mutate_int(int value, int amount)
{
    int change = (rand() % (2 * amount + 1)) - amount;
    int new_value = value + change;

    if(new_value < 0)
        new_value = 0;

    return new_value;
}

int clamp_int(int value, int min, int max)
{
    if(value < min)
        return min;

    if(value > max)
        return max;

    return value;
}

EvalParams mutate_params(EvalParams parent)
{
    EvalParams child = parent;

    child.pawn_value =
        clamp_int(mutate_int(parent.pawn_value, 3), 70, 130);

    child.knight_value =
        clamp_int(mutate_int(parent.knight_value, 5), 250, 380);

    child.bishop_value =
        clamp_int(mutate_int(parent.bishop_value, 5), 270, 400);

    child.rook_value =
        clamp_int(mutate_int(parent.rook_value, 8), 430, 580);

    child.queen_value =
        clamp_int(mutate_int(parent.queen_value, 12), 750, 1050);

    child.doubled_pawn_penalty =
        clamp_int(mutate_int(parent.doubled_pawn_penalty, 2), 1, 25);

    child.rook_open_file_bonus =
        clamp_int(mutate_int(parent.rook_open_file_bonus, 2), 1, 25);

    child.passed_pawn_weight =
        clamp_int(mutate_int(parent.passed_pawn_weight, 2), 1, 25);

    child.bad_bishop_pawn_penalty =
        clamp_int(mutate_int(parent.bad_bishop_pawn_penalty, 2), 1, 25);

    child.castle_bonus =
        clamp_int(mutate_int(parent.castle_bonus, 2), 1, 50);
        
    child.center_pawn_bonus =
        clamp_int(mutate_int(parent.center_pawn_bonus, 2), 1, 25);
    
    child.knight_development_bonus =
        clamp_int(mutate_int(parent.knight_development_bonus, 2), 1, 25);
    
    child.bishop_development_bonus =
        clamp_int(mutate_int(parent.bishop_development_bonus, 2), 1, 25);

    return child;
}

void log_candidate(FILE *log,
                   int generation, char candidate_name,
                   Candidate *c)
{
    fprintf(log,
        "%d,%c,"
        "%d,%d,%d,%d,%d,"
        "%d,%d,%d,%d,%d,"
        "%d,%d,%d,"
        "%d,%d,%d,%d\n",

        generation, candidate_name,

        c->params.pawn_value,
        c->params.knight_value,
        c->params.bishop_value,
        c->params.rook_value,
        c->params.queen_value,

        c->params.doubled_pawn_penalty,
        c->params.rook_open_file_bonus,
        c->params.passed_pawn_weight,
        c->params.bad_bishop_pawn_penalty,
        c->params.castle_bonus,
        c->params.center_pawn_bonus,
        c->params.knight_development_bonus,
        c->params.bishop_development_bonus,

        c->wins,
        c->draws,
        c->losses,
        c->score
    );
}

void save_champion(EvalParams *p)
{
    FILE *f = fopen("fissure_champion.txt", "w");

    if(f == NULL)
    {
        printf("Failed to save champion\n");
        return;
    }

    fprintf(f,
        "%d %d %d %d %d %d %d %d %d %d %d %d %d\n",

        p->pawn_value,
        p->knight_value,
        p->bishop_value,
        p->rook_value,
        p->queen_value,

        p->doubled_pawn_penalty,
        p->rook_open_file_bonus,
        p->passed_pawn_weight,
        p->bad_bishop_pawn_penalty,
        p->castle_bonus,
        p->center_pawn_bonus,
        p->knight_development_bonus,
        p->bishop_development_bonus
    );

    fclose(f);
}

void benchmark_champion(EvalParams *champion_params,
                        EvalParams *benchmark_params,
                        int depth,
                        int games_per_fen)
{
    Candidate champion = {0};
    Candidate benchmark = {0};

    champion.params = *champion_params;
    benchmark.params = *benchmark_params;

    play_match(&champion, &benchmark, depth, games_per_fen);

    printf("\nBENCHMARK TEST\n");
    printf("Champion: %d W / %d D / %d L | score %d\n",
           champion.wins, champion.draws, champion.losses, champion.score);

    printf("Baseline: %d W / %d D / %d L | score %d\n\n",
           benchmark.wins, benchmark.draws, benchmark.losses, benchmark.score);
}

EvalParams run_evolution(FILE *log,
                         EvalParams starting_params,
                         int generations,
                         int depth,
                         int games_per_match)
{
    EvalParams champion_params = starting_params;

    for(int generation = 0; generation < generations; generation++)
    {
        Candidate best = {0};
        best.params = champion_params;

        int best_is_challenger = 0;

//        printf("\n=== Generation %d ===\n", generation);

        for(int i = 0; i < CHALLENGERS_PER_GENERATION; i++)
        {
            Candidate champion = {0};
            Candidate challenger = {0};

            champion.params = champion_params;
            challenger.params = mutate_params(champion_params);

            play_match(&champion, &challenger, depth, games_per_match);

            log_candidate(log, generation, 'A', &champion);
            log_candidate(log, generation, 'B' + i, &challenger);

//            printf("Challenger %d\n", i + 1);
//            printf("Champion:   %d W / %d D / %d L | score %d\n", champion.wins, champion.draws, champion.losses, champion.score);
//            printf("Challenger: %d W / %d D / %d L | score %d\n", challenger.wins, challenger.draws, challenger.losses, challenger.score);

            if(challenger.score > champion.score &&
               challenger.score > best.score)
            {
                best = challenger;
                best_is_challenger = 1;
            }

            fflush(log);
        }

        if(best_is_challenger)
        {
//            printf("Best challenger survived\n\n");
            champion_params = best.params;
        }
        else
        {
//            printf("Champion survived all challengers\n\n");
        }

        save_champion(&champion_params);

        if((generation + 1) % 10 == 0)
        {
            EvalParams baseline = make_balanced_params();

            benchmark_champion(
                &champion_params,
                &baseline,
                depth,
                5
            );
        }
    }

    return champion_params;
}

int load_champion(EvalParams *p)
{
    FILE *f = fopen("fissure_champion.txt", "r");

    if(f == NULL)
        return 0;

    int result = fscanf(
        f,
        "%d %d %d %d %d %d %d %d %d %d %d %d %d",

        &p->pawn_value,
        &p->knight_value,
        &p->bishop_value,
        &p->rook_value,
        &p->queen_value,

        &p->doubled_pawn_penalty,
        &p->rook_open_file_bonus,
        &p->passed_pawn_weight,
        &p->bad_bishop_pawn_penalty,
        &p->castle_bonus,
        &p->center_pawn_bonus,
        &p->knight_development_bonus,
        &p->bishop_development_bonus
    );


    fclose(f);

    return result == 13;
}

void move_to_uci(Move move, char out[6])
{
    out[0] = 'a' + (move.from % 8);
    out[1] = '1' + (move.from / 8);
    out[2] = 'a' + (move.to % 8);
    out[3] = '1' + (move.to / 8);
    out[4] = '\0';

    if(move.flags == MOVE_PROMOTE_QUEEN)
    {
        out[4] = 'q';
        out[5] = '\0';
    }
    else if(move.flags == MOVE_PROMOTE_ROOK)
    {
        out[4] = 'r';
        out[5] = '\0';
    }
    else if(move.flags == MOVE_PROMOTE_BISHOP)
    {
        out[4] = 'b';
        out[5] = '\0';
    }
    else if(move.flags == MOVE_PROMOTE_KNIGHT)
    {
        out[4] = 'n';
        out[5] = '\0';
    }
}

int same_uci_move(Move move, const char *uci)
{
    char move_text[6];
    move_to_uci(move, move_text);

    return strcmp(move_text, uci) == 0;
}

int make_uci_move(const char *uci)
{
    generate_legal_moves();

    Move moves[MAX_MOVES];
    int count = move_count;

    for(int i = 0; i < count; i++)
        moves[i] = move_list[i];

    for(int i = 0; i < count; i++)
    {
        if(same_uci_move(moves[i], uci))
        {
            make_move(moves[i]);
            return 1;
        }
    }

    return 0;
}

void uci_position(char *line)
{
    char copy[8192];
    strncpy(copy, line, sizeof(copy));
    copy[sizeof(copy) - 1] = '\0';

    char *saveptr;
    char *token = strtok_r(copy, " \r\n", &saveptr);

    // token should be "position"
    if(token == NULL || strcmp(token, "position") != 0)
        return;

    token = strtok_r(NULL, " \r\n", &saveptr);

    if(token == NULL)
        return;

    if(strcmp(token, "startpos") == 0)
    {
        load_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

        token = strtok_r(NULL, " \r\n", &saveptr);
    }
    else if(strcmp(token, "fen") == 0)
    {
        char fen[256] = "";
        char *fields[6];

        for(int i = 0; i < 6; i++)
        {
            fields[i] = strtok_r(NULL, " \r\n", &saveptr);

            if(fields[i] == NULL)
                return;
        }

        snprintf(fen, sizeof(fen),
                 "%s %s %s %s %s %s",
                 fields[0],
                 fields[1],
                 fields[2],
                 fields[3],
                 fields[4],
                 fields[5]);

        load_fen(fen);

        token = strtok_r(NULL, " \r\n", &saveptr);
    }

    if(token && strcmp(token, "moves") == 0)
    {
        while((token = strtok_r(NULL, " \r\n", &saveptr)) != NULL)
        {
            if(strlen(token) < 4)
            {
                fprintf(stderr, "BAD UCI TOKEN: %s\n", token);
                continue;
            }

            if(!make_uci_move(token))
            {
                fprintf(stderr, "FAILED UCI MOVE: %s\n", token);
                fflush(stderr);
            }
        }
    }
}

int parse_depth(char *line)
{
    char *depth_text = strstr(line, "depth");

    if(depth_text)
        return atoi(depth_text + 6);

    return 5;
}

void uci_loop()
{
    char line[8192];

    EvalParams params;

    if(!load_champion(&params))
        params = make_balanced_params();

    while(fgets(line, sizeof(line), stdin))
    {
        if(strncmp(line, "ucinewgame", 10) == 0)
        {
            load_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        }
        else if(strcmp(line, "uci\n") == 0 || strcmp(line, "uci\r\n") == 0)
        {
            printf("id name Fissure\n");
            printf("id author Paxton Rivers\n");
            printf("uciok\n");
            fflush(stdout);
        }
        else if(strncmp(line, "isready", 7) == 0)
        {
            printf("readyok\n");
            fflush(stdout);
        }
        else if(strncmp(line, "position", 8) == 0)
        {
            uci_position(line);
        }
        else if(strncmp(line, "go", 2) == 0)
        {
            int depth = parse_depth(line);
    
            Move best = find_best_move(depth, &params);
    
            char move_text[6];
            move_to_uci(best, move_text);
    
            printf("bestmove %s\n", move_text);
            fflush(stdout);
        }
        else if(strncmp(line, "quit", 4) == 0)
        {
            break;
        }
    }
}



int main()
{

	    srand(time(NULL));
	    setup();
	    occupation();
	
	    uci_loop();
	
	    return 0;

/*
	srand(time(NULL));
	setup();
	clear_move_list();

	FILE *log = fopen("fissure_evolution.csv", "a+");

	if(log == NULL)
	{
	    printf("Failed to open evolution log file\n");
	    return 1;
	}
	
	fseek(log, 0, SEEK_END);
	
	if(ftell(log) == 0)
	{
		fprintf(log,
		    "generation,"
		    "candidate,"
		    "pawn,knight,bishop,rook,queen,"
		    "doubled_pawn_penalty,"
		    "rook_open_file_bonus,"
		    "passed_pawn_weight,"
		    "bad_bishop_pawn_penalty,"
		    "castle_bonus,"
		    "center_pawn_bonus,"
		    "knight_development_bonus,"
		    "bishop_development_bonus,"
		    "wins,draws,losses,score\n");
	}

	EvalParams starting_params;
	
	if(load_champion(&starting_params))
	{
	    printf("Loaded previous champion\n");
	}
	else
	{
	    printf("No champion found, using balanced params\n");
	    starting_params = make_balanced_params();
	}
	
	EvalParams champion =
	    run_evolution(
	        log,
	        starting_params,
	        10,
	        4,
	        5
	    );
	
	save_champion(&champion);

	fclose(log);

	printf("All Done!\n");
	
	return 0;
	*/
}


