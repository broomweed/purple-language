Purple
======

Purple (supposedly short for PURPoseless LanguagE) is a slightly esoteric, Turing-complete, stack-based programming language written in C. Well, this implementation is, at least. It could theoretically be written in something else.

Syntax
------

Each command must be on its own line. Comments start with #. Lines can be indented as much as they want.

The general syntax of a command is:
```
command arg1, arg2, arg3...
```
All arguments for commands are, of course, optional. Commands are parsed from right to left in the order that they are seen. Each argument (from right to left) is pushed onto the stack, and then the command is executed. The exceptions to this are the branching syntax, which is a special case.

ASCII character values may be pushed onto the stack by enclosing a sequence of characters in quotes. For example, `"hello"` will push 111, 108, 108, 101, and 104 onto the stack, in that order. You can also interpose them, for character escapes: `"hello", 10` is the same as previously, but a newline (ASCII 10) will be pushed onto the stack first.

Stacks
------

The program has two stacks, the "main stack" and the "auxiliary stack." The auxiliary stack cannot be interacted with directly; its top element may only be moved to and from the main stack.

Attempting to pop either stack when it is empty will cause the entire program to explode. But in a hopefully non-violent way.

Commands
--------

Unlike many esoteric programming languages, which take delight in restricting the programmer to a small subset of available operands, the Purple language has a rich vocabulary of built-in commands. The trick is combining these to make actually useful programs. (It's still difficult.)

Here is a complete (I believe) list of the basic built-in commands:
```
Command     Effect
---------------------------------------------------------------------------------------
push        Essentially a no-op; pushes its arguments onto the stack.
pop         Pops the top value off of the main stack and throws it away.
add         Adds the top two values on the stack. (If called with one argument, adds
                it to the top value on the stack; if called with two arguments, pushes
                the result of their addition onto the stack.)
neg         Negates the top argument on the stack. (If called with one argument, pushes
                the negative version of its argument onto the stack.)
dup         Duplicates the top item on the stack. (If called with one argument, pushes
                two copies of its argument onto the stack.)
swap        Swaps the top two values of the stack.
print       Prints the ASCII value of the top number on the stack, without popping it.
iprint      Prints the numerical value of the top number on the stack, without popping.
line        Prints a newline, ASCII value 10. Equivalent to writing "print 10" followed
                by "pop."
rand        Replaces the top value on the stack with a random integer between 0 and
                the number currently onto the top of the stack.
stack       For debugging purposes, prints out a representation of the current stack.
give        Pops the top value off the main stack and pushes it onto the auxiliary stack.
take        Pops the top value off the auxiliary stack and pushes it onto the main stack.
end         Ends the program. Letting the program run off the end of the file without
                encountering an END statement will print a warning message, but your
                program will still end, because what else would it do?
```

Control flow
------------

There are no loops! Everything is controlled by GOTOs based on simple conditionals. It's kind of like programming in assembly. And who doesn't like programming in assembly? Boring people, that's who.

Labels are declared by putting a label name on its own line preceded by a space -- for example, `:label`. Label names can contain spaces, or any other special characters you want -- even colons!

These labels can be branched to by the following conditionals:
```
Command     Effect
---------------------------------------------------------------------------------------
goto        Unconditionally branch to the specified label.
ebr         Branch to the specified label if the stack is empty.
nebr        Branch to the specified label if the stack is NOT empty.
zbr         Branch to the specified label if the number on top of the stack is zero.
nzbr        Branch to the specified label if the number on top of the stack is NOT zero.
esk         Skip the next command if the stack is empty.
nesk        Skip the next command if the stack is NOT empty.
zsk         Skip the next command if the number on top of the stack is zero.
nzsk        Skip the next command if the number on top of the stack is NOT zero.
```
When branching to a label name, do not use the colon preceding the name; i.e. the label denoted `:label` would be moved to by the command `goto label`, not `goto :label`. Unless your label name actually starts with a colon, but then it would be labeled `::label`. Why are you naming your labels like this in the first place, is the real question.

The label `start` is magical; program execution will start there instead of at the beginning of the file, if it is specified.

User-defined commands
---------------------

You can also define your own commands! They work almost identically to labels, except not. Rather than a colon, they are prefixed by a tilde (`~`). However, they are like labels in that the tilde'd statement has no effect -- program execution will fall right through it and probably blow up when it hits the `return` statement.

Speaking of the `return` statement, it is pretty much the only special command needed for user-defined commands. The syntax to call a user-defined command is exactly the same as for built-in commands: `command arg1, arg2, ...`). When a user-defined command is called, the current line number is pushed onto a special call stack. When the `return` statement is encountered, execution jumps back to the line on top of the call stack. If the return statement is encountered when the call stack is empty, the program will blow up (what did you expect?)

User-defined commands cannot contain spaces. There are probably some other rules too, but I forget. Did I mention I wrote this a while ago?

Here's some more commands that are useful with functions:
```
Command     Effect
---------------------------------------------------------------------------------------
return      I just explained it to you up there.
argc        Pushes onto the stack the number of arguments of the last command that was
                executed with arguments. This makes it useful for determining the number
                of arguments passed to a user-defined command; if argc is the first
                command in a user-defined command sequence, it will give you the number
                of arguments that your command was called with.
```
There is no way to pass labels to a function, or to branch to a different label based on the stack. Well, you could, but it would be a lot of work, and all the labels would have to be hardcoded. The control-flow constructs are just special that way.

User input
----------

User input can be pushed onto the stack with the `input` command. It basically treats the user's input as a quoted argument to `push`. So, for example, if the user enters "hello", the effect will be the same as executing `push "hello"`. Good luck parsing multi-digit numeric user input!!

There's also a second command, `inpc`, which pushes onto the stack the number of characters read by the most recent input operation.

Turing-completeness
-------------------

You could definitely write a BF implementation in this. I haven't actually tried. Perhaps I'll update this section when I do.
