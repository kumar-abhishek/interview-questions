# Problem

How would you implement `tac(1)` with a simple command line script?

`tac(1)` is a program that reads text line by line and prints the lines in reverse order.

# Example

The following command:

```
$ printf "Hello\nWorld\nReversed\n" | tac
```

Should print:

```
Reversed
World
Hello
```

# Solution

There are many possibilities; one of them is to use good old `awk(1)`. It is very easy (and concise) to do this with `awk(1)`.
