# DSA Progress Tracker

A CLI-based tool to log and track daily DSA problem-solving progress.

## Features
- Log daily easy/medium/hard problems solved
- Consecutive-day streak calculation
- 7-day history view
- Configurable daily goal via config.txt
- CSV-based persistent storage with corrupt-row handling
- Input validation and duplicate date prevention

## Build & Run
```bash
g++ -o tracker main.cpp
./tracker
```

## Menu Options
1. Add progress — log today's problems
2. View history — see all entries
3. View last 7 days — recent activity
4. View summary — totals, streak, and daily goal
5. Exit

## Config
Edit `config.txt` to customize:
```
DAILY_GOAL=3
DATA_PATH=progress.csv
```