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

// Perceptron Table
int **perceptronTable;

// Global history register
uint32_t global_history;

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

  // below is for perceptron
  // Initialize Perceptron Table
  uint32_t numPerceptrons = 1 << pcIndexBits;
  perceptronTable = (int **)malloc(numPerceptrons * sizeof(int *));
  for (int i = 0; i < numPerceptrons; i++) {
    perceptronTable[i] = (int *)malloc((ghistoryBits + 1) * sizeof(int)); // +1 for bias weight
    for (int j = 0; j <= ghistoryBits; j++) {
      perceptronTable[i][j] = 0; // Initialize weights to 0
    }
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
    case TOURNAMENT:{
        // local pht
        uint32_t cur_local_instruction = pc & ((1<<pcIndexBits) - 1);
        uint32_t clip_local_history = lht[cur_local_instruction] &((1 << lhistoryBits)-1);
        uint8_t local_prediction = l_pht[clip_local_history] > WN ? TAKEN: NOTTAKEN;

        // global 
        uint32_t g_pht_index = (pc ^ global_history) & ((1 << ghistoryBits) - 1);
        uint8_t global_prediction = g_pht[g_pht_index] > WN ? TAKEN: NOTTAKEN;

        // the selector will choose whom to believe based on the bistory performance 
        return selector[g_pht_index] > WN ? global_prediction : local_prediction;
    }
    case CUSTOM:{
      // Compute perceptron index
      uint32_t numPerceptrons = 1 << pcIndexBits;
      int perceptronIndex = pc % numPerceptrons;

      // Compute the dot product of GHR and perceptron weights
      int sum = perceptronTable[perceptronIndex][0]; // bias weight
      for (int i = 1; i <= ghistoryBits; i++) {
        if (((global_history >> (i - 1)) & 1) == 1) {
          sum += perceptronTable[perceptronIndex][i];
        } else {
          sum -= perceptronTable[perceptronIndex][i];
        }
      }

      // Prediction: taken if sum is greater than 0, not taken otherwise
      return (sum >= 0) ? TAKEN : NOTTAKEN;
    }
      
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
  else if (bpType == CUSTOM){
    uint32_t numPerceptrons = 1 << pcIndexBits;
    // Convert outcome to perceptron target -1 or 1
    int t = outcome == TAKEN ? 1 : -1;
    
    // Compute perceptron index
    int perceptronIndex = pc % numPerceptrons;
    
    // Compute the dot product of GHR and perceptron weights
    int y_out = perceptronTable[perceptronIndex][0]; // bias weight
    for (int i = 1; i <= ghistoryBits; i++) {
        int xi = ((global_history >> (i - 1)) & 1) ? 1 : -1;
        y_out += xi * perceptronTable[perceptronIndex][i];
    }
    
    // Train if the output sign does not match the actual outcome or if it's not confident
    int theta = 1.93 * ghistoryBits + 14;
    if (y_out * t <= 0 || abs(y_out) <= theta) {
        // Update the weights
        perceptronTable[perceptronIndex][0] += t; // Update bias weight
        for (int i = 1; i <= ghistoryBits; i++) {
            int xi = ((global_history >> (i - 1)) & 1) ? 1 : -1;
            perceptronTable[perceptronIndex][i] += t * xi;
        }
    }
    
    // Update global history to include the most recent outcome
    global_history = ((global_history << 1) | (outcome == TAKEN ? 1 : 0)) & ((1 << ghistoryBits) - 1);
  }
}
