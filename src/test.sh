#!/bin/bash

# Path to your predictor binary
PREDICTOR_BIN="./predictor"

# Output file where the results will be saved
OUTPUT_FILE="test_results.txt"

# Check if at least one input file was provided
if [ "$#" -lt 1 ]; then
    echo "Usage: $0 <trace_file1> [trace_file2] ..."
    exit 1
fi

# Helper function to run predictor with arguments
run_predictor() {
    for TRACE_FILE in "$@"; do
        echo "Running tests on: $TRACE_FILE" | tee -a $OUTPUT_FILE
        echo "-----------------------------------------------------" | tee -a $OUTPUT_FILE

        echo "Running: $PREDICTOR_BIN --static $TRACE_FILE" | tee -a $OUTPUT_FILE
        $PREDICTOR_BIN --static $TRACE_FILE | tee -a $OUTPUT_FILE
        echo "-----------------------------------------------------" | tee -a $OUTPUT_FILE

        echo "Running: $PREDICTOR_BIN --gshare:13 $TRACE_FILE" | tee -a $OUTPUT_FILE
        $PREDICTOR_BIN --gshare:13 $TRACE_FILE | tee -a $OUTPUT_FILE
        echo "-----------------------------------------------------" | tee -a $OUTPUT_FILE

        echo "Running: $PREDICTOR_BIN --tournament:9:10:10 $TRACE_FILE" | tee -a $OUTPUT_FILE
        $PREDICTOR_BIN --tournament:9:10:10 $TRACE_FILE | tee -a $OUTPUT_FILE
        echo "-----------------------------------------------------" | tee -a $OUTPUT_FILE

        echo "Running: $PREDICTOR_BIN --custom:13:13:13 $TRACE_FILE" | tee -a $OUTPUT_FILE
        $PREDICTOR_BIN --custom:13:13:13 $TRACE_FILE | tee -a $OUTPUT_FILE
        echo "-----------------------------------------------------" | tee -a $OUTPUT_FILE
    done
}

# Clear the output file at the beginning of the script
> $OUTPUT_FILE

# Test different branch prediction schemes for each trace file
run_predictor "$@"

echo "Tests completed." | tee -a $OUTPUT_FILE
