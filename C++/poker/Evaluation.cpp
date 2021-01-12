#include "Evaluation.h"
#include "StringUtils.h"
#include "HardUtils.h"
#include "MathUtils.h"

#include <vector>
#include <algorithm>
#include <assert.h>
#include <time.h>

namespace Poker
{

using namespace General;


inline HAND_POWER SetKickers(const HAND Hand, const RWORD MaxKickers)
{
   HAND_POWER Result = 0;
   
   RWORD KickersLeft = MaxKickers;

   for (RWORD iCardValueMask = B_ACE; iCardValueMask >= B_DUCE; iCardValueMask--)
   {   
      if ((Hand & CardValueMasks[iCardValueMask]) > 0)
      {
         Result = Result | (KICKERS_START_BIT << (iCardValueMask - 1));

         if (--KickersLeft == 0)
           break;
      }
   }

   return Result;
}

HAND_POWER CalcHandPower_Holdem(const HAND Hand)
{
   HAND_POWER Result = 0;

   K_UINT64 StraightFlushPower  = 0;

   // There can be only one quads
   K_UINT64 QuadsPower          = 0;

   // There can be up to two trips
   K_UINT64 FirstTripsPower     = 0;
   K_UINT64 SecondTripsPower    = 0;

   // There can be up to three pairs
   K_UINT64 FirstPairPower      = 0;
   K_UINT64 SecondPairPower     = 0;
   K_UINT64 ThirdPairPower      = 0;

   K_UINT64 FlushCards          = 0;
   K_UINT64 FlushSuit           = 0;

   K_UINT64 StraightPower       = 0;


   // Check for pairs, threes or fours of kind
   for (RWORD iCardValueMask = B_DUCE; iCardValueMask <= B_ACE; iCardValueMask++)
   {
      K_UINT64 OfKind = Popcount(Hand & CardValueMasks[iCardValueMask]);

      if (OfKind == 4)
      {
         QuadsPower = iCardValueMask; 

         // There can not be any stronger combination except straight-flush
         break;
      }
      else if (OfKind == 3)
      {
         if (iCardValueMask > FirstTripsPower)
         {
            // Push trips stack
            SecondTripsPower = FirstTripsPower;
            FirstTripsPower = iCardValueMask;
         }
      }
      else if (OfKind == 2)
      {
         if (iCardValueMask > FirstPairPower)
         {
            // Push pairs stack
            ThirdPairPower = SecondPairPower;
            SecondPairPower = FirstPairPower;
            FirstPairPower = iCardValueMask;
         }
      }
   }
   
   // Check for flush
   for (RWORD iCardSuitMask = B_DIAMONDS; iCardSuitMask <= B_SPADES; iCardSuitMask++)
   {
      K_UINT64 SuitedCards = Popcount(Hand & CardSuitMasks[iCardSuitMask]);

      if (SuitedCards >= 5)
      {
         FlushCards = Hand & CardSuitMasks[iCardSuitMask];
         FlushSuit = iCardSuitMask;

         // There can be only one suit for flush
         break;
      }
   }

   // Check for straight-flush
   if (FlushCards != 0)
   {
      K_UINT64 FiveCardsInRowMask = STRAIGHT_FLUSH_SEARCH >> ((FlushSuit - 1)*13);
      K_UINT64 FiveCardWheelMask  = WHEEL_FLUSH_SEARCH >> ((FlushSuit - 1)*13);

      if ((FiveCardWheelMask & FlushCards) == FiveCardWheelMask)
          StraightFlushPower = 4;

      for (RWORD iCardValue = 0; iCardValue <= (B_ACE - 5); iCardValue++)
      {           
         if ((FiveCardsInRowMask & FlushCards) == FiveCardsInRowMask)
         {
            StraightFlushPower = B_ACE - iCardValue;
            break;
         }                                             
         FiveCardsInRowMask = FiveCardsInRowMask >> 1; 
      }
   }

   // Here we can check if we need further investigation

   if (StraightFlushPower > 0)
   {
      // Set combination
      Result = Result | STRAIGHT_FLUSH;
      // Set power
      Result = Result | (POWER_START_BIT << (StraightFlushPower - 1));
      // No kickers
      return Result;
   }

   if (QuadsPower > 0)
   {
      // Set combination
      Result = Result | FOUR_OF_KIND;
      // Set power
      Result = Result | (POWER_START_BIT << (QuadsPower - 1));
      // Set kickers
      Result = Result | SetKickers(Hand & ~CardValueMasks[QuadsPower], 1);

      return Result;
   }

   // First trips full of first pair
   if ((FirstTripsPower > 0) && (FirstPairPower > 0))
   {
      // Set combination
      Result = Result | FULL_HOUSE;
      // Set power
      Result = Result | (POWER_START_BIT << (FirstTripsPower - 1));
      // Set pair power as kicker
      Result = Result | (KICKERS_START_BIT << (FirstPairPower - 1));

      return Result;
   }

   // First trips full of second trips (of course only pair)
   if ((FirstTripsPower > 0) && (SecondTripsPower > 0))
   {
      // Set combination
      Result = Result | FULL_HOUSE;
      // Set power
      Result = Result | (POWER_START_BIT << (FirstTripsPower - 1));
      // Set pair power as kicker
      Result = Result | (KICKERS_START_BIT << (SecondTripsPower - 1));

      return Result;
   }

   if (FlushCards != 0)
   {
      // Set combination
      Result = Result | FLUSH;
      // No power, just kickers
      // Set kickers
      Result = Result | SetKickers(FlushCards, 5);

      return Result;
   }
   
   // Check for straight
   RWORD iFiveCardCounter = 0;

   K_INT64 iCardValueMask = 13;
   do
   {
      // 0 means here Ace for wheel combination
      if (((iCardValueMask == 0) && ((Hand & CardValueMasks[B_ACE]) > 0)) ||
         ((Hand & CardValueMasks[iCardValueMask]) > 0))
      {
         if (StraightPower == 0)
            StraightPower = iCardValueMask;

         iFiveCardCounter++;
      }
      else
      {
         StraightPower = 0;
         iFiveCardCounter = 0;
      }

      if (iFiveCardCounter == 5)
         break;

      iCardValueMask--;
   }while (iCardValueMask >= 0);

   // Straight
   if ((StraightPower > 0) && (iFiveCardCounter == 5))
   {
      // Set combination
      Result = Result | STRAIGHT;

      // Set power
      Result = Result | (POWER_START_BIT << (StraightPower - 1));

      // No kickers
      return Result;
   }

   // Three of kind
   if (FirstTripsPower > 0)
   {
      // Set combination
      Result = Result | THREE_OF_KIND;
      // Set power
      Result = Result | (POWER_START_BIT << (FirstTripsPower - 1));
      // Set kickers
      Result = Result | SetKickers(Hand & ~CardValueMasks[FirstTripsPower], 2);

      return Result;
   }

   // Two pairs
   if ((FirstPairPower > 0) && (SecondPairPower > 0))
   {
      // Set combination
      Result = Result | TWO_PAIRS;
      // Set power
      Result = Result | (POWER_START_BIT << (FirstPairPower - 1));
      Result = Result | (POWER_START_BIT << (SecondPairPower - 1));
      // Set kickers
      Result = Result | SetKickers(Hand & ~(CardValueMasks[FirstPairPower] | CardValueMasks[SecondPairPower]), 1);

      return Result;
   }

   // Pair
   if (FirstPairPower > 0)
   {
      // Set combination
      Result = Result | PAIR;
      // Set power
      Result = Result | (POWER_START_BIT << (FirstPairPower - 1));
      // Set kickers
      Result = Result | SetKickers(Hand & ~(CardValueMasks[FirstPairPower]), 3);

      return Result;
   }

   // High card
   Result = Result | SetKickers(Hand, 5);

   return Result;

   // Error!
   return 0; 
}

K_UINT64 GetCardValueMask(const wchar_t Value)
{
   switch (Value)
   {
   case '2':
      return DUCES;
   case '3':
      return THREES;
   case '4':
      return FOURS;
   case '5':
      return FIVES;
   case '6':
      return SIXES;
   case '7':
      return SEVENS;
   case '8':
      return EIGHTS;
   case '9':
      return NINES;
   case 'T':
      return TENS;
   case 'J':
      return JACKS;
   case 'Q':
      return QUEENS;
   case 'K':
      return KINGS;
   case 'A':
      return ACES;
   default:
      return 0;
   }
}

K_UINT64 GetSuitMask(const wchar_t Value)
{
   switch (Value)
   {
   case 'c':
      return CLUBS;
   case 'd':
      return DIAMONDS;
   case 's':
      return SPADES;
   case 'h':
      return HEARTS;
   default:
      return 0;
   }
}

// Example: "Tc4d"
HAND StringToHand(const STRING& Hand)
{
   HAND Result = 0;

   if ((Hand.length()%2) != 0)
      return Result;

   for (STRING::const_iterator iChar = Hand.begin(); iChar < Hand.end(); ++iChar)
   {
      HAND aCard = 0;
      aCard = aCard | GetCardValueMask(*iChar);
      aCard = aCard & GetSuitMask(*(++iChar));

      Result = Result | aCard;
   }

   return Result;
}

// Range example: "JJ+,A7s-A2s,T9s,98s,87s,Tc4d,Tc4h,Tc4s,Td4h,Td4s,Th4d,Th4s,Ts4c,Ts4d,Ts4h"
bool RangeToHandList_Holdem(const STRING& Range, HAND_LIST& HandList)
{
   // Remove all spaces

   // Split string by "," delimiter
   STRING_LIST Ranges = Split(Range, L",", 100);

   size_t Pos = 0;   

   for(STRING_LIST::iterator iRange = Ranges.begin(); iRange < Ranges.end(); iRange++)
   {
      // Determine values and suits ranges
      bool bPair     = ((*iRange)[0] == (*iRange)[1]);
      bool bOffsuit  = ((*iRange).find('o') != -1);
      bool bSuit     = ((*iRange).length() == 3) && ((*iRange).find('s') != -1);
      bool bPlus     = ((*iRange).find('+') != -1);
      bool bMinus    = ((*iRange).find('-') != -1);

      size_t Start1 = CardValues.find((*iRange)[0]);
      size_t Start2 = Start1;;
      size_t End1;
      size_t End2;

      // Single
      if (!(bPair || bOffsuit || bSuit || bPlus || bMinus ))
         HandList.push_back(StringToHand(*iRange));
      else
      {
         Start2 = CardValues.find((*iRange)[1]);

         if (bPair)
         {
            if (bPlus)
            {
              End1 = CardValues.length() - 1;
              End2 = End1;
            }
            else if (bMinus)
            {
               End1 = CardValues.find((*iRange)[3]);
               End2 = End1;
            }
         }
         else
         {
            if (bPlus)
            {
              End1 = Start1;
              End2 = Start1-1;
            }
            else if (bMinus)
            {
               End1 = Start1;
               End2 = CardValues.find((*iRange)[5]);
            }
            else if (bSuit || bOffsuit)
            {
               End1 = Start1;
               End2 = Start1-1;
            }
         }

         if (Start1 > End1)
         {
            size_t tmp = End1;
            End1 = Start1;
            Start1 = tmp;
         }

         if (Start2 > End2)
         {
            size_t tmp = End2;
            End2 = Start2;
            Start2 = tmp;
         }

         // Value of card2
         for (size_t iCardValue2 = Start2; iCardValue2 <= End2; iCardValue2++)
         {
            if (bOffsuit || bPair)
            {
               // Suit of card1
               for (size_t iSuit1 = 0; iSuit1 < Suits.length(); iSuit1++)
               {
                  // Suit of card2
                  for (size_t iSuit2 = bPair?iSuit1:0; iSuit2 < Suits.length(); iSuit2++)
                  {
                     if (iSuit1 != iSuit2)
                     {
                        STRING Hand = L"";
                        if (bPair)
                           Hand = Hand + CardValues[iCardValue2] + Suits[iSuit1] + CardValues[iCardValue2] + Suits[iSuit2];
                        else
                           Hand = Hand + CardValues[CardValues.find((*iRange)[0])] + Suits[iSuit1] + CardValues[iCardValue2] + Suits[iSuit2];

                        HandList.push_back(StringToHand(Hand));
                     }
                  }
               }
            }
            else
            {
               // Suit of card2
               for (size_t iSuit1 = 0; iSuit1 < Suits.length(); iSuit1++)
               {
                  STRING Hand = L"";
                  Hand = Hand + CardValues[CardValues.find((*iRange)[0])] + Suits[iSuit1] + CardValues[iCardValue2] + Suits[iSuit1];
                  HandList.push_back(StringToHand(Hand));
               }
            }
         }
      }
   }

   return true;
}

STRING HandPowerToString(const HAND_POWER HandPower)
{
   STRING Result = L"";

   RWORD KickersToShow = 0;

   K_UINT64 Power1 = 0;
   K_UINT64 Power2 = 0;

   for (RWORD iCardValue = B_ACE; iCardValue >= B_DUCE; iCardValue--)
   {
      if ((POWER_START_BIT << (iCardValue - 1)) & HandPower)
      {
         if (Power1 == 0)
            Power1 = iCardValue;
         else
            Power2 = iCardValue;
      }
   }

   if ((HandPower & PAIR) != 0)
   {
      Result = Result + rsPAIR + rsOF + rsCardValues[Power1];
      KickersToShow = 3;
   }
   else if ((HandPower & TWO_PAIRS) != 0)
   {
      Result = Result + rsTWO_PAIRS + L", " + rsCardValues[Power1] + rsAND + rsCardValues[Power2] ;
      KickersToShow = 1;
   }
   else if ((HandPower & THREE_OF_KIND) != 0)
   {
      Result = Result + rsTHREE_OF_KIND + L", " + rsCardValues[Power1];
      KickersToShow = 2;
   }
   else if ((HandPower & STRAIGHT) != 0)
   {
      Result = Result + rsSTRAIGHT + rsFROM + rsCardValue[Power1];
   }
   else if ((HandPower & FLUSH) != 0)
   {
      Result = Result + rsFLUSH;
      KickersToShow = 5;
   }
   else if ((HandPower & FULL_HOUSE) != 0)
   {

      for (RWORD iCardValue = 13; iCardValue >= 1; iCardValue--)
      {
         if ((KICKERS_START_BIT << (iCardValue - 1)) & HandPower)
         {              
            Power2 = iCardValue;
            break;
         }
      }

      Result = Result + rsFULL_HOUSE + L", " + rsCardValues[Power1] + rsFULL_OF + rsCardValues[Power2];
   }
   else if ((HandPower & FOUR_OF_KIND) != 0)
   {
      Result = Result + rsFOUR_OF_KIND + L", " + rsCardValues[Power1];
      KickersToShow = 1;
   }
   else if ((HandPower & STRAIGHT_FLUSH) != 0)
   {
      Result = Result + rsSTRAIGHT_FLUSH + rsFROM + rsCardValue[Power1];
   }
   else 
   {
      Result = Result + rsHIGH_CARD;
      KickersToShow = 5;
   }


   //Append kickers info
   if (KickersToShow > 0)
   {
      Result = Result + L", " + rsKICKERS;

      for (RWORD iCardValue = B_ACE; iCardValue >= B_DUCE; iCardValue--)
         if (((KICKERS_START_BIT << (iCardValue - 1)) & HandPower) != 0 )
         {
            Result = Result + CardValues[iCardValue - 1];
            
            if (--KickersToShow > 0)
               Result = Result + L", ";
         }
   }

   return Result;
}

STRING HandToStringSimple(const HAND Hand)
{
   STRING Result = L"";
   
   K_UINT64 Card = 1;

   RWORD Cards = Popcount(Hand);
   
   for (RWORD iCardValue = B_ACE; iCardValue >= B_DUCE; iCardValue--)
   {
      for (RWORD iCardSuit = B_SPADES; iCardSuit >= B_DIAMONDS; iCardSuit--)
      {
         RWORD Shift = iCardValue + (B_ACE)*(iCardSuit - 1) - 1;

         if ((Hand & (Card << Shift)) !=0)
         {
            Result = Result + CardValues[iCardValue-1] + Suits[iCardSuit-1];

            if (0 != --Cards)
               Result = Result + L", ";
         }
      }
   }

   return Result;
}

void Shuffle(DECK& Deck)
{
   K_UINT64 Shift = 1;   

   HAND DeckCheck = 0;

   RWORD Cards = 52; 

   RWORD iCard = 0;

   while (iCard != Cards)
   {
      HAND aCard = Shift << (rand() % 52);

      if ((DeckCheck & aCard) == 0)
      {
         DeckCheck = DeckCheck | aCard;
         Deck[iCard] = aCard;
         iCard++;
      }
   }

   assert(DeckCheck == FULL_DECK);
}

void inline Shuffle(DECK& Deck, const HAND DeadCards, const RWORD NumberOfHands)
{
#ifdef _DEBUG
   RWORD nAttmpts = 0;
#endif

   HAND DeckCheck = 0;

   RWORD Shift = 0;
   RWORD iCard = 0;
   
   while (iCard != NumberOfHands)
   {
      HAND aCard = 1;
      Shift = Random64_fast();

      if (Shift > 51) continue;
      aCard = aCard << Shift;

      if (((DeckCheck & aCard) == 0) && ((DeadCards & aCard)) == 0)
      {
         DeckCheck = DeckCheck | aCard;
         Deck[iCard] = aCard;
         iCard++;
      }
#ifdef _DEBUG
      nAttmpts ++;
#endif
   }
}

HAND* CreateAllCombinations(RWORD nCardsInDeck, RWORD nCardsToDeal)
{
   RWORD C = Combinations(nCardsInDeck, nCardsToDeal);

   // All combinations array
   HAND* AllCombinations = new HAND[C];

   HAND Shift = 1;

   RWORD* Begins = new RWORD[nCardsToDeal];
   RWORD* Ends = new RWORD[nCardsToDeal];
   RWORD* Steps = new RWORD[nCardsToDeal];
   RWORD* Current = new RWORD[nCardsToDeal];

   for (RWORD iCardsToDeal = 0; iCardsToDeal < nCardsToDeal; iCardsToDeal++)
   {
      Begins[iCardsToDeal] = nCardsToDeal - iCardsToDeal  - 1;
      Ends[iCardsToDeal] = nCardsInDeck - iCardsToDeal;
      Steps[iCardsToDeal] = 1;
      Current[iCardsToDeal] = Begins[iCardsToDeal];
   }

   RWORD iCombination = 0;

   while (Current[nCardsToDeal - 1] < Ends[nCardsToDeal - 1])
   {
      ///

      HAND Hand = 0;

      for (RWORD iCardsToDeal = 0; iCardsToDeal < nCardsToDeal; iCardsToDeal++)
         Hand = Hand | (Shift << Current[iCardsToDeal]);

      AllCombinations[iCombination] = Hand;

      iCombination++;
       
      ///

      RWORD Add = Steps[0];
      for(RWORD iLevel = 0; (iLevel < nCardsToDeal && Add > 0); iLevel++)
      {
         Current[iLevel] += Add;
         if(Current[iLevel] < Ends[iLevel])
         {
            Add = 0;

            if (iLevel > 0)
               for (INT64 iUpperLevel = iLevel - 1; iUpperLevel >= 0; iUpperLevel--)
                  Current[iUpperLevel] = Current[iLevel] + ((iLevel) - (RWORD)iUpperLevel);
         }
         else
         {
            if (iLevel == (nCardsToDeal - 1))
               break;

            Add = Steps[iLevel + 1];
         }
      }
   }

   delete [] Begins;
   delete [] Ends;
   delete [] Steps;
   delete [] Current;

   assert(iCombination == C);

   return AllCombinations;
}

HAND_POWER* CreatePerfectHash52x7(void)
{
   // All 7-card combination
   HAND_POWER* All7Cards = new HAND_POWER[133784560];
      
   RWORD nCardsToDeal = 7;

   HAND Shift = 1;

   RWORD* Begins = new RWORD[nCardsToDeal];
   RWORD* Ends = new RWORD[nCardsToDeal];
   RWORD* Steps = new RWORD[nCardsToDeal];
   RWORD* Current = new RWORD[nCardsToDeal];

   for (RWORD iCardsToDeal = 0; iCardsToDeal < nCardsToDeal; iCardsToDeal++)
   {
      Begins[iCardsToDeal] = nCardsToDeal - iCardsToDeal  - 1;
      Ends[iCardsToDeal] = 52 - iCardsToDeal;
      Steps[iCardsToDeal] = 1;
      Current[iCardsToDeal] = Begins[iCardsToDeal];
   }

   RWORD iCombination = 0;

   while (Current[nCardsToDeal - 1] < Ends[nCardsToDeal - 1])
   {
      ///

      HAND Hand7 = 0;

      for (RWORD iCardsToDeal = 0; iCardsToDeal < nCardsToDeal; iCardsToDeal++)
         Hand7 = Hand7 | (Shift << Current[iCardsToDeal]);

      All7Cards[iCombination] = CalcHandPower_Holdem(Hand7);

      iCombination++;

 
      ///

      RWORD Add = Steps[0];
      for(RWORD iLevel = 0; (iLevel < nCardsToDeal && Add > 0); iLevel++)
      {
         Current[iLevel] += Add;
         if(Current[iLevel] < Ends[iLevel])
         {
            Add = 0;

            if (iLevel > 0)
               for (INT64 iUpperLevel = iLevel - 1; iUpperLevel >= 0; iUpperLevel--)
                  Current[iUpperLevel] = Current[iLevel] + ((iLevel) - (RWORD)iUpperLevel);
         }
         else
         {
            if (iLevel == (nCardsToDeal - 1))
               break;

            Add = Steps[iLevel + 1];
         }
      }
   }

   delete [] Begins;
   delete [] Ends;
   delete [] Steps;
   delete [] Current;

   return All7Cards;
}


bool CalcRangesEquity_Holdem(const STRING_LIST& Ranges, 
                                   HAND_EQUITY_LIST& Equities, 
                             const STRING& Board, 
                             const STRING& DeadCards,
                                   RWORD& nGamesPlayed)
{
   RWORD nPlayers = Ranges.size();

   if (nPlayers < 2) return false;

   HAND hBoard = StringToHand(Board);
   HAND hDeadCards = StringToHand(DeadCards);

   // Convert ranges to lists of HAND
   HAND_LIST_LIST HandLists;

   for (STRING_LIST::const_iterator iRange = Ranges.begin(); iRange < Ranges.end(); iRange++)
   {
      HAND_LIST HandList;
      RangeToHandList_Holdem(*iRange, HandList);
      HandLists.push_back(HandList);
   }

   // Enumerate all combinations
   RWORD nSets = 1;
   RWORD* Begins = new RWORD[nPlayers];
   RWORD* Ends = new RWORD[nPlayers];
   RWORD* Steps = new RWORD[nPlayers];
   RWORD* Current = new RWORD[nPlayers];

   for (RWORD iPlayer = 0; iPlayer < nPlayers; iPlayer++)
   {
      nSets = nSets*(HandLists[iPlayer].size());
      Begins[iPlayer] = 0;
      Ends[iPlayer] = HandLists[iPlayer].size();
      Steps[iPlayer] = 1;
      Current[iPlayer] = 0;
   }
     
   HAND* HandSets = new HAND[nSets*nPlayers];

   RWORD iHandSet = 0;

   while (Current[nPlayers - 1] < Ends[nPlayers - 1])
   {
      HAND SkipCheck = 0;
      bool bSkipSet = false;

      for (RWORD iPlayer = 0; iPlayer < nPlayers; iPlayer++)
      {      
         HAND iHand = HandLists[iPlayer][Current[iPlayer]];
      
         // Skip all combinations with coincided cards
         if (((SkipCheck & iHand) != 0) ||
             ((hBoard & iHand) != 0) ||
             ((hDeadCards & iHand) != 0))
         {
           bSkipSet = true;
           break;
         }
         else
           SkipCheck = SkipCheck | iHand;

         HandSets[iHandSet + iPlayer] = iHand;
      }
      
      if (!bSkipSet) 
      {         

         iHandSet += nPlayers;
      }

      RWORD Add = Steps[0];
      for(RWORD iLevel = 0; (iLevel < nPlayers && Add > 0); iLevel++)
      {
         Current[iLevel] += Add;
         if(Current[iLevel] < Ends[iLevel])
            Add = 0;
         else
         {
            if (iLevel == (nPlayers - 1))
               break;

            Add = Steps[iLevel + 1];
            Current[iLevel] = Begins[iLevel];
         }
      }
   }

   delete [] Begins;
   delete [] Ends;
   delete [] Steps;
   delete [] Current;

   
   // Create thread pool
   RWORD nCores = 1;

   CCalcEquityThread** CalcEquityThreadPool = new CCalcEquityThread*[nCores];

   for(RWORD iCore = 0; iCore < nCores; iCore++)
   {
      CalcEquityThreadPool[iCore] = new CCalcEquityThread(true, 
                                                          HandSets, 
                                                          iHandSet/nPlayers, 
                                                          nPlayers, 
                                                          hBoard, 
                                                          hDeadCards);
   }

   // Run
   for(RWORD iCore = 0; iCore < nCores; iCore++)
     CalcEquityThreadPool[iCore]->Resume();

   for(RWORD iCore = 0; iCore < nCores; iCore++)
     CalcEquityThreadPool[iCore]->WaitFor();

   // Calculate results
   for(RWORD iCore = 0; iCore < nCores; iCore++)
   {
      for (RWORD iPlayer = 0; iPlayer < nPlayers; iPlayer++)
      {
         Equities[iPlayer].Wins += CalcEquityThreadPool[iCore]->m_HandEquities[iPlayer].Wins;
         Equities[iPlayer].Loses += CalcEquityThreadPool[iCore]->m_HandEquities[iPlayer].Loses;
         Equities[iPlayer].Ties += CalcEquityThreadPool[iCore]->m_HandEquities[iPlayer].Ties;
      }

      nGamesPlayed += CalcEquityThreadPool[iCore]->nGamesPlayed;
   }
      
   for(RWORD iCore = 0; iCore < nCores; iCore++)
     delete CalcEquityThreadPool[iCore];

   delete [] CalcEquityThreadPool;  

   return true;
}


CCalcEquityThread::CCalcEquityThread( bool CreateSuspended, 
                                      HAND* HandSets, 
                                      const RWORD SetCount,                      
                                      const RWORD PlayerCount,
                                      const HAND  Board,
                                      const HAND  DeadCards):
CThread( CreateSuspended ), 
m_HandSets( HandSets ),
m_SetCount( SetCount ),
m_PlayerCount( PlayerCount ),
m_Board( Board ),
m_DeadCards( DeadCards )
{
   nGamesPlayed = 0;
   m_HandEquities = new HAND_EQUITY[m_PlayerCount];

   for (RWORD iHand = 0; iHand < m_PlayerCount; iHand++)
   {
      m_HandEquities[iHand].Wins = 0;
      m_HandEquities[iHand].Loses = 0;
      m_HandEquities[iHand].Ties = 0;
   }
}

CCalcEquityThread::~CCalcEquityThread( void )
{
   delete [] m_HandEquities;   
}


void CCalcEquityThread::EnumerateAll(void)
{    
   HAND Shift = 1;

   const RWORD nCardsInDeck = 52;
   RWORD nCardsToDeal = 5 - Popcount(m_Board);

   RWORD C = Combinations(nCardsInDeck, nCardsToDeal);

   HAND* All5CardCombinations = CreateAllCombinations(nCardsInDeck, nCardsToDeal);

   HAND_POWER* HandPowers = new HAND_POWER[m_PlayerCount];
  
   for (RWORD iHandSet = 0; iHandSet < m_SetCount*m_PlayerCount; iHandSet+=m_PlayerCount)
   {  

      for (RWORD i5CardCombination = 0; i5CardCombination < C; i5CardCombination++)
      {
         ///

         HAND Board = All5CardCombinations[i5CardCombination];

         bool bSkip = false;

         if (((Board & m_Board) != 0) ||
             ((Board & m_DeadCards) != 0))
             bSkip = true;
         
         if (!bSkip)
            for (RWORD iHand = 0; iHand < m_PlayerCount; iHand++)
               if ((Board & m_HandSets[iHandSet + iHand]) != 0)
               {
                  bSkip = true;
                  break;
               }

         // Skip if any of cards coinsides in Hand1, Hand2, Board and DeadCards
         // Coinsides between the board and the dead card should be checked in advance
         if (!bSkip)
         {
            HAND_POWER BestHandPower = 0;
            bool HaveOneWinner = false;
            RWORD nTiedHands = 0;
          
            for (RWORD iHand = 0; iHand < m_PlayerCount; iHand++)
            {
               HandPowers[iHand] = CalcHandPower_Holdem(m_HandSets[iHandSet + iHand] | Board);

               if (HandPowers[iHand] > BestHandPower)
               {
                  HaveOneWinner = true;
                  BestHandPower = HandPowers[iHand];
                  nTiedHands = 0;
               }
               else if (HandPowers[iHand] == BestHandPower)
               {
                  nTiedHands++;
                  HaveOneWinner = false;
               }
            }

            for (RWORD iHand = 0; iHand < m_PlayerCount; iHand++)
            {
               if (HandPowers[iHand] == BestHandPower)
               {
                  if (HaveOneWinner)
                  {
                     m_HandEquities[iHand].Wins++;
                  }
                  else
                     m_HandEquities[iHand].Ties += 1.0f/(nTiedHands + 1);
               }
               else
                  m_HandEquities[iHand].Loses++;
            }   

            nGamesPlayed++;
         }
 
         ///
      }

   }

   delete [] HandPowers;
   delete [] All5CardCombinations;
}



/*
void CCalcEquityThread::EnumerateAll(void)
{    
   HAND Shift = 1;

   RWORD nCardsToDeal = 5 - Popcount(m_Board);

   HAND_POWER* HandPowers = new HAND_POWER[m_PlayerCount];

   RWORD* Begins = new RWORD[nCardsToDeal];
   RWORD* Ends = new RWORD[nCardsToDeal];
   RWORD* Steps = new RWORD[nCardsToDeal];
   RWORD* Current = new RWORD[nCardsToDeal];
      
   const RWORD nCards = 52;
  
   for (RWORD iHandSet = 0; iHandSet < m_SetCount*m_PlayerCount; iHandSet+=m_PlayerCount)
   {  
      for (RWORD iCardsToDeal = 0; iCardsToDeal < nCardsToDeal; iCardsToDeal++)
      {
         Begins[iCardsToDeal] = nCardsToDeal - iCardsToDeal  - 1;
         Ends[iCardsToDeal] = nCards - iCardsToDeal;
         Steps[iCardsToDeal] = 1;
         Current[iCardsToDeal] = Begins[iCardsToDeal];
      }

      while (Current[nCardsToDeal - 1] < Ends[nCardsToDeal - 1])
      {
         ///

         HAND Board = 0;

         for (RWORD iCardsToDeal = 0; iCardsToDeal < nCardsToDeal; iCardsToDeal++)
            Board = Board | (Shift << Current[iCardsToDeal]);

         bool bSkip = false;

         if (((Board & m_Board) != 0) ||
             ((Board & m_DeadCards) != 0))
             bSkip = true;

         
         if (!bSkip)
            for (RWORD iHand = 0; iHand < m_PlayerCount; iHand++)
               if ((Board & m_HandSets[iHandSet + iHand]) != 0)
               {
                  bSkip = true;
                  break;
               }

         // Skip if any of cards coinsides in Hand1, Hand2, Board and DeadCards
         // Coinsides between the board and the dead card should be checked in advance
         if (!bSkip)
         {
            HAND_POWER BestHandPower = 0;
            bool HaveOneWinner = false;
            RWORD nTiedHands = 0;
          
            for (RWORD iHand = 0; iHand < m_PlayerCount; iHand++)
            {
               HandPowers[iHand] = CalcHandPower_Holdem(m_HandSets[iHandSet + iHand] | Board);

               if (HandPowers[iHand] > BestHandPower)
               {
                  HaveOneWinner = true;
                  BestHandPower = HandPowers[iHand];
                  nTiedHands = 0;
               }
               else if (HandPowers[iHand] == BestHandPower)
               {
                  nTiedHands++;
                  HaveOneWinner = false;
               }
            }

            for (RWORD iHand = 0; iHand < m_PlayerCount; iHand++)
            {
               if (HandPowers[iHand] == BestHandPower)
               {
                  if (HaveOneWinner)
                  {
                     m_HandEquities[iHand].Wins++;
                  }
                  else
                     m_HandEquities[iHand].Ties += 1.0f/(nTiedHands + 1);
               }
               else
                  m_HandEquities[iHand].Loses++;
            }   

            nGamesPlayed++;
         }
 
         ///

         RWORD Add = Steps[0];
         for(RWORD iLevel = 0; (iLevel < nCardsToDeal && Add > 0); iLevel++)
         {
            Current[iLevel] += Add;
            if(Current[iLevel] < Ends[iLevel])
            {
               Add = 0;

               if (iLevel > 0)
                  for (INT64 iUpperLevel = iLevel - 1; iUpperLevel >= 0; iUpperLevel--)
                     Current[iUpperLevel] = Current[iLevel] + ((iLevel) - (RWORD)iUpperLevel);
            }
            else
            {
               if (iLevel == (nCardsToDeal - 1))
                  break;

               Add = Steps[iLevel + 1];
            }
         }
      }
   }
                            
   delete [] Begins;
   delete [] Ends;
   delete [] Steps;
   delete [] Current;

   delete [] HandPowers;
}
*/

void CCalcEquityThread::MonteCarlo(void)
{
   //srand_sse ((unsigned int)time(NULL), 0);     
   fast_srand((unsigned int)time(NULL));

   DECK Deck;
   HAND Board = 0;
   RWORD CardsToDeal = 5 - Popcount(m_Board);

   const RWORD Iterations = 100000000/m_SetCount; 

   HAND_POWER* HandPowers = new HAND_POWER[m_PlayerCount];

   if (m_PlayerCount == 2)
   {
      // It is quite simplier and faster and most frequent user case
      for (RWORD iIteration = 1; iIteration <= Iterations; iIteration++)
      {
         for (RWORD iHandSet = 0; iHandSet < m_SetCount*2; iHandSet+=2)
         {
            HAND DeadCards = m_Board | m_DeadCards | m_HandSets[iHandSet] | m_HandSets[iHandSet + 1];

            Shuffle(Deck, DeadCards, CardsToDeal);

            Board = m_Board;

            for (RWORD iCard = 0; iCard < CardsToDeal; iCard++)
               Board = Board | Deck[iCard];

            HAND_POWER BestHandPower = 0;
            bool HaveOneWinner = false;
            RWORD nTiedHands = 0;
          
            HandPowers[0] = CalcHandPower_Holdem(m_HandSets[iHandSet] | Board);
            HandPowers[1] = CalcHandPower_Holdem(m_HandSets[iHandSet + 1] | Board);
                        
            if (HandPowers[0] > HandPowers[1])
            {
               m_HandEquities[0].Wins++;
               m_HandEquities[1].Loses++;
            }
            else if (HandPowers[0] < HandPowers[1])
            {
               m_HandEquities[0].Loses++;
               m_HandEquities[1].Wins++;
            }
            else
            {
               m_HandEquities[0].Ties += 0.5f;
               m_HandEquities[1].Ties += 0.5f;
            }

            nGamesPlayed++;
         }
      }
   }
   else
   {
      for (RWORD iIteration = 1; iIteration <= Iterations; iIteration++)
      {

         for (RWORD iHandSet = 0; iHandSet < m_SetCount*m_PlayerCount; iHandSet+=m_PlayerCount)
         {
            HAND DeadCards = m_Board | m_DeadCards;

            for (RWORD iHand = 0; iHand < m_PlayerCount; iHand++)
               DeadCards = DeadCards | m_HandSets[iHandSet + iHand];

            Shuffle(Deck, DeadCards, CardsToDeal);

            Board = m_Board;

            for (RWORD iCard = 0; iCard < CardsToDeal; iCard++)
               Board = Board | Deck[iCard];

            HAND_POWER BestHandPower = 0;
            bool HaveOneWinner = false;
            RWORD nTiedHands = 0;
          
            for (RWORD iHand = 0; iHand < m_PlayerCount; iHand++)
            {
               HandPowers[iHand] = CalcHandPower_Holdem(m_HandSets[iHandSet + iHand] | Board);

               if (HandPowers[iHand] > BestHandPower)
               {
                  HaveOneWinner = true;
                  BestHandPower = HandPowers[iHand];
                  nTiedHands = 0;
               }
               else if (HandPowers[iHand] == BestHandPower)
               {
                  nTiedHands++;
                  HaveOneWinner = false;
               }
            }

            for (RWORD iHand = 0; iHand < m_PlayerCount; iHand++)
            {
               if (HandPowers[iHand] == BestHandPower)
               {
                  if (HaveOneWinner)
                  {
                     m_HandEquities[iHand].Wins++;
                  }
                  else
                     m_HandEquities[iHand].Ties += 1.0f/(nTiedHands + 1);
               }
               else
                  m_HandEquities[iHand].Loses++;
            }    

            nGamesPlayed++;
         }
      }
   }

   delete [] HandPowers;
}

void CCalcEquityThread::Execute( void )
{  
   //MonteCarlo();
   EnumerateAll();
}

}