// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

// Actors v3 (current)

struct Actor {
    int id;
    Stack* stack;
};

Stack* create_actor(World* world, Block* block);
bool state_inject(Stack* stack, caValue* name, caValue* value);
void context_inject(Stack* stack, caValue* name, caValue* value);

} // namespace circa
