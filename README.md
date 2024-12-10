# Frogger-like Console Game in C

## Overview

This is a classic Frogger-style arcade game implemented in C using the ncurses library. The player controls a frog trying to navigate through a busy road, avoiding cars and obstacles while attempting to reach the top of the screen.

## Features

### Gameplay Mechanics
- Multi-level progressive difficulty
- Timer-based gameplay
- Obstacle and collision management
- Bonus car mechanic
- Leaderboard system

### Game Elements
- Cars with varying speeds and types
- Randomly placed trees as obstacles
- Bonus cars that provide extra time
- Friendly cars with unique movement patterns

## Technical Details

### Core Components
- Written in C
- Uses ncurses for terminal-based graphics
- Modular design with separate header files for different game mechanics

## Input Controls
- `W`: Move Up
- `S`: Move Down
- `A`: Move Left
- `D`: Move Right
- `Q`: Quit Game
- `L`: Show Leaderboard

## Technical Architecture

### Files
- `game.c`: Main game logic
- `cars_trees.h`: Car and tree spawning/management
- `structures.h`: Game data structures
- `window.h`: Window initialization
- `fileprocessing.h`: Leaderboard management


## Build and Run

### Prerequisites
- GCC compiler
- ncurses library

### Compilation
```bash
# Compile the game
gcc -o frogger game.c -lncurses

# Run the game
./frogger
```


## Performance Considerations
- Efficient memory management
- Modular design
- Low computational overhead
- Minimal dynamic memory allocation

## Debugging and Logging
- Use GDB for debugging
- Compile with `-g` flag for symbol information
- Add print statements in critical sections if needed

## Future Improvements
- Enhanced graphics
- Sound effects
- More obstacle types
- Power-up system
- Difficulty settings

## Known Limitations
- Terminal size dependency
- Color support varies by terminal
- Performance on very large terminals


## Acknowledgments
- Inspired by classic Frogger arcade game
- Built as a programming exercise in C
- Uses ncurses for terminal graphics
