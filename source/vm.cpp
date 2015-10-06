#include "BitVM.h"

#include <stdlib.h>

namespace bitvm {

void error(ERROR code, int subcode)
{
    printf("Error: %d [%d]\n", code, subcode);
    die();
}

#define pop() (*--sp)
#define top() (sp[-1])
#define push(v) { uint32_t tmp2 = v; *sp++ = tmp2; }
#define directArg() (op >> 8)
#define nextArg() (*pc++)
#define nextArg24() ((directArg()<<16) | nextArg())

#define applyRefMaskAt(n) \
    if (refmask & (1 << n)) decr(sp[n])

void applyRefMaskCore(uint32_t refmask, uint32_t *sp)
{
    applyRefMaskAt(0);
    applyRefMaskAt(1);
    applyRefMaskAt(2);
    applyRefMaskAt(3);
    if (refmask & 0xf0) {
        applyRefMaskAt(4);
        applyRefMaskAt(5);
        applyRefMaskAt(6);
        applyRefMaskAt(7);
    }
}

#define incrpush(e) { uint32_t tmp2 = e; incr(tmp2); *sp++ = tmp2; }
#define applyRefMask() applyRefMaskCore(nextArg(), sp)



uint32_t *globals;
uint32_t *strings;
int numGlobals, numStrings;

uint32_t exec_function(const uint16_t *pc, uint32_t *args)
{
    uint32_t ver = *pc++;
    check(ver == V1FUNC, ERR_INVALID_FUNCTION_HEADER);

    uint32_t numLocals = *pc++;
    uint32_t stackSize = *pc++;
    uint32_t locals[numLocals];
    uint32_t stack[stackSize];
    uint32_t *sp = stack;

    memset(locals, 0, numLocals * sizeof(uint32_t));

    while (true) {
        uint32_t op = *pc++;
        uint32_t tmp;

        // printf("EXEC: %x\n", op);

        switch (op & 0xff) {
        case NOOP: break;
        case RET0:
            check(sp == stack, ERR_STACK_ONRETURN);
            return 0;
        case RET1:
            check(sp == stack + 1, ERR_STACK_ONRETURN);
            return pop();
        case LDPTR: push((uint32_t)(&bytecode[nextArg24()])); break;
        case LDZERO: push(0); break;
        case LDCONST8: push(directArg()); break;
        case LDCONST16: push(nextArg()); break;
        case LDCONST32:
            tmp = nextArg();
            push((nextArg() << 16) | tmp);
            break;
        case LDARGREF: incrpush(args[directArg()]); break;
        case LDARG: push(args[directArg()]); break;
        case LDLOCREF: incrpush(locals[directArg()]); break;
        case LDLOC: push(locals[directArg()]); break;
        case LDENUM: push(enums[directArg()]); break;
        case POP: pop(); break;
        case POPREF: tmp = pop(); decr(tmp); break;
        case STLOC: locals[directArg()] = pop(); break;
        case STLOCREF:
            decr(locals[directArg()]);
            locals[directArg()] = pop();
            break;
        case CLRLOCREF:
            decr(locals[directArg()]);
            locals[directArg()] = 0;
            break;

        case LDGLBREF: incrpush(globals[directArg()]); break;
        case LDGLB: push(globals[directArg()]); break;
        case STGLB: globals[directArg()] = pop(); break;
        case STGLBREF:
            decr(globals[directArg()]);
            globals[directArg()] = pop();
            break;
        case LDSTRREF:
            tmp = strings[directArg()];
            if (tmp) pc++;
            else tmp = strings[directArg()] = (uint32_t)string::fromLiteral((const char*)&bytecode[nextArg()]);
            incrpush(tmp);
            break;

        case NOT: top() = top() == 0; break;
        case ISNULL: decr(top()); top() = top() == 0; break;
        case NEG: top() = -top(); break;

        case LDFLD: push(((RefRecord*)pop())->ld(directArg())); break;
        case LDFLDREF: push(((RefRecord*)pop())->ldref(directArg())); break;
        case STFLD: tmp = pop(); ((RefRecord*)pop())->st(directArg(), tmp); break;
        case STFLDREF: tmp = pop(); ((RefRecord*)pop())->stref(directArg(), tmp); break;

        case STCLO: tmp = pop(); ((RefAction*)top())->st(directArg(), tmp); break;

        case JMP: tmp = nextArg24(); pc = bytecode + tmp; break;
        case JMPZ: tmp = nextArg24(); if (pop() == 0) pc = bytecode + tmp; break;
        case JMPNZ: tmp = nextArg24(); if (pop() != 0) pc = bytecode + tmp; break;

        case FLATUCALLPROC:
            sp -= directArg();
            exec_function(bytecode + nextArg(), sp);
            break;
        case FLATUCALLFUNC:
            sp -= directArg();
            tmp = exec_function(bytecode + nextArg(), sp);
            push(tmp);
            break;

        case UCALLPROC:
            sp -= directArg();
            exec_function(bytecode + nextArg(), sp);
            applyRefMask();
            break;
        case UCALLFUNC:
            sp -= directArg();
            tmp = exec_function(bytecode + nextArg(), sp);
            applyRefMask(); push(tmp);
            break;

        case FLATCALL0PROC:
            ((void (*)())callProc0[directArg()])();
            break;
        case FLATCALL1PROC:
            sp -= 1;
            ((void (*)(uint32_t))callProc1[directArg()])(sp[0]);
            break;
        case FLATCALL2PROC:
            sp -= 2;
            ((void (*)(uint32_t, uint32_t))callProc2[directArg()])(sp[0], sp[1]);
            break;
        case FLATCALL3PROC:
            sp -= 3;
            ((void (*)(uint32_t, uint32_t, uint32_t))callProc3[directArg()])(sp[0], sp[1], sp[2]);
            break;
        case FLATCALL4PROC:
            sp -= 4;
            ((void (*)(uint32_t, uint32_t, uint32_t, uint32_t))callProc4[directArg()])(sp[0], sp[1], sp[2], sp[3]);
            break;
        case FLATCALL0FUNC:
            tmp = (((uint32_t (*)())callFunc0[directArg()])());
            push(tmp);
            break;
        case FLATCALL1FUNC:
            sp -= 1;
            tmp = (((uint32_t (*)(uint32_t))callFunc1[directArg()])(sp[0]));
            push(tmp);
            break;
        case FLATCALL2FUNC:
            sp -= 2;
            tmp = (((uint32_t (*)(uint32_t, uint32_t))callFunc2[directArg()])(sp[0], sp[1]));
            push(tmp);
            break;
        case FLATCALL3FUNC:
            sp -= 3;
            tmp = (((uint32_t (*)(uint32_t, uint32_t, uint32_t))callFunc3[directArg()])(sp[0], sp[1], sp[2]));
            push(tmp);
            break;
        case FLATCALL4FUNC:
            sp -= 4;
            tmp = (((uint32_t (*)(uint32_t, uint32_t, uint32_t, uint32_t))callFunc4[directArg()])(sp[0], sp[1], sp[2], sp[3]));
            push(tmp);
            break;

        case CALL0PROC:
            ((void (*)())callProc0[directArg()])();
            applyRefMask();
            break;
        case CALL1PROC:
            sp -= 1;
            ((void (*)(uint32_t))callProc1[directArg()])(sp[0]);
            applyRefMask();
            break;
        case CALL2PROC:
            sp -= 2;
            ((void (*)(uint32_t, uint32_t))callProc2[directArg()])(sp[0], sp[1]);
            applyRefMask();
            break;
        case CALL3PROC:
            sp -= 3;
            ((void (*)(uint32_t, uint32_t, uint32_t))callProc3[directArg()])(sp[0], sp[1], sp[2]);
            applyRefMask();
            break;
        case CALL4PROC:
            sp -= 4;
            ((void (*)(uint32_t, uint32_t, uint32_t, uint32_t))callProc4[directArg()])(sp[0], sp[1], sp[2], sp[3]);
            applyRefMask();
            break;
        case CALL0FUNC:
            tmp = (((uint32_t (*)())callFunc0[directArg()])());
            applyRefMask(); push(tmp);
            break;
        case CALL1FUNC:
            sp -= 1;
            tmp = (((uint32_t (*)(uint32_t))callFunc1[directArg()])(sp[0]));
            applyRefMask(); push(tmp);
            break;
        case CALL2FUNC:
            sp -= 2;
            tmp = (((uint32_t (*)(uint32_t, uint32_t))callFunc2[directArg()])(sp[0], sp[1]));
            applyRefMask(); push(tmp);
            break;
        case CALL3FUNC:
            sp -= 3;
            tmp = (((uint32_t (*)(uint32_t, uint32_t, uint32_t))callFunc3[directArg()])(sp[0], sp[1], sp[2]));
            applyRefMask(); push(tmp);
            break;
        case CALL4FUNC:
            sp -= 4;
            tmp = (((uint32_t (*)(uint32_t, uint32_t, uint32_t, uint32_t))callFunc4[directArg()])(sp[0], sp[1], sp[2], sp[3]));
            applyRefMask(); push(tmp);
            break;

        default:
            printf("bad opcode: 0x%lx\n", op);
            error(ERR_BAD_OPCODE);
        }

        //check(sp >= stack, ERR_STACK_UNDERFLOW);
        //check(sp <= stack + stackSize, ERR_STACK_OVERFLOW);
    }
}

uint32_t *allocate(uint16_t sz)
{
    uint32_t *arr = new uint32_t[sz];
    memset(arr, 0, sz * 4);
    return arr;
}

int exec_binary()
{
    uint32_t pc = 0;
    uint32_t ver = bytecode[pc++];
    check(ver == V1BINARY, ERR_INVALID_BINARY_HEADER);
    numGlobals = bytecode[pc++];
    numStrings = bytecode[pc++];
    globals = allocate(numGlobals);
    strings = allocate(numStrings);
    pc += 3; // reserved
    return exec_function(bytecode + pc, NULL);
}

} // namespace bitvm

// vim: ts=4 sw=4
