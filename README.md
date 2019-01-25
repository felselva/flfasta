# Filter by Length FASTA

A simple application named `flfasta` to filter FASTA files based on sequence length (minimum or maximum).

## Compiling

You need a compiler capable of compiling C source-code written in at least the C89 standard. For GCC, the compilation can be done with the command `gcc main.c -o flfasta`.

## Usage

Execute the application `flfasta` to see the usage manual.

```
flfasta (Filter by Length FASTA) version YYYY.MM.DD.micro.
Created by Felipe Ferreira da Silva.

Usage:
  flfasta [1] [2] [3] [4] [5]

  1: Path to input file.
  2: Path to output file.
  3: Minimum length.
  4: Maximum length (0 to ignore maximum length).
  5: Show progress (0 for no, 1 for yes).

File input must be in single-line FASTA format.
```

For example, executing `flfasta input.fasta output.fasta 300 0 1` will generate a file `output.fasta` containing only the sequences from `input.fasta` that are longer than 300. Since the argument of maximum length is `0`, this there will be no filtering based on the maximum length. The application will output the progress because the final argument is `1`.

## License

Copyright (c) 2019 Felipe Ferreira da Silva

This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held liable for any damages arising from the use of this software.

Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
