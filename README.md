# Optimizing BrainFuck MultiCompiler

## Optimizations

List of performing optimiations:
* Sequences like ++++... are translating into +n

## Targets

OBFMC Can generate 8086 and i386 machine code. It allows to build apps for:
* 16 bit DOS (.com)
* 32 bit KolibriOS (.kex)

OBFMC also supports translation into another programming languages:
* C

## Why Multi?

Because you can generate code for whatever supported platform in whatever platform where the compiler works. In one execution! (```obfmc hello.bf --kos32 --dos16 --c```)
