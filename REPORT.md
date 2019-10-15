# ECS 150 Project 1
### by Shayan Mandegarian and Faraz Ahamed

The main structure of our program consists of the main function, which parses
input in a while loop, deals with the processes, and calls helper functions.
There is also a struct called "input" which stores information about what the
user inputted. All of this is located in a while (1) loop which keeps the
program running repeatedly until the user purposely exits it.

Our main sources of help for our implementation came from the lecture pages,
various questions on StackOverflow and Piazza, as well as the gnu.org manual
pages linked in the project description, and other varied programming help
websites like tutorialspoint.com and geeksforgeeks.org.

## Phase 2


## Phase 3
This phase is where the while loop that parses the input, and the input struct
were created, which are part of the vital structure of the program. That while
loop uses cmd[0] != NULL as its conditional, where cmd[0] is defined by
strtok(userIn->input, ' '), then strtok(NULL, ' ') from there on. userIn is the
instance of the input struct that relates to the current command from the user.
This loop also sets multiple flags that controls the flow of the program,
including "cont", "background", and "error" which tell the program to either
skip the fork part of the program, or that the current command is a background
command.

## Phase 4


## Phase 5
For input redirection, the program has two main steps.
First, if the "<" character is detected, it calls the inRedirect function,
which separates what came before the character, and what came after, to be
able to distinguish between the command, and the file. This also sets a flag
which will break out of the parsing while loop, since the function already
parsed it. That same flag also changes the way the processes will be handled,
which calls open, dup2, etc. As for the error handling, the inRedirect function
checks for the existence of a given input file, while the ability to open the
given file is checked when trying to open it.

Unfortunately, the fact that there may or may not be spaces around the "<"
character was a major roadblock for us, and ultimately we could not solve it
for every possible case, so in the case where there is NO space around it, ie
"grep toto<file.txt", it will not work, but if there is a space before or after
it, it will function.

## Phase 6


## Phase 7


## Phase 8
In order to allow for background functions, we added a for loop to the start of
the main while (1) loop. This for loop cycles through all of the aforementioned
input structs in the array that background commands are stored in (will be
explained later). For each of those background commands, waitpid is called with
the PID that the function belongs to with the WNOHANG option, which allows the
program to continue on if the PID is still unfinished, if waitpid returns that
the process had finished, it will display the "+ completed..." message for it,
then continue on with the program with the "sshell $" prompt.

Another major aspect of this phase was correctly parsing the input to correctly
detect the "&" character, which again was very challenging. The case where "&"
is surrounded by spaces was easy to implement with strtok(NULL, " "), but the
case that "&" is right next to the command, ie "sleep 1&" was harder, which
entailed each word tokenized by strtok have its final character be compared to
"&", which happens near the beginning of the while (1) loop before the word can
be added to the arguments section of the input struct. Unlike with input
redirection, spacing does not affect performance in this phase.

Finally, in order to get the order of the "+ completed..." messages when there
are both background commands and non-background commands running at the same
time, like in the tester script provided, there is a flag that is set that will
tell the program to run the same for loop mentioned above before calling
waitpid again at the end of the main while (1) loop.
