#include "algorithms.h"

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
      if (!in_shortest_path[i] &&
          (!(avoid[i]) || (i == (dest - track)) || (i == (src - track)))) {
        min_index = i;
      }
    } else {
      if ((dist[i] < dist[min_index]) && (!in_shortest_path[i]) &&
          (!(avoid[i]) || (i == (dest - track)) || (i == (src - track)))) {
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
  }
}

int dijkstras_min_length(track_node *track, track_node *src, track_node *dest,
                         track_node **prev1, track_node **prev2,
                         track_node **intermediate, bool *avoid, int min_dist) {

  int best_dist = INF;

  for (unsigned int i = 0; i < TRACK_MAX; i++) {

    if (&(track[i]) == src) {
      continue;
    }
    if (&(track[i]) == dest) {
      continue;
    }

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

// int augment_path(track_node *track, track_node **prev, track_node *dest,
//                  int offset, int *new_offset, int *orientation,
//                  track_node **new_dest, int dist, int *new_dist) {

//   *new_offset = 0;

//   *new_dist = dist + offset;

//   if (offset < 0) {

//     track_node *prev_node = prev[dest - track];

//     *new_offset = offset + dest->reverse->edge[DIR_STRAIGHT].dist;

//     *orientation = -1;

//     if (prev_node->type == NODE_BRANCH) {
//       if (prev_node->edge[DIR_AHEAD].dest == dest) {
//         *orientation = DIR_AHEAD;
//       } else {
//         *orientation = DIR_CURVED;
//       }
//     }

//     prev[dest - track] = NULL;

//     *new_dest = prev_node;

//     if (*new_offset < 0) {
//       return -1;
//     }
//     return 0;
//   } else if (offset > 0) {

//     bool is_offset_valid = true;

//     if (dest->type == NODE_BRANCH) {

//       if (dest->edge[DIR_STRAIGHT].dist < offset) {
//         is_offset_valid = false;
//       }

//       if (dest->edge[DIR_CURVED].dist < offset) {
//         is_offset_valid = false;
//       }
//     }

//     if (dest->edge[DIR_STRAIGHT].dist < offset) {
//       is_offset_valid = false;
//     }

//     if (!is_offset_valid) {
//       return -1;
//     }

//     *new_offset = offset;
//     *orientation = -1;
//     *new_dest = dest;

//     return 0;
//   }
// }