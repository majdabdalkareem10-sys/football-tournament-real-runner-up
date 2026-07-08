# Football Tournament Real Runner-Up

A C program that simulates a knockout football tournament and determines the real runner-up.

The real runner-up is selected by running a secondary tournament between all teams that lost directly against the tournament champion.

This project was originally developed during the 2024/2025 Data Structures course and was later reviewed and refactored for portfolio documentation.

## Features

- Supports multiple tournament test cases
- Uses a circular array queue
- Uses a linked-list stack
- Applies different winner rules for odd and even rounds
- Handles automatic qualification when the number of teams is odd
- Tracks all teams defeated directly by the champion
- Runs a secondary tournament to determine the real runner-up
- Supports positive and negative team IDs
- Accepts spaces or commas between input values
- Handles dynamic memory safely

## Data Structures Used

### Circular Queue

The teams participating in each tournament round are stored in a circular array queue.

The queue is used to:

- Store teams in their current order
- Remove teams participating in each match
- Store winners for the next round
- Move an automatically qualified team to the front of the next round

### Linked-List Stack

A stack is used to store the teams that lost directly against the champion.

The stack reverses their order so that the team that lost in the final round enters the secondary tournament first.

## Tournament Rules

- During odd-numbered rounds, the team with the smaller ID wins.
- During even-numbered rounds, the team with the larger ID wins.
- If the number of teams is odd, the last team qualifies automatically.
- The automatically qualified team is placed at the beginning of the next round.
- Input ends when `-9999` is entered on a separate line.

## Project Files

```text
.
├── main.c
├── README.md
└── .gitignore