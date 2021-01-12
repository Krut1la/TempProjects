#ifndef EVALUATION_H
#define EVALUATION_H

#include "PokerTypes.h"
#include "StringUtils.h"
#include "ThreadUtils.h"
#include "InterlRNG.h"
#include "index52c7.h"

namespace Poker
{

static unsigned int g_seed; 

static inline void fast_srand( int seed )  
{
   g_seed = seed; 
} 

static inline int Random64_fast() 
{ 
   g_seed = (214013*g_seed+2531011); 
 
   return (g_seed>>16)&63;  
} 

inline static RWORD Random64_rdtsc(void)
{
   return (__rdtsc() & 63);
}

inline static RWORD Random64_sse(unsigned int iSeedCell)
{
   unsigned int Random64;
   rand_sse(&Random64, iSeedCell);
   return (Random64 & 63);   
}

inline static RWORD Random64_standard(void)
{
   return (rand() & 63);
}

inline HAND_POWER SetKickers(const HAND Hand, const RWORD MaxKickers);

HAND_POWER CalcHandPower_Holdem(const HAND Hand);

static inline HAND_POWER CalcHandPower_Holdem(const HAND Hand, const HAND_POWER* Hash52x7)
{
   return Hash52x7[index52c7(Hand)];
}

// Example: "A"
K_UINT64 GetCardValueMask(const wchar_t Value);

// Example: "c"
K_UINT64 GetSuitMask(const wchar_t Value);

// Example: "Tc4d"
HAND StringToHand(const STRING& Hand);

// Range example: "JJ+,A7s-A2s,T9s,98s,87s,Tc4d,Tc4h,Tc4s,Td4h,Td4s,Th4d,Th4s,Ts4c,Ts4d,Ts4h"
bool RangeToHandList_Holdem(const STRING& Range, HAND_LIST& HandList);

STRING HandPowerToString(const HAND_POWER HandPower);
STRING HandToStringSimple(const HAND Hand);

// Shuffles full deck
void Shuffle(DECK& Deck);
// Shuffles deck without some cads
void inline Shuffle(DECK& Deck, const HAND DeadCards, const RWORD NumberOfHands);

HAND* CreateAllCombinations(RWORD nCardsInDeck, RWORD nCardsToDeal);

HAND_POWER* CreatePerfectHash52x7(void);

bool CalcRangesEquity_Holdem(const STRING_LIST& Ranges, 
                                   HAND_EQUITY_LIST& Equities, 
                             const STRING& Board, 
                             const STRING& DeadCards,
                                   RWORD& nGamesPlayed);



class CCalcEquityThread: public CThread
{
protected:
   HAND* m_HandSets;
   RWORD m_PlayerCount;
   RWORD m_SetCount; 
   HAND  m_Board;
   HAND  m_DeadCards;

void EnumerateAll(void);
void MonteCarlo(void);

public:
   RWORD nGamesPlayed;
   THREADMETHOD f_CallBack;
   HAND_EQUITY* m_HandEquities;

   CCalcEquityThread( bool CreateSuspended, 
                      HAND* HandSets, 
                      const RWORD SetCount,                      
                      const RWORD PlayerCount,
                      const HAND  Board,
                      const HAND  DeadCards);

   virtual ~CCalcEquityThread( void );

   virtual void Execute( void );
};

}

#endif // EVALUATION_H