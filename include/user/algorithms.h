#pragma once

#include "stdbool.h"
#include "track_data.h"
#include "track_node.h"
int dijkstra(track_node *track, track_node *src, track_node *dest,
             track_node **prev, bool *avoid);

int dijkstras_min_length(track_node *track, track_node *src, track_node *dest,
                         track_node **prev1, track_node **prev2,
                         track_node **intermediate, bool *avoid, int min_dist);

int get_path_dist(track_node *track, int *path, int path_len);
// int augment_path(track_node *track, track_node **prev, track_node *dest,
//                  int offset, int *new_offset, int *orientation,
//                  track_node **new_dest, int dist, int *new_dist);