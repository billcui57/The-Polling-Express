#include "pathworker.h"
#include <syscall.h>

// return path + distance

void pathfind_worker() {

  task_tid parent = MyParentTid();

  navigationserver_request req;
  navigationserver_response res;
  memset(&req, 0, sizeof(navigationserver_request));

  while (true) {
    req.type = PATHFIND_WORKER;
    Send(parent, (char *)&req, sizeof(req), (char *)&res, sizeof(res));

    if (res.type == PATHFIND_WORKER_HERES_WORK) {

      // printf(BW_COM2, "got here\r\n");

      track_node *prev[TRACK_MAX];

      track_node *src = &(track[res.data.pathfindworker.src]);
      track_node *dest = &(track[res.data.pathfindworker.dest]);

      bool *reserved = res.data.pathfindworker.reserved_nodes;

      track_node *intermediate;

      int result = dijkstra(track, src, dest, prev, reserved);

      char debug_buffer[100];
      sprintf(debug_buffer, "A path should exist between [%s] and [%s]\r\n",
              src->name, dest->name);
      KASSERT(result != -1, debug_buffer);

      track_node *node = dest;

      track_node *path[TRACK_MAX];

      unsigned int path_len = 1;
      unsigned int path_dist = result;

      while (node != src) {
        path[prev[node - track] - track] = node;
        node = prev[node - track];
        path_len += 1;
      }

      node = src;

      for (unsigned int i = 0; i < path_len; i++) {
        req.data.pathfindworker_done.path[i] = node - track;
        node = path[node - track];
      }

      req.type = PATHFIND_WORKER_DONE;
      req.data.pathfindworker_done.train_num = res.data.pathfindworker.train;
      req.data.pathfindworker_done.path_dist =
          path_dist + res.data.pathfindworker.offset;
      req.data.pathfindworker_done.path_len = path_len;

      // printf(BW_COM2, "worker good\r\n");
      Send(parent, (char *)&req, sizeof(req), (char *)&res, 0);
    }
  }
}
