Coding Style Guidelines
=======================

"the code is more what you'd call guidelines than actual rules"
                                                - Cap'n Barbossa

Principles
----------

* Readable
* Maintainable
* Pragmatic
* Elegant
* Progressive
* Respectful

The compiler doesn't care about style at all. It's purely for human readability. That is, how easy is the code to understand (correctly), and how easily can I make changes without introducing bugs. So try to maximize readability instead of just blindy following some rules. Every style choice affects readability in some way, so seek to be objective even when things appear only subjective. Also keep in mind that your code will be used for many years. Issues caused by technical limitations of outdated tools will disappear fast. That said, don't just go rewriting someone else's code to match the latest style consensus, unless they want you to.

Formatting
----------

### Braces

Use [Allman style](https://en.wikipedia.org/wiki/Indent_style#Allman_style). That is, put the opening brace on the next line, aligned with the control statement, and align the closing brace on the same indentation level. It makes it easier to read complex control flow. The criticism that it wastes lines is less relevant on today's high resolution monitors.

Preferably put braces around single-line statements as well. This makes it less prone to introducing a bug when adding more statements and forgetting to add braces. In some rare cases it is very unlikely to add another statement, and braces are distracting, so they can be omitted.

### Indentation and alignment

Tabs for indentation, spaces for alignment. This allows anyone to used their preferred indentation width. This preference can be greatly influenced by people's previous experience with code bases, the font that is used, or the medium (display, presentation, printout, book).

Tabs are indivisible, so they are easy to navigate and select correctly.

Indent switch cases to the same depth as the switch keyword. They are code labels which should stand out sufficiently just like other control flow statements.

### Line length

Anything sensible. Some code or comments are of lesser importance and should be allowed to flow out of view instead of being forced onto the next line(s). Especially at several indentation levels deep the code starts to look cramped when a hard limit is imposed, and developers waste time trying to fit it.

That said, be mindful that for presentations, blogs, and books long lines often need to be edited.

### Spacing

Don't put a space after a keyword unless syntactically required. In particular, there's no need for a space in `if(condition)`. Code that does put a space there is usually very inconstent about its use in other constructs such as `for`, `switch`, `sizeof`, etc. The argument that it makes them stand out from function calls has become moot due to syntax coloring in practically any editor. The recommended Allman style bracing also makes confusing control statements for function calls very unlikely.

Spaces are useful for improving the readability of expressions which can be logically separated, like after each comma in an argument list. In `if (condition)` it causes an awkward separation between closely related parts. Two or more spaces between function arguments wouldn't seem terribly off (and is often done for matrix element alignment), but for a conditional statement it would seem to make the condition float away.

Put the `*` of pointer declarations with the variable name. C/C++ parses things right-to-left before variable names. So `int *&x;` is easier to read correctly as a reference to a pointer than `int*& x`.