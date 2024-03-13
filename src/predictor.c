//========================================================//
//  predictor.c                                           //
//  Source file for the Branch Predictor                  //
//                                                        //
//  Implement the various branch predictors below as      //
//  described in the README                               //
//========================================================//
#include <stdio.h>
#include "predictor.h"

//
// TODO:Student Information
//
const char *studentName = "NAME";
const char *studentID   = "PID";
const char *email       = "EMAIL";

//------------------------------------//
//      Predictor Configuration       //
//------------------------------------//

// Handy Global for use in output routines
const char *bpName[4] = { "Static", "Gshare",
                          "Tournament", "Custom" };

int ghistoryBits; // Number of bits used for Global History
int lhistoryBits; // Number of bits used for Local History
int pcIndexBits;  // Number of bits used for PC index
int bpType;       // Branch Prediction Type
int verbose;

//------------------------------------//
//      Predictor Data Structures     //
//------------------------------------//

//
//TODO: Add your own Branch Predictor data structures here
//
// Global history register
uint32_t global_history;

// Pattern History Table
uint8_t *pht;

//------------------------------------//
//        Predictor Functions         //
//------------------------------------//

// Initialize the predictor
//
void
init_predictor()
{
  //
  //TODO: Initialize Branch Predictor Data Structures
  //
  // Initialize Global History Register
  global_history = 0;

  // Initialize Pattern History Table
  // The size of PHT is 2^ghistoryBits
  int pht_size = 1 << ghistoryBits;
  pht = (uint8_t *)malloc(pht_size * sizeof(uint8_t));
  for (int i = 0; i < pht_size; i++) {
    pht[i] = WN;  // Initialize to weakly not taken
  }
}

// Make a prediction for conditional branch instruction at PC 'pc'
// Returning TAKEN indicates a prediction of taken; returning NOTTAKEN
// indicates a prediction of not taken
//
uint8_t
make_prediction(uint32_t pc)
{
  //
  //TODO: Implement prediction scheme
  //

  // Make a prediction based on the bpType
  switch (bpType) {
    case STATIC:
      return TAKEN;
    case GSHARE:{
      // Compute index for PHT using XOR of PC and global history
      uint32_t pht_index = (pc ^ global_history) & ((1 << ghistoryBits) - 1);

      // Predict taken if the counter is WT or ST, not taken otherwise
      if (pht[pht_index] > WN) {
        return TAKEN;
      } else {
        return NOTTAKEN;
      }}
    case TOURNAMENT:
    case CUSTOM:
    default:
      break;
  }

  // If there is not a compatable bpType then return NOTTAKEN
  return NOTTAKEN;
}

// Train the predictor the last executed branch at PC 'pc' and with
// outcome 'outcome' (true indicates that the branch was taken, false
// indicates that the branch was not taken)
//
void
train_predictor(uint32_t pc, uint8_t outcome)
{
  //
  //TODO: Implement Predictor training
  //
    if (bpType == GSHARE) {
    // Compute index for PHT using XOR of PC and global history
    uint32_t pht_index = (pc ^ global_history) & ((1 << ghistoryBits) - 1);

    // Update the pattern history table based on outcome
    if (outcome == TAKEN) {
      if (pht[pht_index] < ST) {
        pht[pht_index]++;
      }
    } else {
      if (pht[pht_index] > SN) {
        pht[pht_index]--;
      }
    }

    // Update global history register
    global_history = ((global_history << 1) | outcome) & ((1 << ghistoryBits) - 1);
  }
}
