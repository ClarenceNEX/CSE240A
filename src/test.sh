#!/bin/bash

# Path to your predictor binary
PREDICTOR_BIN="./predictor"

# Check if an input file was provided
if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <trace_file>"
    exit 1
fi

# The trace file is the first argument
TRACE_FILE="$1"

# Helper function to run predictor with arguments
run_predictor() {
    echo "Running: $PREDICTOR_BIN $@ $TRACE_FILE"
    $PREDICTOR_BIN $@ $TRACE_FILE
    echo "-----------------------------------------------------"
}

# Test different branch prediction schemes
run_predictor --static
run_predictor --gshare:8
run_predictor --tournament:8:8:8
run_predictor --custom:32:32:32

echo "Tests completed."
