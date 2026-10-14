#!/bin/bash
#
# SHELL PROGRAM (c): Sum of the first N natural numbers using a while loop
# ---------------------------------------------------------------------------------
# Computes 1 + 2 + 3 + ... + N by counting up with a while loop, then
# cross-checks the result against the closed-form formula N*(N+1)/2
# as a sanity check that the loop got it right.
#
# Usage: ./sum_natural_while.sh [N]
#        ./sum_natural_while.sh      (prompts for N interactively)

if [ $# -ge 1 ]; then
    n="$1"
else
    read -p "Enter N: " n
fi

if ! [[ "$n" =~ ^[0-9]+$ ]]; then
    echo "Error: \"$n\" is not a valid non-negative integer."
    exit 1
fi

sum=0
i=1
while [ "$i" -le "$n" ]; do
    sum=$(( sum + i ))
    i=$(( i + 1 ))
done

echo "Sum of the first $n natural numbers (via while loop) = $sum"

# Cross-check against the closed-form formula.
formula_sum=$(( n * (n + 1) / 2 ))
if [ "$sum" -eq "$formula_sum" ]; then
    echo "Cross-check with N*(N+1)/2 = $formula_sum -> MATCH"
else
    echo "Cross-check with N*(N+1)/2 = $formula_sum -> MISMATCH!"
fi

exit 0

# Sample runs:
#   $ ./sum_natural_while.sh 10
#   Sum of the first 10 natural numbers (via while loop) = 55
#   Cross-check with N*(N+1)/2 = 55 -> MATCH
#
#   $ ./sum_natural_while.sh 1
#   Sum of the first 1 natural numbers (via while loop) = 1
#   Cross-check with N*(N+1)/2 = 1 -> MATCH
#
#   $ ./sum_natural_while.sh 0
#   Sum of the first 0 natural numbers (via while loop) = 0
#   Cross-check with N*(N+1)/2 = 0 -> MATCH
