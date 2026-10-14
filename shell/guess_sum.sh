#!/bin/bash
#
# SHELL PROGRAM (b): Generate two random numbers and guess whether
#                     their sum is correct or not
# ---------------------------------------------------------------------------------
# Demonstrates the if-elif-else-fi conditional construct.
#
# The script:
#   1. Generates two random numbers in the range 1-100 using bash's
#      built-in $RANDOM (a pseudo-random integer 0-32767 each time it
#      is referenced), reduced to 1-100 with the modulo trick
#      $(( RANDOM % 100 + 1 )).
#   2. Shows the two numbers and asks the user to GUESS their sum.
#   3. Compares the guess to the real sum with if-elif-else-fi and
#      reports whether the guess was exactly correct, too high, or
#      too low (and by how much).
#
# Run: ./guess_sum.sh

num1=$(( RANDOM % 100 + 1 ))
num2=$(( RANDOM % 100 + 1 ))
actual_sum=$(( num1 + num2 ))

echo "Random number 1: $num1"
echo "Random number 2: $num2"
read -p "Guess their sum: " guess

# Validate the guess is an integer before comparing.
if ! [[ "$guess" =~ ^-?[0-9]+$ ]]; then
    echo "Error: \"$guess\" is not a valid integer."
    exit 1
fi

if [ "$guess" -eq "$actual_sum" ]; then
    echo "Correct! $num1 + $num2 = $actual_sum. Well guessed!"
elif [ "$guess" -gt "$actual_sum" ]; then
    diff=$(( guess - actual_sum ))
    echo "Not quite - your guess was too HIGH by $diff."
    echo "The actual sum of $num1 + $num2 is $actual_sum."
else
    diff=$(( actual_sum - guess ))
    echo "Not quite - your guess was too LOW by $diff."
    echo "The actual sum of $num1 + $num2 is $actual_sum."
fi

exit 0

# Sample runs (random numbers differ every time):
#   $ ./guess_sum.sh
#   Random number 1: 37
#   Random number 2: 58
#   Guess their sum: 95
#   Correct! 37 + 58 = 95. Well guessed!
#
#   $ ./guess_sum.sh
#   Random number 1: 12
#   Random number 2: 9
#   Guess their sum: 30
#   Not quite - your guess was too HIGH by 9.
#   The actual sum of 12 + 9 is 21.
#
#   $ ./guess_sum.sh
#   Random number 1: 80
#   Random number 2: 65
#   Guess their sum: 100
#   Not quite - your guess was too LOW by 45.
#   The actual sum of 80 + 65 is 145.
