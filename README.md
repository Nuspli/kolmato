# kolmato
C port and continuation of my Javascript chess engine

## Features
- "magic bitboards" for quick move generation
- piece square tables for opening and endgame aswell as some basic strategy and king safety evaluation
- custom opening book based on the first few moves of almost 4 million games played on lichess (2200+ elo), downloaded from the [lichess elite database](https://database.nikonoel.fr/) (until to May 2022). you can use the included book or one where I parsed the first 20 moves of each game. sadly that one exeeds githubs file size limit of 100mb and can only be downloaded [here](https://drive.google.com/file/d/1nge9umm9D4zpN-gmNQSVIQauuqeR-L26/view?usp=sharing)
- zobrist hashing for the transposition tables
- iterative deepening using a recursive negamax algorithm with a quiescence search
- various pruning methods like move ordering, null move pruning, delta pruning, evaluation pruning, razoring, futility pruning, aspiration windows and late move reductions
- piece list for faster move ordering and move making
- passes [these](https://www.chessprogramming.org/Perft_Results) perft tests

## compiled using (gcc)
> `gcc -Ofast bitboards.c book.c check.c counters.c eval.c fen.c magic.c main.c move.c moveGeneration.c parser.c print.c random.c search.c tables.c -o kolmato`

for a list of all commands
> `kolmato.exe`
## 
![kolmato](https://github.com/Nuspli/kolmato/assets/108233076/ad8dc935-8edb-4c8e-b989-ef31d37f0b0d)

### TODO

- [ ] UCI support
- [ ] profile code and improve the speed
- [ ] fix check extensions interfering with the transposition table and it behaving weird in some endgames
