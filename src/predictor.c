//========================================================//
//  predictor.c                                           //
//  Source file for the Branch Predictor                  //
//                                                        //
//  Implement the various branch predictors below as      //
//  described in the README                               //
//========================================================//
#include <stdio.h>
#include "predictor.h"
#include <string.h>
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
uint8_t *g_pht;

// this is local history table, it contains mutiple local pattern history table for different local branches
uint32_t *lht;

// local pattern table 
uint8_t *l_pht;

uint8_t *selector;

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
  int g_pht_size = 1 << ghistoryBits;
  g_pht = (uint8_t *)malloc(g_pht_size * sizeof(uint8_t));
  for (int i = 0; i < g_pht_size; i++) {
    g_pht[i] = WN;  
  }



  // below is special for TOURNAMENT

  // we should keep 2^pcIndexBits possible instructions, every local instructions will have a corresponding local history
  uint32_t num_ins = 1 << pcIndexBits;
  lht = (uint32_t*)malloc(num_ins*sizeof(uint32_t)); 
  memset(lht, 0, num_ins*sizeof(uint32_t));

  // For all kinds of local history, we should give a result taken/not taken
  uint32_t num_local_history = 1 << lhistoryBits;
  l_pht = (uint8_t*)malloc(num_local_history*sizeof(uint8_t));
  for (int i = 0; i < num_local_history; i++) {
    // As the instruction given, we need to initialize to wn
    l_pht[i] = WN;  
  };

  uint32_t selector_size = 1<<ghistoryBits;
  selector = (uint8_t *)malloc(selector_size*sizeof(uint8_t));
    for (int i = 0; i < selector_size; i++) {
    // as instruction given, this should also be a two bit counter
    selector[i] = WN;  
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
      uint32_t g_pht_index = (pc ^ global_history) & ((1 << ghistoryBits) - 1);

      // Predict taken if the counter is WT or ST, not taken otherwise
      if (g_pht[g_pht_index] > WN) {
        return TAKEN;
      } else {
        return NOTTAKEN;
      }}
    case TOURNAMENT:
        // local pht
        uint32_t cur_local_instruction = pc & ((1<<pcIndexBits) - 1);
        uint32_t clip_local_history = lht[cur_local_instruction] &((1 << lhistoryBits)-1);
        uint8_t local_prediction = l_pht[clip_local_history] > WN ? TAKEN: NOTTAKEN;

        // global 
        uint32_t g_pht_index = (pc ^ global_history) & ((1 << ghistoryBits) - 1);
        uint8_t global_prediction = g_pht[g_pht_index] > WN ? TAKEN: NOTTAKEN;

        // the selector will choose whom to believe based on the bistory performance 
        return selector[g_pht_index] > WN ? global_prediction : local_prediction;

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
      uint32_t g_pht_index = (pc ^ global_history) & ((1 << ghistoryBits) - 1);

      // Update the pattern history table based on outcome
      if (outcome == TAKEN) {
        if (g_pht[g_pht_index] < ST) {
          g_pht[g_pht_index]++;
        }
      } else {
        if (g_pht[g_pht_index] > SN) {
          g_pht[g_pht_index]--;
        }
      }

      // Update global history register
      global_history = ((global_history << 1) | outcome) & ((1 << ghistoryBits) - 1);
  }
  
  else if (bpType == TOURNAMENT){
      uint32_t cur_local_instruction = pc & ((1<<pcIndexBits) - 1);
      uint32_t clip_local_history = lht[cur_local_instruction] &((1 << lhistoryBits)-1);
      uint32_t g_pht_index = (pc ^ global_history) & ((1 << ghistoryBits) - 1);

      // get the last round prediction before updating their value
      uint8_t local_prediction = l_pht[clip_local_history] > WN ? TAKEN : NOTTAKEN;
      uint8_t global_prediction = g_pht[g_pht_index] > WN ? TAKEN : NOTTAKEN;

      // add the new history to postfix and only keep the useful history part
      lht[cur_local_instruction] = ((lht[cur_local_instruction] << 1) | outcome) & ((1 << lhistoryBits) - 1);

      // update the local based on outcome
      if (outcome == TAKEN){l_pht[clip_local_history] = l_pht[clip_local_history] < ST ? l_pht[clip_local_history]+1 : l_pht[clip_local_history];}
      else{l_pht[clip_local_history] = l_pht[clip_local_history] > SN ? l_pht[clip_local_history]-1 : l_pht[clip_local_history];}


      // Same logic for update the global pht
      if (outcome == TAKEN) {if (g_pht[g_pht_index] < ST) {g_pht[g_pht_index]++;}} 
      else {if (g_pht[g_pht_index] > SN) {g_pht[g_pht_index]--;}}
      global_history = ((global_history << 1) | outcome) & ((1 << ghistoryBits) - 1);

      // update the selector only when local and global gives different result
      if (local_prediction != global_prediction){
          if(local_prediction==outcome){ selector[g_pht_index] = selector[g_pht_index] > SN ? selector[g_pht_index] - 1: selector[g_pht_index];}
          else{selector[g_pht_index] = selector[g_pht_index] < ST ? selector[g_pht_index] + 1: selector[g_pht_index];}
      }
  }
}
