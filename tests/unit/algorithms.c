#include "algorithms.h"
#include <assert.h>
#include <stdio.h>

void basic(int src_num, int dest_num) {

  printf("Pathfind from [%d] to [%d]\r\n", src_num, dest_num);

  track_node track[TRACK_MAX];
  init_tracka(&track);

  track_node *prev[TRACK_MAX];

  track_node *src = &(track[src_num]);
  track_node *dest = &(track[dest_num]);

  int status = dijkstra(&track, src, dest, prev);

  if (status == -1) {
    printf("Could not find path\r\n");
    return;
  }

  track_node *node = dest;

  while (1) {

    if (node == NULL) {
      break;
    }

    printf("[%s]\r\n", node->name);

    node = prev[node - track];
  }
}

int main() {
  basic(0, 91);
  basic(1, 91);
  basic(0, 90);

  return 0;
}