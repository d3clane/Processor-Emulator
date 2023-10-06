#include <assert.h>
#include <string.h>

#include "Disassembly.h"
#include "../../InputOutput.h"

static inline void SkipAddedInfo(TextType* byteCode);
static inline void ReturnToTheByteCodeStart(TextType* byteCode);
static inline CommandsErrors CheckAddedInfo(TextType* byteCode);

static inline char* CopyLine(const char* source, char* target);

static const uint32_t DisassemblyVersion = 1;

CommandsErrors Disassembly(FILE* inStream, FILE* outStream)
{
    assert(inStream);
    assert(outStream);

    TextType byteCode = {};
    TextTypeCtor(&byteCode, inStream);

    CommandsErrors addedInfoErrors = CheckAddedInfo(&byteCode);

    if (addedInfoErrors != CommandsErrors::NO_ERR)
    {
        COMMANDS_ERRORS_LOG_ERROR(addedInfoErrors);
                           return addedInfoErrors;
    }

    SkipAddedInfo(&byteCode);

    static const size_t maxCommandLength = 5;
    char* asmCode    = (char*) calloc(byteCode.textSz * maxCommandLength, sizeof(*asmCode));
    char* asmCodePtr = asmCode;

    for (size_t line = 0; line < byteCode.linesCnt; ++line)
    {
        int command = -1;
        bool quitCycle = false;

        size_t readCommandLength = 0; //TODO: можно короче бинарный файл и удобнее будет
        sscanf(byteCode.lines[line].line, "%d", &command);

        assert(asmCodePtr < asmCode + byteCode.textSz * maxCommandLength);

        switch((Commands) command)
        {
            case Commands::PUSH_ID:
                asmCodePtr += sprintf(asmCodePtr, "%s", PUSH);
                readCommandLength = 1;
                break;
            case Commands::IN_ID:
                asmCodePtr += sprintf(asmCodePtr, "%s", IN);
                readCommandLength = 1;
                break;
            case Commands::DIV_ID:
                asmCodePtr += sprintf(asmCodePtr, "%s", DIV);
                readCommandLength = 1;
                break;
            case Commands::ADD_ID:
                asmCodePtr += sprintf(asmCodePtr, "%s", ADD);
                readCommandLength = 1;
                break;
            case Commands::SUB_ID:
                asmCodePtr += sprintf(asmCodePtr, "%s", SUB);
                readCommandLength = 1;
                break;
            case Commands::MUL_ID:
                asmCodePtr += sprintf(asmCodePtr, "%s", MUL);
                readCommandLength = 1;
                break;
            case Commands::OUT_ID:
                asmCodePtr += sprintf(asmCodePtr, "%s", OUT);
                readCommandLength = 1;
                break;
            case Commands::HLT_ID:
                asmCodePtr += sprintf(asmCodePtr, "%s", HLT);
                readCommandLength = 1;
                quitCycle = true;
                break;
            default:
                COMMANDS_ERRORS_LOG_ERROR(CommandsErrors::INVALID_COMMAND_ID);

                TextTypeDestructor(&byteCode);

                free(asmCode);
                asmCode    = nullptr;
                asmCodePtr = nullptr;

                return CommandsErrors::INVALID_COMMAND_ID;
        }

        if (quitCycle)
            break;

        asmCodePtr = CopyLine(byteCode.lines[line].line + readCommandLength, asmCodePtr);
    }

    PrintText(asmCode, strlen(asmCode), outStream);

    ReturnToTheByteCodeStart(&byteCode);
    TextTypeDestructor(&byteCode);

    free(asmCode);
    asmCode    = nullptr;
    asmCodePtr = nullptr;

    return CommandsErrors::NO_ERR;
}

static inline char* CopyLine(const char* source, char* target)
{
    assert(source);
    assert(target);

    const char* srcPtr  = source;
          char* targPtr = target;
    
    while (*srcPtr != '\n' && *srcPtr != '\0')
    {
        *targPtr = *srcPtr;

        ++targPtr;
        ++srcPtr;
    }

    *targPtr = *srcPtr;
    ++targPtr;

    return targPtr;
}

static inline void SkipAddedInfo(TextType* byteCode)
{
    assert(byteCode);
    assert(byteCode->linesCnt >= AddedInfoNumberOfLines);

    byteCode->lines    += AddedInfoNumberOfLines;
    byteCode->linesCnt -= AddedInfoNumberOfLines;

    /*
    byteCode->text   += AddedInfoSize;
    byteCode->textSz -= AddedInfoSize;
    */
   //Не меняю потому что во-первых проблема в том, что надо узнать размер добавленной инфы в виде строчек
   // решение, которые я придумал - перейти на бинарный файл, там уже сразу знаю
   // другое решение - самому сделать глобальную коснтанту потому что я могу предсказать кол-во чаров
   // (ну типо я буквально знаю что я пушу так как это константы)
}

static inline void ReturnToTheByteCodeStart(TextType* byteCode)
{
    assert(byteCode);

    byteCode->lines    -= AddedInfoNumberOfLines;
    byteCode->linesCnt += AddedInfoNumberOfLines;

    /*
    byteCode->text   -= AddedInfoSize;
    byteCode->textSz += AddedInfoSize;
    */
}

static inline CommandsErrors CheckAddedInfo(TextType* byteCode)
{
    assert(byteCode);
    assert(byteCode->linesCnt > 1);

    SignatureType fileSignature = (SignatureType) strtol(byteCode->lines[0].line, nullptr, 10);
    
    if (fileSignature != Signature)
    {
        COMMANDS_ERRORS_LOG_ERROR(CommandsErrors::INVALID_SIGNATURE);
                           return CommandsErrors::INVALID_SIGNATURE; 
    }
    
    VersionType fileVersion = (VersionType) strtol(byteCode->lines[1].line, nullptr, 10);

    if (fileVersion != DisassemblyVersion)
    {
        COMMANDS_ERRORS_LOG_ERROR(CommandsErrors::INVALID_VERSIONS);
                           return CommandsErrors::INVALID_VERSIONS;
    }

    return CommandsErrors::NO_ERR;
}
