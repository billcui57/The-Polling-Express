#include "algorithms.h"
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

  int status = dijkstra(&track, src, dest, prev);

  if (status == -1) {
    printf("Could not find path\r\n");
    return;
  }

  track_node *node = dest;

  while (1) {

    printf("[%s]\r\n", node->name);

    node = prev[node - track];

    if (node == src) {
      break;
    }
  }

  // while (prev[node - track] != src) {
  //   node = prev[node - track];
  // }

  // printf("[%d]", node->num);
}

int main() {
  basic();

  return 0;
}