#pragma once

/* THIS FILE IS GENERATED CODE -- DO NOT EDIT */

#include "memory.h"

#ifdef IS_TARGET
#include "my_assert.h"
#else
#include "assert.h"
#endif
#include "track_node.h"

// The track initialization functions expect an array of this size.
#define TRACK_MAX 144

#define INF 100000

extern track_node track[TRACK_MAX];
extern char which_track;

void init_tracka(track_node *track);
void init_trackb(track_node *track);

int track_name_to_num(track_node *track, char *name);
void mark_sensor_broken(track_node *track, int node);
void mark_switch_broken(track_node *track, int node, int stuck_direction);
