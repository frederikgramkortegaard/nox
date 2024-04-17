## Nox - A low-level programming language written in idiomatic C++
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://github.com/frederikgramkortegaard/nox/blob/master/LICENSE)
[![unstable](http://badges.github.io/stability-badges/dist/unstable.svg)](http://github.com/badges/stability-badges)
[![CodeQL](https://github.com/frederikgramkortegaard/nox/workflows/CodeQL/badge.svg)](https://github.com/frederikgramkortegaard/nox/actions?query=workflow%3ADependency+Review)



---
## Using the Nox compiler
To use the compiler, the following command can be run:
```bash
$ ./path/to/compiler ./path/to/file.nox
```

From here, the newly created `output.s` file can be constructed into a runnable by using `gcc`. Here it is important to note that the `-no-pie` flag for GCC is required when using strings.

We advice the reader to test the following examples:
```bash
$ ./build/compiler  ./examples/hello_world.nox
$ gcc -no-pie output.s
$ ./a.out
```

## Code Samples
### Stringified Enum's
Converting Enum's to their respective string representations is a pattern which occurs often in many projects. In the C++ code for Nox itself, this is handled by using `maps`. Nox has built-in compile-time support for enum stringification by using the directive `#enum_strings`, and works as follows.
```c++
#enum_strings

enum Colors {
  Red,
  Green,
  Blue
}

func int main() {
    my_color: enum Colors = Colors.Green;
    print("My color is %s\n", my_color.str())
}
```
```bash
>> My color is Green
```
### Miscellaneous
- Built-in `string` types similar to C++
- Default Values for Struct Fields
- Compile-time logic via. directives: `#some_directive`
- Functions as first-class citizens

## Requirements
When compiling Nox from source, it is dependent on the precense of [boost version 1.84.0 ](https://www.boost.org/users/history/version_1_84_0.html). 

## Compiling Nox from Source
It is possible to compile Nox from source, by navigating to the `./src` folder and running `make`. Verbose output from the compilation process such as Abstract Syntax Tree structures or the code generation process can be exposed by using `make debug` instead. 

```bash 
$ cd ./src && make
```
