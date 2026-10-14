#!/bin/bash
#
# SHELL PROGRAM (b): Generate the Fibonacci series using a for loop
# ---------------------------------------------------------------------------------
# The Fibonacci series starts 0, 1 and each later term is the sum of
# the two before it: 0 1 1 2 3 5 8 13 21 ...
#
# Usage: ./fibonacci_for.sh [N]     (prints the first N terms)
#        ./fibonacci_for.sh         (prompts for N interactively)

if [ $# -ge 1 ]; then
    n="$1"
else
    read -p "How many Fibonacci terms to generate? " n
fi

if ! [[ "$n" =~ ^[0-9]+$ ]]; then
    echo "Error: \"$n\" is not a valid non-negative integer."
    exit 1
fi

if [ "$n" -eq 0 ]; then
    echo "Fibonacci series (0 terms): (nothing to print)"
    exit 0
fi

a=0     # term i-2
b=1     # term i-1
series=""

for (( i=0; i<n; i++ )); do
    if [ $i -eq 0 ]; then
        next=$a
    elif [ $i -eq 1 ]; then
        next=$b
    else
        next=$(( a + b ))
        a=$b
        b=$next
    fi
    series="$series $next"
done

echo "Fibonacci series ($n terms):$series"
exit 0

# Sample runs:
#   $ ./fibonacci_for.sh 10
#   Fibonacci series (10 terms): 0 1 1 2 3 5 8 13 21 34
#
#   $ ./fibonacci_for.sh 1
#   Fibonacci series (1 terms): 0
#
#   $ ./fibonacci_for.sh 0
#   Fibonacci series (0 terms): (nothing to print)
#
#   $ ./fibonacci_for.sh
#   How many Fibonacci terms to generate? 7
#   Fibonacci series (7 terms): 0 1 1 2 3 5 8
