#include "algorithms.h"
#include "stdio.h"

int get_path_dist(track_node *track, int *path, int path_len) {
  int dist = 0;
  for (int i = 0; i < path_len - 1; i++) {

    if (track[path[i]].type == NODE_BRANCH) {
      if (track[path[i]].edge[DIR_CURVED].dest == &(track[path[i + 1]])) {
        dist += track[path[i]].edge[DIR_CURVED].dist;
      } else if (track[path[i]].edge[DIR_STRAIGHT].dest ==
                 &(track[path[i + 1]])) {
        dist += track[path[i]].edge[DIR_STRAIGHT].dist;
      } else {
        return -1;
      }
    } else {
      if (track[path[i]].edge[DIR_AHEAD].dest == &(track[path[i + 1]])) {
        dist += track[path[i]].edge[DIR_AHEAD].dist;
      } else {
        return -1;
      }
    }
  }
  return dist;
}

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

track_node *min_distance(track_node *track, int *dist, bool *in_shortest_path,
                         bool *avoid, track_node *src, track_node *dest) {
  int min_index = -1;
  for (unsigned int i = 0; i < TRACK_MAX; i++) {

    if (min_index == -1) {
      if (!in_shortest_path[i] && !(avoid[i]) &&
          !(avoid[track[i].reverse - track])) {
        min_index = i;
      }
    } else {
      if ((dist[i] < dist[min_index]) && (!in_shortest_path[i]) &&
          !(avoid[i])) {
        min_index = i;
      }
    }
  }

  return &(track[min_index]);
}

int dijkstra(track_node *track, track_node *src, track_node *dest,
             track_node **prev, bool *avoid) {

  int dist[TRACK_MAX];

  bool in_shortest_path[TRACK_MAX];

  for (unsigned int i = 0; i < TRACK_MAX; i++) {
    dist[i] = INF;
    prev[i] = NULL;
    in_shortest_path[i] = false;
  }

  dist[src - track] = 0;

  for (;;) {

    track_node *u =
        min_distance(track, dist, in_shortest_path, avoid, src, dest);

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

    track_edge mock_reverse_edge;
    mock_reverse_edge.dest = u->reverse;
    mock_reverse_edge.dist = 0;
    mock_reverse_edge.reverse = NULL;
    mock_reverse_edge.src = u;
    v = mock_reverse_edge.dest;

    update_dist(track, u, v, in_shortest_path, prev, &mock_reverse_edge, dist);
  }
}

int dijkstras_min_length(track_node *track, track_node *src, track_node *dest,
                         track_node **prev1, track_node **prev2,
                         track_node **intermediate, bool *avoid, int min_dist) {

  int best_dist = INF;

  for (unsigned int i = 0; i < TRACK_MAX; i++) {

    track_node *cur_intermediate = &(track[i]);
    track_node *cur_prev1[TRACK_MAX];
    track_node *cur_prev2[TRACK_MAX];

    int path_len1 = dijkstra(track, src, cur_intermediate, cur_prev1, avoid);
    int path_len2 = dijkstra(track, cur_intermediate, dest, cur_prev2, avoid);

    if ((path_len1 != -1) && (path_len2 != -1)) {

      if (path_len1 + path_len2 >= min_dist) {
        if (path_len1 + path_len2 < best_dist) {
          best_dist = path_len1 + path_len2;
          memcpy(prev1, cur_prev1, sizeof(track_node *) * TRACK_MAX);
          memcpy(prev2, cur_prev2, sizeof(track_node *) * TRACK_MAX);
          *intermediate = cur_intermediate;
        }
      }
    }
  }

  if (best_dist == INF) {
    return -1;
  }
  return best_dist;
}
