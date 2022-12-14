#include "pathworker.h"
#include <syscall.h>

// return path + distance

void pathfind_worker() {

  task_tid navigationserver = MyParentTid();
  task_tid clockserver = WhoIsBlock("clockserver");

  navigationserver_request req;
  navigationserver_response res;
  memset(&req, 0, sizeof(navigationserver_request));

  req.type = WHOAMI;
  req.data.whoami.worker_type = PATHFIND;
  Send(navigationserver, (char *)&req, sizeof(navigationserver_request),
       (char *)&res, sizeof(navigationserver_response));

  v_train_num train = res.data.whoami.train;
  char debug_buffer[MAX_DEBUG_STRING_LEN];
  sprintf(debug_buffer, "Got whoami as train %d", v_p_train_num(train));
  debugprint(debug_buffer, PATH_WORKER_DEBUG);

  while (true) {
    memset(&req, 0, sizeof(navigationserver_request));
    req.type = PATHFIND_WORKER;
    req.data.pathfindworker.train_num = train;
    Send(navigationserver, (char *)&req, sizeof(req), (char *)&res,
         sizeof(res));

    if (res.type == PATHFIND_WORKER_HERES_WORK) {
      sprintf(debug_buffer, "Got work for train %d", v_p_train_num(train));
      debugprint(debug_buffer, PATH_WORKER_DEBUG);

      // printf(BW_COM2, "got here\r\n");

      track_node *prev[TRACK_MAX];

      track_node *src = &(track[res.data.pathfindworker.src_num]);
      track_node *dest = &(track[res.data.pathfindworker.dest_num]);

      int delay_time = res.data.pathfindworker.delay_time;

      Delay(clockserver, delay_time);

      bool no_reserve[TRACK_MAX];
      memset(no_reserve, 0, sizeof(bool) * TRACK_MAX);

      int result = dijkstra(track, src, dest, prev, no_reserve);

      if (result == -1) {
        memset(&req, 0, sizeof(navigationserver_request));
        req.type = PATHFIND_WORKER_DONE;
        req.data.pathfindworker_done.pathfind_result = NO_PATH_AT_ALL;
        req.data.pathfindworker_done.train_num = train;
        Send(navigationserver, (char *)&req, sizeof(req), (char *)&res, 0);
        continue;
      }

      bool *reserved = res.data.pathfindworker.reserved_nodes;

      result = dijkstra(track, src, dest, prev, reserved);

      if (result == -1) {
        memset(&req, 0, sizeof(navigationserver_request));
        req.type = PATHFIND_WORKER_DONE;
        req.data.pathfindworker_done.pathfind_result = NO_PATH_WITH_RESERVE;
        req.data.pathfindworker_done.train_num = train;
        Send(navigationserver, (char *)&req, sizeof(req), (char *)&res, 0);
        continue;
      }

      unsigned int path_len = 0;

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
      req.data.pathfindworker_done.pathfind_result = FOUND_PATH;
      req.data.pathfindworker_done.train_num = train;
      req.data.pathfindworker_done.path_len = path_len;

      // printf(BW_COM2, "worker good\r\n");
      Send(navigationserver, (char *)&req, sizeof(req), (char *)&res, 0);
    }
  }
}
