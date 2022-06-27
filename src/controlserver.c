#include "controlserver.h"
#include <syscall.h>

#define INF 100000

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
  int min_index = 0;
  for (unsigned int i = 0; i < TRACK_MAX; i++) {
    if ((dist[i] < dist[min_index]) && (!in_shortest_path[i])) {
      min_index = i;
    }
  }

  return &(track[min_index]);
}

// TODO: need to add a way to see reverse node and node as same for reverse
// pathfinding to work
void dijkstra(track_node *track, track_node *src, track_node *dest,
              track_node **prev) {

  int dist[TRACK_MAX];

  bool in_shortest_path[TRACK_MAX];

  for (unsigned int i = 0; i < TRACK_MAX; i++) {
    dist[i] = INF;
    prev[i] = NULL;
    in_shortest_path[i] = false;
  }

  dist[src - track] = 0;

  for (unsigned int i = 0; i < TRACK_MAX - 1; i++) {

    track_node *u = min_distance(track, dist, in_shortest_path);

    if ((u == dest) || (u == dest->reverse)) {
      return;
    }

    for (unsigned int is_reverse = 0; is_reverse < 2; is_reverse++) {
      if (is_reverse == 1) {
        u = u->reverse;
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
}

bool is_switch_node(track_node *node) {
  return (node->type == NODE_BRANCH) || ((node->type == NODE_MERGE));
}

void pathfind_worker() {

  task_tid clock = WhoIsBlock("clockserver");
  task_tid trainserver = WhoIsBlock("trainctl");
  task_tid parent = MyParentTid();

  controlserver_request req;
  memset(&req, 0, sizeof(controlserver_request));
  controlserver_response res;

  while (true) {
    req.type = CONTROLSERVER_WORKER;
    req.data.worker.time = Time(clock);
    Send(parent, (char *)&req, sizeof(controlserver_request), (char *)&res,
         sizeof(controlserver_response));

    if (res.type == WORKER_PATHFIND) {
      // do dijkstras

      int src_num = res.data.worker_pathfind.src_num;
      int dest_num = res.data.worker_pathfind.dest_num;

      track_node *prev[TRACK_MAX];

      track_node *src = &(track[src_num]);
      track_node *dest = &(track[dest_num]);

      dijkstra(track, src, dest, prev);

      track_node *node =
          prev[dest_num] == NULL ? track[dest_num].reverse : &(track[dest_num]);

      track_node *path[TRACK_MAX];

      while (node != src) {
        path[prev[node - track] - track] = node;
        node = prev[node - track];
      }

      node = src;
      while (true) {
        track_node *next = path[node - track];

        if (node->type == NODE_BRANCH) {
          if (next != NULL) {

            int switch_num = node->num;

            TrainCommand(trainserver, Time(clock), SWITCH, switch_num,
                         next == node->edge[DIR_STRAIGHT].dest ? 0 : 1);
          }
        }

        node = next;
      }
    }
  }

  // pathfinder server contains occupied nodes

  // CONTROLSERVER_WORKER only gets the next nodes until next sensor from
  // dijkstras before sending to server server receives with occupied node and
  // adds to occupied nodes

  // server gets next task from heap
}

void control_server() {

  RegisterAs("controlserver");

  controlserver_request req;
  controlserver_response res;
  task_tid client;

  memset(&res, 0, sizeof(res));

  for (;;) {
    Receive(&client, (char *)&req, sizeof(controlserver_request));
  }
}
