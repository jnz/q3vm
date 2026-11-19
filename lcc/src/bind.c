#include "c.h"
extern Interface nullIR;
extern Interface bytecodeIR;
extern Interface bytecode64IR;
Binding bindings[] = {
	{ "null",          &nullIR },
	{ "bytecode",      &bytecodeIR },
	{ "bytecode64",    &bytecode64IR },
	{ NULL,            NULL },
};
