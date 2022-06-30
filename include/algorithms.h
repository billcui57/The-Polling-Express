#pragma once

#include "stdbool.h"
#include "track_data.h"
#include "track_node.h"
int dijkstra(track_node *track, track_node *src, track_node *dest,
             track_node **prev);

// int augment_path(track_node *track, track_node **prev, track_node *dest,
//                  int offset, int *new_offset, int *orientation,
//                  track_node **new_dest, int dist, int *new_dist);