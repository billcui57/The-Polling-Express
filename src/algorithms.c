#include "algorithms.h"

#define INF 100000

bool is_switch_node(track_node *node) {
  return (node->type == NODE_BRANCH) || ((node->type == NODE_MERGE));
}

void update_dist(track_node *track, track_node *u, track_node *v,
                 bool *in_shortest_path, track_node **prev, track_edge *uv,
                 int *dist) {
  if (!in_shortest_path[v - track]) {

    int alt = dist[u - track] + uv->dist;

    if ((alt < dist[v - track]) && (dist[u - track] != INF)) {
      dist[v - track] = alt;
      prev[v - track] = u;
    }
  }
}

track_node *min_distance(track_node *track, int *dist, bool *in_shortest_path) {
  int min_index = -1;
  for (unsigned int i = 0; i < TRACK_MAX; i++) {

    if (min_index == -1) {
      if (!in_shortest_path[i]) {
        min_index = i;
      }
    } else {
      if ((dist[i] < dist[min_index]) && (!in_shortest_path[i])) {
        min_index = i;
      }
    }
  }

  return &(track[min_index]);
}

int dijkstra(track_node *track, track_node *src, track_node *dest,
             track_node **prev) {

  int dist[TRACK_MAX];

  bool in_shortest_path[TRACK_MAX];

  for (unsigned int i = 0; i < TRACK_MAX; i++) {
    dist[i] = INF;
    prev[i] = NULL;
    in_shortest_path[i] = false;
  }

  dist[src - track] = 0;

  for (;;) {

    track_node *u = min_distance(track, dist, in_shortest_path);

    if (dist[u - track] == INF) {
      return -1;
    }

    if (u == dest) {
      return dist[u - track];
    }

    in_shortest_path[u - track] = true;

    track_edge *uv;
    track_node *v;

    if ((u->type == NODE_MERGE) || (u->type == NODE_SENSOR) ||
        (u->type == NODE_ENTER)) {

      uv = &((u->edge)[DIR_AHEAD]);
      v = uv->dest;

      update_dist(track, u, v, in_shortest_path, prev, uv, dist);
    }

    if (u->type == NODE_BRANCH) {
      uv = &((u->edge)[DIR_STRAIGHT]);
      v = uv->dest;
      update_dist(track, u, v, in_shortest_path, prev, uv, dist);
      uv = &((u->edge)[DIR_CURVED]);
      v = uv->dest;

      update_dist(track, u, v, in_shortest_path, prev, uv, dist);
    }
  }
}