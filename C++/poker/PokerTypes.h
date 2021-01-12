#ifndef POKERTYPES_H
#define POKERTYPES_H

#include "Types.h"
#include "StringUtils.h"
#include <vector>


namespace Poker
{
   using namespace General;

   // Resource strings
   const STRING rsHIGH_CARD      = L"High card";
   const STRING rsPAIR           = L"Pair";
   const STRING rsTWO_PAIRS      = L"Two pairs";
   const STRING rsTHREE_OF_KIND  = L"Three of kind";
   const STRING rsSTRAIGHT       = L"Straight";
   const STRING rsFLUSH          = L"Flush";
   const STRING rsFULL_HOUSE     = L"Full house";
   const STRING rsFOUR_OF_KIND   = L"Four of kind";
   const STRING rsSTRAIGHT_FLUSH = L"Straight flush";
   const STRING rsROYAL_FLUSH    = L"Royal flush";

   const STRING rsFROM           = L" from ";
   const STRING rsOF             = L" of ";
   const STRING rsAND            = L" and ";
   const STRING rsFULL_OF        = L" full of ";
   const STRING rsKICKERS        = L"with kickers: ";

   const STRING rsDUCES    = L"Duces";
   const STRING rsTHREES   = L"Threes";
   const STRING rsFOURS    = L"Fours";
   const STRING rsFIVES    = L"Fives";
   const STRING rsSIXES    = L"Sixes";
   const STRING rsSEVENS   = L"Sevens";
   const STRING rsEIGHTS   = L"Eights";
   const STRING rsNINES    = L"Nines";
   const STRING rsTENS     = L"Tens";
   const STRING rsJACKS    = L"Jacks";
   const STRING rsQUEENS   = L"Queens";
   const STRING rsKINGS    = L"Kings";
   const STRING rsACES     = L"Aces";

   const STRING rsDUCE    = L"Duce";
   const STRING rsTHREE   = L"Three";
   const STRING rsFOUR    = L"Four";
   const STRING rsFIVE    = L"Five";
   const STRING rsSIXE    = L"Six";
   const STRING rsSEVEN   = L"Seven";
   const STRING rsEIGHT   = L"Eight";
   const STRING rsNINE    = L"Nine";
   const STRING rsTEN     = L"Ten";
   const STRING rsJACK    = L"Jack";
   const STRING rsQUEEN   = L"Queen";
   const STRING rsKING    = L"King";
   const STRING rsACE     = L"Ace";

   const STRING               rsCardValues[14] = {L"", rsDUCES, rsTHREES, rsFOURS, rsFIVES, rsSIXES, rsSEVENS, rsEIGHTS, 
                                                  rsNINES, rsTENS, rsJACKS, rsQUEENS, rsKINGS, rsACES};

   const STRING               rsCardValue[14] = {L"", rsDUCE, rsTHREE, rsFOUR, rsFIVE, rsSIXE, rsSEVEN, rsEIGHT, 
                                                  rsNINE, rsTEN, rsJACK, rsQUEEN, rsKING, rsACE};


   // General types
   typedef K_UINT64                   HAND;
   typedef K_UINT64                   HAND_POWER;
   typedef HAND                     HAND_PAIR[2];
   typedef std::vector<HAND>        HAND_LIST;
   typedef std::vector<HAND_LIST>   HAND_LIST_LIST;
   typedef HAND                     DECK[52];

   const RWORD  MAX_BOARD_CARDS = 5; 
   
   const STRING CardValues = L"23456789TJQKA";
   const STRING Suits      = L"dhcs";   



   // Boundaries
   const RWORD                B_DUCE     = 1;
   const RWORD                B_ACE      = 13;

   const RWORD                B_DIAMONDS = 1;
   const RWORD                B_SPADES   = 4;

   // Masks
   const K_UINT64               DIAMONDS = 0xFFF8000000000;   //1111111111111000000000000000000000000000000000000000
   const K_UINT64               HEARTS   = 0x7FFC000000;      //0000000000000111111111111100000000000000000000000000
   const K_UINT64               CLUBS    = 0x3FFE000;         //0000000000000000000000000011111111111110000000000000
   const K_UINT64               SPADES   = 0x1FFF;            //0000000000000000000000000000000000000001111111111111
                                                                                                                
   const K_UINT64               DUCES    = 0x8004002001;      //0000000000001000000000000100000000000010000000000001
   const K_UINT64               THREES   = 0x10008004002;     //0000000000010000000000001000000000000100000000000010
   const K_UINT64               FOURS    = 0x20010008004;     //0000000000100000000000010000000000001000000000000100
   const K_UINT64               FIVES    = 0x40020010008;     //0000000001000000000000100000000000010000000000001000
   const K_UINT64               SIXES    = 0x80040020010;     //0000000010000000000001000000000000100000000000010000
   const K_UINT64               SEVENS   = 0x100080040020;    //0000000100000000000010000000000001000000000000100000
   const K_UINT64               EIGHTS   = 0x200100080040;    //0000001000000000000100000000000010000000000001000000
   const K_UINT64               NINES    = 0x400200100080;    //0000010000000000001000000000000100000000000010000000
   const K_UINT64               TENS     = 0x800400200100;    //0000100000000000010000000000001000000000000100000000
   const K_UINT64               JACKS    = 0x1000800400200;   //0001000000000000100000000000010000000000001000000000
   const K_UINT64               QUEENS   = 0x2001000800400;   //0010000000000001000000000000100000000000010000000000
   const K_UINT64               KINGS    = 0x4002001000800;   //0100000000000010000000000001000000000000100000000000
   const K_UINT64               ACES     = 0x8004002001000;   //1000000000000100000000000010000000000001000000000000

   const K_UINT64              FULL_DECK = 0xFFFFFFFFFFFFF;   //1111111111111111111111111111111111111111111111111111

   const K_UINT64 CardValueMasks[14] = {0, DUCES, THREES, FOURS, FIVES, SIXES, SEVENS, EIGHTS, NINES, TENS, JACKS, QUEENS, KINGS, ACES};
   const K_UINT64 CardSuitMasks[5]   = {0, DIAMONDS, HEARTS, CLUBS, SPADES};
   

   // "High" combinations specific : Hold'em, Omaha HI, 7-card Stud Hi, 5-card draw

   const K_UINT64  STRAIGHT_FLUSH_SEARCH = 0xF800000000000;   //1111100000000000000000000000000000000000000000000000
   const K_UINT64  WHEEL_FLUSH_SEARCH    = 0x8078000000000;   //1000000001111000000000000000000000000000000000000000

   // Hand power bits                                       //combination  power         kickers
   const K_UINT64               PAIR           = 0x4000000;   //00000001     0000000000000 0000000000000
   const K_UINT64               TWO_PAIRS      = 0x8000000;   //00000010     0000000000000 0000000000000
   const K_UINT64               THREE_OF_KIND  = 0x10000000;  //00000100     0000000000000 0000000000000
   const K_UINT64               STRAIGHT       = 0x20000000;  //00001000     0000000000000 0000000000000
   const K_UINT64               FLUSH          = 0x40000000;  //00010000     0000000000000 0000000000000
   const K_UINT64               FULL_HOUSE     = 0x80000000;  //00100000     0000000000000 0000000000000
   const K_UINT64               FOUR_OF_KIND   = 0x100000000; //01000000     0000000000000 0000000000000
   const K_UINT64               STRAIGHT_FLUSH = 0x200000000; //10000000     0000000000000 0000000000000
   
   const K_UINT64               POWER_MASK     = 0x3FFE000;   //00000000     1111111111111 0000000000000
   const K_UINT64              POWER_START_BIT = 0x2000;      //00000000     0000000000001 0000000000000
   const K_UINT64            KICKERS_START_BIT = 0x1;         //00000000     0000000000000 0000000000001

   const K_UINT64            BBJP              = 0x100080001; //01000000     0000001000000 0000000000001 Quad 8888

   // Low combinations specific : Omaha H\L, 7-card Stud H\L, Razz

   // Hand power bits                                       //



     

   // Equity

   typedef struct 
   {
      RWORD Wins;
      RWORD Loses;
      double Ties;
   } HAND_EQUITY;

   typedef std::vector<HAND_EQUITY> HAND_EQUITY_LIST;
}

#endif // POKERTYPES_H