Report:

---------------------------------------------------------
Team: HMW
Members: Hannah McLaughlin, Mathew Tepley, William Wagner
FSUIDs: hmm14e, mt14d, wcw13
---------------------------------------------------------
Problem Statement:
    Design and implement a basic shell interface that supports input/output redirection, pipes, background
    processing, and a series of built in functions as specified below. The shell should be robust (e.g. it should not crash under any circumstance beyond machine failure).
    The required features should adhere to the operational semantics of the bash shell.

---------------------------------------------------------

Assumptions:
    - No more than 255 characters would be used for input.
    - Redirection and piping would not be mixed within a single command.
    - Can treat commands without whitespace (e.g cdProject1) as a single command.

---------------------------------------------------------
Files:
    - **command**: defines command_group struct and corresponding methods for creation and execution
    - **builtins**: defines the builtin functions (cd, echo, etime, exit, io)
    - **utils**: defines some utility funciton, mainly string and array manipulations
    - **shell**: defines the functions that prompt, parse, and expand command line arguments

---------------------------------------------------------
Usage notes:
    - With regards to background processing, printing the background proceses stdout leads to messy output.
    - Zombie PIDs are reaped right before prompting the user, so to get updates just press enter a bunch of times.

---------------------------------------------------------
