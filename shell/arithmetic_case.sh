#!/bin/bash
#
# SHELL PROGRAM (a): Arithmetic operations using a case statement
# ---------------------------------------------------------------------------------
# Demonstrates the case...esac construct: it matches one value against
# several patterns (here, an operator symbol) and runs only the
# matching branch - a cleaner alternative to a long if-elif-else chain
# when you're testing the same variable against many fixed values.
#
# Usage: ./arithmetic_case.sh <num1> <operator> <num2>
#        operator is one of: + - \* / %
#   e.g. ./arithmetic_case.sh 12 + 5
#        ./arithmetic_case.sh 12 \* 5     (escape * so the shell doesn't glob it)
# If no arguments are given, it will prompt for them interactively instead.

if [ $# -ge 3 ]; then
    num1="$1"
    op="$2"
    num2="$3"
else
    read -p "Enter first number: " num1
    read -p "Enter operator (+ - * / %): " op
    read -p "Enter second number: " num2
fi

# Validate both operands are integers.
if ! [[ "$num1" =~ ^-?[0-9]+$ ]] || ! [[ "$num2" =~ ^-?[0-9]+$ ]]; then
    echo "Error: both operands must be integers."
    exit 1
fi

case "$op" in
    "+")
        result=$(( num1 + num2 ))
        echo "$num1 + $num2 = $result"
        ;;
    "-")
        result=$(( num1 - num2 ))
        echo "$num1 - $num2 = $result"
        ;;
    "*")
        result=$(( num1 * num2 ))
        echo "$num1 * $num2 = $result"
        ;;
    "/")
        if [ "$num2" -eq 0 ]; then
            echo "Error: division by zero is not allowed."
            exit 1
        fi
        result=$(( num1 / num2 ))
        echo "$num1 / $num2 = $result (integer division)"
        ;;
    "%")
        if [ "$num2" -eq 0 ]; then
            echo "Error: modulo by zero is not allowed."
            exit 1
        fi
        result=$(( num1 % num2 ))
        echo "$num1 % $num2 = $result"
        ;;
    *)
        echo "Error: unknown operator \"$op\". Use one of + - * / %"
        exit 1
        ;;
esac

exit 0

# Sample runs:
#   $ ./arithmetic_case.sh 12 + 5
#   12 + 5 = 17
#
#   $ ./arithmetic_case.sh 12 \* 5
#   12 * 5 = 60
#
#   $ ./arithmetic_case.sh 12 / 5
#   12 / 5 = 2 (integer division)
#
#   $ ./arithmetic_case.sh 12 % 5
#   12 % 5 = 2
#
#   $ ./arithmetic_case.sh 12 / 0
#   Error: division by zero is not allowed.
#
#   $ ./arithmetic_case.sh 12 ^ 5
#   Error: unknown operator "^". Use one of + - * / %
