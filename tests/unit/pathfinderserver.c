#include "pathfinderserver.h"
#include <assert.h>
#include <stdio.h>

void basic() {

  track_node track[TRACK_MAX];
  init_tracka(&track);

  track_node *prev[TRACK_MAX];

  track_node *src = &(track[4]);
  track_node *dest = &(track[91]);

  dijkstra(&track, src, dest, prev);

  track_node *node = prev[91] == NULL ? &(track[90]) : &(track[91]);

  while (1) {

    printf("[%s]", node->name);

    node = prev[node - track];

    if (node == src) {
      break;
    } else {
      printf("<-");
    }
  }
}

int main() {
  basic();

  return 0;
}