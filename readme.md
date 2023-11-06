# Hackulator
This calculator in form of a windows console application is ideal for process hackers who need to add offsets together, perform left shift, binary calculations and the like.
You can enter your calculation formula with various operators from the C-language such as + (add), % (modulo), & (bitwise and), << (bitwise left shift) and number formats such as binary, dual, octal, decimal, hexadecimal and ascii.
The result can also be displayed in various number formats such as binary, dual, octal, decimal, hexadecimal and ascii.

Hackulator is highly customizable, for example you can change the operators and number format prefixes to fit your desired string.

The following is the output of the 'help' function.
```
<<<<<<<<<<<<<<<<<<<<<<<<< HELP >>>>>>>>>>>>>>>>>>>>>>>>>
> Enter a calculation or one of the commands below.
> Input is unsigned 64 bit number.
> All input is case insensitive.
> Default input format (when no prefix is given): decimal
> Result order: buodxa
> Result prefix visibility: buox
> Auto saving: On
>
<<<<<<<<<<<<<<<<<<<<<<< COMMANDS >>>>>>>>>>>>>>>>>>>>>>>
> help... Lists this help.
> set_inputformat... Sets the default input format (when no prefix is stated). Parameters: <Input format label (see below "INPUT PREFIXES")>. Example: 'set_inputformat hexadecimal'
> set_inputprefix... Changes the prefix of an input format to your preferred string. Parameters: <Input format label (see below "INPUT PREFIXES")> <string>. Example: 'set_inputprefix octal :w'
> set_operator... Changes an operator to your preferred string. Parameters: <Operator label (see below "OPERATORS")> <string>. Example: 'set_operator exponential ^'
> set_resultorder... Sets the order of results (not listed = not visible). Parameters: <One or more input format short names (see below "INPUT PREFIXES")>. Example: 'set_resultorder dxb'
> set_resultprefixvis... Sets the visibility of prefixes on results (not listed = not visible). Parameters: <One or more input format short names (see below "INPUT PREFIXES")>. Example: 'set_resultprefixvis db'
> set_autosave... Enables or disables auto saving of settings (saves whenever a setting is changed). Parameters: <on/off>. Example: 'set_autosave off'
> save... Manually saves the current settings.
> reset_settings... Resets the settings to their defaults.
> clear... Clears the screen.
> exit... Exits the program.
>
<<<<<<<<<<<<<<<<<<<< INPUT PREFIXES >>>>>>>>>>>>>>>>>>>>
> binary (short b)... :b
> dual (short u)... :u
> octal (short o)... :o
> decimal (short d)... :d
> hexadecimal (short x)... :x
> ascii (short a)... :a
>
<<<<<<<<<<<<<<<<<<<<<< OPERATORS >>>>>>>>>>>>>>>>>>>>>>>
> add... +
> subtract... -
> multiply... *
> divide... /
> exponential... "
> modulo... %
> bitwise_and... &
> bitwise_or... |
> bitwise_xor... ^
> bitwise_left_shift... <<
> bitwise_right_shift... >>
> invert... ~
> revert... $
> bracket_open... (
> bracket_close... )
>
<<<<<<<<<<<<<<<<<< OPERATOR DETAILS >>>>>>>>>>>>>>>>>>>>
> +...  precedence: 2, associativity: left
> -...  precedence: 2, associativity: left
> *...  precedence: 3, associativity: left
> /...  precedence: 3, associativity: left
> "...  precedence: 4, associativity: right
> %...  precedence: 3, associativity: left
> &...  precedence: 1, associativity: left
> |...  precedence: 1, associativity: left
> ^...  precedence: 1, associativity: left
> <<... precedence: 5, associativity: left
> >>... precedence: 5, associativity: left
> ~...  precedence: 0, associativity: none (Only usable as first token)
> $...  precedence: 0, associativity: none (Only usable as first token)
> (...  precedence: 0, associativity: none
> )...  precedence: 0, associativity: none
>
<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>>>>
```

# Examples
## Calculations
Input, prefixed with ':x', you can enter hexadecimal numbers
```
:xDEADBEEF
```
Output, the output is configured to print the result with the following representations: decimal, hexadecimal, binary
```
| 3735928559 | :xDEADBEEF | :b11011110101011011011111011101111 |
```

Input, prefixed with ':x' (hexadecimal), ':b' (binary) and ':u' (dual)
```
:xF0 + 3 * :b1001 & :u33 ^ 5
```
Output, the output is configured to print the result with the following representations: binary, dual, octal, decimal, hexadecimal, ascii
```
| :b00001110 | :u32 | :o16 | 14 | :xE | SO (shift out) |
```

## Commands
Input
```
set_operator exponential ^
```
Result: The operator 'exponential' is now '^'.

Input
```
set_resultorder dxb
```
Result: The output is configured to print the result with the following representations: decimal, hexadecimal, binary

Input
```
set_inputformat hexadecimal
```
Result: The default input format is now hexadecimal, which means you don't have to prefix hexadecimal numbers with ':x' any more.

# Program call parameters
```
-m... Whether to start with a maximized window (optional, 0 = off (default), 1 = on)
-s... Specify the save file path, please include save file name and extension (optional, default: program location --> 'settings.txt')

Example: Hackulator.exe -m 1, -s C:\Users\Bob\Desktop\settings.dat
```

# Thanks
... to these websites for helping me implement the hackulator.
```
https://en.wikipedia.org/wiki/Reverse_Polish_notation
https://en.wikipedia.org/wiki/Shunting_yard_algorithm
https://mathcenter.oxford.emory.edu/site/cs171/shuntingYardAlgorithm/
http://csis.pace.edu/~wolf/CS122/infix-postfix.htm
https://en.wikipedia.org/wiki/Operator_associativity
https://en.wikipedia.org/wiki/Operators_in_C_and_C%2B%2B#Operator_precedence
https://www.scaler.com/topics/postfix-evaluation/
```