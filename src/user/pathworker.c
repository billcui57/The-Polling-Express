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

      int result = dijkstra(track, src, dest, prev, reserved);

      char debug_buffer[100];
      sprintf(debug_buffer, "A path should exist between [%s] and [%s]\r\n",
              src->name, dest->name);
      KASSERT(result != -1, debug_buffer);

      unsigned int path_len = 0;
      unsigned int path_dist = result;

      track_node *node = dest;
      while (1) {
        if (node == NULL) {
          break;
        }
        path_len++;
        node = prev[node - track];
      }

      memset(&req, 0, sizeof(navigationserver_request));

      node = dest;
      for (int i = path_len - 1; i > -1; i--) {
        req.data.pathfindworker_done.path[i] = node - track;
        node = prev[node - track];
      }

      req.type = PATHFIND_WORKER_DONE;
      req.data.pathfindworker_done.train_num = res.data.pathfindworker.train;
      req.data.pathfindworker_done.path_dist = path_dist;
      req.data.pathfindworker_done.path_len = path_len;

      // printf(BW_COM2, "worker good\r\n");
      Send(parent, (char *)&req, sizeof(req), (char *)&res, 0);
    }
  }
}
