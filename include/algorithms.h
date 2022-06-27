#pragma once

#include "stdbool.h"
#include "track_data.h"
#include "track_node.h"
int dijkstra(track_node *track, track_node *src, track_node *dest,
             track_node **prev);