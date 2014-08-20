// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#include "common_headers.h"

#include <cassert>

#include "block.h"
#include "hashtable.h"
#include "interpreter.h"
#include "inspection.h"
#include "kernel.h"
#include "list.h"
#include "names_builtin.h"
#include "world.h"

#include "debug.h"

namespace circa {

bool DEBUG_TRAP_NAME_LOOKUP = false;
bool DEBUG_TRAP_RAISE_ERROR = false;

int DEBUG_BREAK_ON_TERM = -1;

uint64 PERF_STATS[c_numPerfStats];

void dump(Block& block)
{
    Value str;
    print_block(&block, &str);
    std::cout << as_cstring(&str);
}
void dump(Block* block)
{
    Value str;
    print_block(block, &str);
    std::cout << as_cstring(&str);
}
void dump_with_props(Block* block)
{
    Value str;
    print_block_with_properties(block, &str);
    std::cout << as_cstring(&str);
}
void dump_bytecode(Block* block)
{
    Value str;
    RawOutputPrefs prefs;
    prefs.showBytecode = true;
    print_block(block, &prefs, &str);
    std::cout << as_cstring(&str);
}

void dump_with_props(Block& block)
{
    Value str;
    print_block_with_properties(&block, &str);
    std::cout << as_cstring(&str);
}

void dump(Term* term)
{
    Value str;
    print_term(term, &str);
    std::cout << as_cstring(&str);
}

void dump(Value& value)
{
    Value str;
    to_string(&value, &str);
    std::cout << as_cstring(&str) << std::endl;
}
void dump(Value* value)
{
    Value str;
    to_string(value, &str);
    std::cout << as_cstring(&str) << std::endl;
}

void dump(Stack* stack)
{
    Value str;
    stack_to_string(stack, &str, false);
    write_log(as_cstring(&str));
}

void internal_error(const char* message)
{
    std::string msg("internal_error: ");
    msg += message;
    write_log(msg.c_str());
    assert(false);
}

void internal_error(std::string const& message)
{
    internal_error(message.c_str());
}

void ca_assert_function(bool expr, const char* exprStr, int line, const char* file)
{
    if (!expr) {
        std::stringstream msg;
        msg << "ca_assert(" << exprStr << ") failed in " << file << " line " << line;
        internal_error(msg.str().c_str());
    }
}

void ca_debugger_break()
{
#if CIRCA_TEST_BUILD
    //asm { int 3 };
    internal_error("debugger break");
#else
    internal_error("debugger break");
#endif
}

CIRCA_EXPORT void circa_perf_stats_dump()
{
    printf("perf_stats_dump:\n");
    for (int i=c_firstStatIndex; i < sym_LastStatIndex-1; i++)
        printf("  %s = %llu\n", builtin_symbol_to_string(i), PERF_STATS[i - c_firstStatIndex]);
}
CIRCA_EXPORT void circa_perf_stats_reset()
{
    for (int i = c_firstStatIndex; i < sym_LastStatIndex-1; i++)
        PERF_STATS[i - c_firstStatIndex] = 0;
}
void perf_stats_to_list(Value* list)
{
    set_list(list, c_numPerfStats);
    for (int i = c_firstStatIndex; i < sym_LastStatIndex-1; i++) {
        Symbol name = i;
        int64 value = PERF_STATS[i - c_firstStatIndex];
        Value* element = list_get(list, i - c_firstStatIndex);
        set_list(element, 2);
        set_symbol(list_get(element, 0), name);
        char buf[100];
        sprintf(buf, "%llu", value);
        set_string(list_get(element, 1), buf);
    }
}

void perf_stats_to_map(Value* map)
{
    u64 frozenStats[c_numPerfStats];

    for (int name = c_firstStatIndex; name < sym_LastStatIndex-1; name++) {
        int i = name - c_firstStatIndex;
        frozenStats[i] = PERF_STATS[i];
    }

    set_hashtable(map);

    for (int name = c_firstStatIndex; name < sym_LastStatIndex-1; name++) {
        int i = name - c_firstStatIndex;
        u64 value = frozenStats[i];
        set_int(hashtable_insert_symbol_key(map, name), value);
    }
}

void perf_stat_inc(int name)
{
    PERF_STATS[name - c_firstStatIndex]++;
}

void perf_stat_add(int name, int n)
{
    PERF_STATS[name - c_firstStatIndex] += n;
}

#if CIRCA_ENABLE_LOGGING

FILE* g_logFile = NULL;
int g_logArgCount = 0;
int g_logInProgress = false;

void log_start(int channel, const char* name)
{
    ca_assert(!g_logInProgress);

    if (g_logFile == NULL)
        g_logFile = fopen("circa.log", "w");

    // Ignore channel for now

    // Timestamp
    time_t rawtime;
    struct tm * timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);

    char timestamp[100];
    strftime(timestamp, 100, "%H:%M:%S", timeinfo);
    fprintf(g_logFile, "%s %s(", timestamp, name);

    g_logArgCount = 0;
    g_logInProgress = true;
}

void log_arg(const char* key, const char* val)
{
    ca_assert(g_logInProgress);
    if (g_logArgCount > 0)
        fprintf(g_logFile, ", ");

    fprintf(g_logFile, "%s = %s", key, val);
    g_logArgCount++;
}
void log_arg(const char* key, int val)
{
    char buf[32];
    sprintf(buf, "%d", val);
    log_arg(key, buf);
}

void log_finish()
{
    ca_assert(g_logInProgress);
    fprintf(g_logFile, ")\n");
    g_logInProgress = false;
    g_logArgCount = 0;
}
void log_msg(int channel, const char* name)
{
    log_start(channel, name);
    log_finish();
}

#endif

void write_log(World* world, const char* msg)
{
    if (world == NULL || world->logFunc == NULL)
        printf("%s\n", msg);
    else
        world->logFunc(world->logContext, msg);
}
void write_log(const char* msg)
{
    write_log(global_world(), msg);
}

} // namespace circa
