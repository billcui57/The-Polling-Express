#include "pathfinderserver.h"
#include <assert.h>
#include <stdio.h>

void basic() {

  track_node track[TRACK_MAX];
  init_tracka(&track);

  track_node *prev[TRACK_MAX];

  int src_num = 4;
  int dest_num = 91;

  track_node *src = &(track[src_num]);
  track_node *dest = &(track[dest_num]);

  dijkstra(&track, src, dest, prev);

  track_node *node =
      prev[dest_num] == NULL ? track[dest_num].reverse : &(track[dest_num]);

  // while (1) {

  //   printf("[%s]", node->name);

  //   node = prev[node - track];

  //   if (node == src) {
  //     break;
  //   } else {
  //     printf("<-");
  //   }
  // }

  while (prev[node - track] != src) {
    node = prev[node - track];
  }

  printf("[%d]", node->num);
}

int main() {
  basic();

  return 0;
}