// Copyright (c) Andrew Fischer. See LICENSE file for license terms.

#pragma once

namespace circa {

struct World {
    circa::Value actorList;
    caStack* actorStack;

protected:
    // Disallow C++ construction
    World();
    ~World();
};

World* alloc_world();
caValue* find_actor(World* world, const char* name);
void actor_send_message(caValue* actor, caValue* message);
void actor_run_message(caStack* stack, caValue* actor, caValue* message);

} // namespace circa