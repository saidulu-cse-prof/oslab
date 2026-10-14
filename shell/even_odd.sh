#!/bin/bash
#
# SHELL PROGRAM (a): Check whether a given number is EVEN or ODD
# ---------------------------------------------------------------------------------
# Demonstrates the if-elif-else-fi conditional construct.
#
# The number can be supplied either as a command-line argument
# ( ./even_odd.sh 17 ) or, if no argument is given, the script will
# prompt for it interactively with `read`.
#
# Logic: n % 2 == 0  -> EVEN
#        otherwise    -> ODD
# (works correctly for negative numbers too, e.g. -4 % 2 = 0 in bash)
#
# Run: ./even_odd.sh [number]

if [ $# -ge 1 ]; then
    num="$1"
else
    read -p "Enter a number: " num
fi

# Validate: must be an optional leading '-' followed by one or more digits.
if ! [[ "$num" =~ ^-?[0-9]+$ ]]; then
    echo "Error: \"$num\" is not a valid integer."
    exit 1
fi

remainder=$(( num % 2 ))

if [ $remainder -eq 0 ]; then
    echo "$num is EVEN."
elif [ $remainder -eq 1 ] || [ $remainder -eq -1 ]; then
    # bash's % can return -1 for a negative odd number (e.g. -7 % 2 = -1),
    # so both +1 and -1 mean "odd" here.
    echo "$num is ODD."
else
    echo "Could not determine - unexpected remainder $remainder."
fi

exit 0

# Sample runs:
#   $ ./even_odd.sh 17
#   17 is ODD.
#
#   $ ./even_odd.sh 42
#   42 is EVEN.
#
#   $ ./even_odd.sh -7
#   -7 is ODD.
#
#   $ ./even_odd.sh
#   Enter a number: 8
#   8 is EVEN.
#
#   $ ./even_odd.sh abc
#   Error: "abc" is not a valid integer.
