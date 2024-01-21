#include <stdio.h>

#include "debug.h"
#include "value.h"

void disassembleChunk(Chunk* chunk, const char* name) {
    printf("== %s ==\n", name);

    for (int i = 0; i < chunk->linesCount; i++) {
        printf("count: %d line: %d\n",
            chunk->lines[i * 2], chunk->lines[i * 2 + 1]);
    }

    for (int offset = 0; offset < chunk->count;) {
        offset = disassembleInstruction(chunk, offset);
    }
}

static int getLine(Chunk* chunk, int offset) {
    for (int i = 0; i < chunk->count; i++) {
        if (chunk->lines[i * 2] >= offset) {
            return chunk->lines[i * 2 + 1];
        }
    }
    return -1;
}

static int simpleInstruction(const char* name, int offset) {
    printf("%s\n", name);
    return offset + 1;
}

static int constantInstruction(const char* name, Chunk* chunk, int offset) {
    uint8_t constant = chunk->code[offset + 1];
    printf("%-16s %4d '", name, constant);
    printValue(chunk->constants.values[constant]);
    printf("'\n");
    return offset + 2;
}

int disassembleInstruction(Chunk* chunk, int offset) {
    printf("%04d ", offset);

    int currentLine = getLine(chunk, offset);
    int previousLine = getLine(chunk, offset - 1);
    if (offset > 0 && currentLine == previousLine) {
        printf("    | ");
    } else {
        printf("%4d ", currentLine);
    }

    /* if (offset > 0 && */
    /*         chunk->lines[offset] == chunk->lines[offset - 1]) { */
    /*     printf("    | "); */
    /* } else { */
    /*     printf("%4d ", chunk->lines[offset]); */
    /* } */

    uint8_t instruction = chunk->code[offset];
    switch(instruction) {
        case OP_RETURN:
            return simpleInstruction("OP_RETURN", offset);
        case OP_CONSTANT:
            return constantInstruction("OP_CONSTANT", chunk, offset);
        default:
            printf("Unknown opcode %d\n", instruction);
            return offset + 1;
    }
}
