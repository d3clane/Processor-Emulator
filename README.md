# Processor-Emulator

## Installation 

```
git clone https://github.com/d3clane/Processor-Emulator.git
make -f asmMake
make -f spuMake
```

## Description 

The project consists of two programs:
- Assembler
- SPU (software processor unit) 

Assembler converts human-readable text format into machine code (binary file with ints inside of it). After that this code can be executed by SPU. SPU is built as stack machine.

In this project I have used some kind of auto generation of code because there a lot in common between different programs - assembler and spu (for example - instructions). It is implemented by using [define file](Common/Commands.h) that can create different code based on how is define defined.  

SPU supports registers, stack and RAM.

## Assembler 

Assembler code example is available in [file](Assembler/code.txt). It is auto generated code of program solving quadratic equations, generated by backend of my [programming language](https://github.com/d3clane/ProgrammingLanguage).

Assembler understand these types of operations:
- Register based operations - push rax / pop rax
- Stack based operations - add / sub / mul etc
- Jump commands - jmp, je, jg, etc
- RAM based operations - push [5](means push on stack from memory adr 5), pop [5](means pop from stack to memory adr 5).
- Mixed: push [rax + 5], push [rax]
- in / out / outc to work with input / output from stdio. Outc puts char with it's ASCII code.

The fact that operations use different arguments is saved in operation id in machine code. It is added using bitwise or with [constants](/Common/Common.h). Also, every command in machine code is fixed-length; however, there could be different sizes because of arguments. For example, "push 5" need to allocate memory in machine code for saving argument data, whereas the "add" instruction does not.

Assembler supports labels (used for jumps) and procedures (functions).

Assembler uses two-pass compilation in order to precalculate labels and functions addresses and use their addresses for jumps and calls.

Also, assembler adds signature to the file beginning so that any SPU could check it and compare with it's own signature. It is done because machine code designed for my SPU shouldn't be executed on others to avoid bugs.

## Spu

Spu checks signature from machine code and begins executing code if everything is correct. It has registers, ram, two stacks and ip pointer.

Ip pointer is instruction pointer. According to the naming - it is used to understand which instruction to execute next; could changed by jumps and after executing each instruction.

Registers are supposed to be a faster storage for data than ram, but, unfortunately, in SPU they are equal in performance. Ram is just a fixed-size array with data in it.

First stack is used for data, all arithmetic and logic operations are made on stack because it is a stack based machine. And another stack is used for functions return addresses as it is in Harvard architecture.

## Auto generation of code

The main idea behind auto generation is that the same instruction is used multiple times in assembler and spu. The instruction code is written in assembler files and spu files and there in lies the problem - if I would like to add another instruction or fix existing ones I have to watch through the files and change something in each of them. This sequences of actions increases probability of forgetting something and therefore there could be bugs in my program. Also, there are a lot in common between different instructions so I may have to do copy-paste every time I want to add something and I want to avoid it.

The solution is to create one file with all instructions data and update it instead of other files. There could be different ways of doing this:
- Creating another program on any language that will auto generate code in file.
- Using defines to do the same.

These solutions are pretty the same because preprocessor language is also a language and could be used for generating code.

Now as we sorted out our problem, let's move on to the implementation. I have created file [Common/Commands.h](Common/Commands.h) in which for every instruction I use define "DEF_CMD(NAME, ENUM_ID, USE_LABELS, SPU_CODE, SWITCH_CASE_SPU_CALL, ...)" (there could be more arguments). In this file I have defined code that some parts of the main code will use in the future. For example, for generating if/else construct when parsing commands that contains labels I have done this:

![generate labels](https://github.com/d3clane/Processor-Emulator/blob/main/ReadmeAssets/imgs/AutoGeneration.png)

In this case I compare only instructions that use labels (it is written inside of commands.h, argument USE_LABELS) with my instruction. Instead of writing large if/else construct I have written a few code lines, so auto generation also saves a lot of time.

Unfortunately, such code generation dramatically reduces the level of code clarity and readability and therefore it's better to think twice before applying this trick.
