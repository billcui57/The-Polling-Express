#pragma once

#include "heap.h"
#include "stdbool.h"
#include "track_data.h"
#include "track_node.h"

void dijkstra(track_node *track, track_node *src, track_node *dest,
              track_node **prev);
