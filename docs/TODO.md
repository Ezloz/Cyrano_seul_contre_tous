



# Urgent

- Save TurnQueue
- Make LoadCharacterStatsMenu() pretty
- Add Options
- Add Panache (Special Attack)

# When possible
- Add complexPath (maybe even better AI ?)
- Add AV next to character
- Add Cinematics
- Add Battle Animations
- terrain propreties
- Add archer enemies
- Add Christian 
- Add Roxane

# List of things to rework

- walkableGrid. Absolutely atrocious
- Menus. Inputs are poorly handled
- Animations. Use coroutine
- Map.ProcessInputs(). Way too long

Search for comments "REWORK" in code

# Questions to ponder about
- stats : float or int ? Decide. (For example : range is int for sure, but speed ?)
- UIManager : Only one ?
- Gamestates. Seems useful for inputs, but for anything else... ? Will it work better with coroutine or worse ?