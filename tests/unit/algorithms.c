#include "algorithms.h"
#include <assert.h>
#include <stdio.h>

void dijkstra_test(track_node *track, char *src_name, char *dest_name,
                   bool *avoid, int min_dist) {

  printf("Pathfind from [%s] to [%s]\r\n", src_name, dest_name);

  track_node *prev1[TRACK_MAX];
  track_node *prev2[TRACK_MAX];

  // printf("%d\r\n", track_name_to_num(track, src));

  track_node *src = &(track[track_name_to_num(track, src_name)]);
  track_node *dest = &(track[track_name_to_num(track, dest_name)]);

  track_node *intermediate;

  int status = dijkstras_min_length(track, src, dest, prev1, prev2,
                                    &intermediate, avoid, min_dist);

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

    node = prev2[node - track];
  }

  node = prev1[intermediate - track];
  while (1) {

    if (node == NULL) {
      break;
    }

    printf("[%s]\r\n", node->name);

    node = prev1[node - track];
  }
}

int main() {
  bool avoid[TRACK_MAX];
  memset(avoid, 0, sizeof(bool) * TRACK_MAX);

  // avoid[104] = true;
  track_node track[TRACK_MAX];
  init_tracka(track);
  // mark_switch_broken(track, &(track[track_name_to_num(track, "BR156")]),
  //                    DIR_CURVED);
  // mark_switch_broken(track, &(track[track_name_to_num(track, "BR155")]),
  //                    DIR_STRAIGHT);
  dijkstra_test(track, "C2", "D2", avoid, 500);

  // printf("%d\r\n",
  //        track[track_name_to_num(track, "C15")].edge[DIR_STRAIGHT].dist);
  // printf("%d\r\n",
  //        track[track_name_to_num(track, "BR7")].edge[DIR_STRAIGHT].dist);

  // mark_sensor_broken(track, &(track[track_name_to_num(track, "D11")]));

  // printf("%d\r\n",
  //        track[track_name_to_num(track, "C15")].edge[DIR_STRAIGHT].dist);
  // printf("%d\r\n",
  //        track[track_name_to_num(track, "BR7")].edge[DIR_STRAIGHT].dist);

  // dijkstra_test(0, 91);
  // dijkstra_test(1, 91);
  // dijkstra_test(0, 90);

  return 0;
}