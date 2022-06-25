#pragma once

/* THIS FILE IS GENERATED CODE -- DO NOT EDIT */

#include "memory.h"
#include "track_node.h"

// The track initialization functions expect an array of this size.
#define TRACK_MAX 144

void init_tracka(track_node *track);
void init_trackb(track_node *track);

int track_name_to_num(track_node *track, char *name);