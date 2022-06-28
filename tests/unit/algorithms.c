#include "algorithms.h"
#include <assert.h>
#include <stdio.h>

void dijkstra_test(int src_num, int dest_num) {

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

void normalize_node_test(int src_num, int dest_num, int offset) {
  printf("Pathfind from [%d] to [%d]\r\n", src_num, dest_num);

  track_node track[TRACK_MAX];
  init_tracka(&track);

  track_node *prev[TRACK_MAX];

  track_node *src = &(track[src_num]);
  track_node *dest = &(track[dest_num]);

  int dist = dijkstra(&track, src, dest, prev);

  if (dist == -1) {
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

  printf("dist [%d]\r\n", dist);

  printf("==========\r\n");
  printf("Augmented\r\n");

  int new_offset;
  int orientation;

  int new_dist;
  track_node *new_dest;

  int status = augment_path(track, prev, dest, offset, &new_offset,
                            &orientation, &new_dest, dist, &new_dist);

  if (status == -1) {
    printf("Invalid offset\r\n");
    return;
  }

  node = new_dest;

  while (1) {

    if (node == NULL) {
      break;
    }

    printf("[%s]\r\n", node->name);

    node = prev[node - track];
  }

  printf("New offset [%d] | switch orientation on last [%c] | new dist [%d]",
         new_offset, orientation == DIR_AHEAD ? 's' : 'c', new_dist);
}

int main() {
  // dijkstra_test(0, 1);
  // dijkstra_test(0, 91);
  // dijkstra_test(1, 91);
  // dijkstra_test(0, 90);

  // normalize_node_test(89, -3);

  // normalize_node_test(5, -3);
  // normalize_node_test(5, -300);
  normalize_node_test(75, 58, -5);
  normalize_node_test(75, 58, -300);
  normalize_node_test(75, 58, 500);

  return 0;
}