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

  int result = dijkstras_min_length(track, src, dest, prev1, prev2,
                                    &intermediate, avoid, min_dist);

  if (result == -1) {
    printf("Could not find path\r\n");
    return;
  }

  track_node *path1[TRACK_MAX];
  track_node *path2[TRACK_MAX];

  memset(path1, NULL, sizeof(track_node *) * TRACK_MAX);
  memset(path2, NULL, sizeof(track_node *) * TRACK_MAX);

  unsigned int path_len = 2;
  unsigned int path_dist = result;

  track_node *final_path[TRACK_MAX * 2];
  memset(final_path, NULL, sizeof(track_node *) * TRACK_MAX);

  track_node *node = dest;

  while (node != intermediate) {
    path2[prev2[node - track] - track] = node;
    node = prev2[node - track];
    path_len += 1;
  }

  node = prev1[intermediate - track];

  while (node != src) {
    path1[prev1[node - track] - track] = node;
    node = prev1[node - track];
    path_len += 1;
  }

  node = src;

  unsigned int partial_len = 0;

  for (unsigned int i = 0; i < path_len; i++) {
    final_path[i] = node;
    printf("[%s]", node->name);
    node = path1[node - track];
    partial_len++;

    if (node == NULL) {
      break;
    }
  }

  node = intermediate;

  for (unsigned int i = 0; i < path_len; i++) {
    final_path[i + partial_len] = node;
    printf("[%s]", node->name);
    node = path2[node - track];

    if (node == NULL) {
      break;
    }
  }

  printf("\r\n");
  for (unsigned int i = 0; i < path_len; i++) {
    printf("[%s]", final_path[i]->name);
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
  dijkstra_test(track, "C2", "D2", avoid, 20000);

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