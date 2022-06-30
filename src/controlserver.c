#include "controlserver.h"
#include <syscall.h>

// return path + distance

void control_worker() {
  task_tid trainserver = WhoIsBlock("trainctl");
  task_tid clock = WhoIsBlock("clockserver");
  task_tid parent = MyParentTid();

  controlserver_request req;
  controlserver_response res;
  memset(&req, 0, sizeof(controlserver_request));

  while (true) {
    req.type = CONTROL_WORKER;
    Send(parent, (char *)&req, sizeof(req), (char *)&res, sizeof(res));

    if (res.type == WORKER_PATHFIND) {

      // printf(BW_COM2, "got here\r\n");

      track_node *prev1[TRACK_MAX];
      track_node *prev2[TRACK_MAX];

      track_node *src = &(track[res.worker.src_num]);
      track_node *dest = &(track[res.worker.dest_num]);

      bool *reserved = res.worker.reserved;

      track_node *intermediate;

      int result = dijkstras_min_length(track, src, dest, prev1, prev2,
                                        &intermediate, reserved, MIN_DIST);

      char debug_buffer[100];
      sprintf(debug_buffer, "A path should exist between [%s] and [%s]\r\n",
              src->name, dest->name);
      KASSERT(result != -1, debug_buffer);

      track_node *path1[TRACK_MAX];
      track_node *path2[TRACK_MAX];

      memset(path1, NULL, sizeof(track_node *) * TRACK_MAX);
      memset(path2, NULL, sizeof(track_node *) * TRACK_MAX);

      unsigned int path_len = 2;
      unsigned int path_dist = result;

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
        req.worker.path[i] = node;
        node = path1[node - track];
        partial_len++;

        if (node == NULL) {
          break;
        }
      }

      node = intermediate;

      for (unsigned int i = 0; i < path_len; i++) {
        req.worker.path[i + partial_len] = node;
        node = path2[node - track];

        if (node == NULL) {
          break;
        }
      }

      req.type = CONTROL_WORKER_DONE;
      req.worker.type = WORKER_PATHFIND_GOOD;
      req.worker.whomfor = res.worker.whomfor;
      req.worker.path_dist = path_dist + res.worker.offset;
      req.worker.path_len = path_len;

      // printf(BW_COM2, "worker good\r\n");
      Send(parent, (char *)&req, sizeof(req), (char *)&res, 0);
    }
  }
}

void control_server() {

  RegisterAs("controlserver");

  task_tid worker = Create(10, control_worker);

  controlserver_request req;
  controlserver_response res;
  memset(&res, 0, sizeof(controlserver_response));
  task_tid client;

  memset(&res, 0, sizeof(res));

  bool worker_parked = false;

  controlserver_client_task task_backing;
  controlserver_client_task *task = NULL;

  bool node_reserved[TRACK_MAX];
  memset(node_reserved, 0, sizeof(bool) * TRACK_MAX);

  for (;;) {
    Receive(&client, (char *)&req, sizeof(controlserver_request));

    if (req.type == PATHFIND) {
      int src_num = req.client.src;
      int dest_num = req.client.dest;
      int offset = req.client.offset;

      if (worker_parked) {
        res.type = WORKER_PATHFIND;
        res.worker.src_num = src_num;
        res.worker.dest_num = dest_num;
        res.worker.whomfor = client;
        res.worker.offset = offset;
        memcpy(res.worker.reserved, node_reserved, sizeof(bool) * TRACK_MAX);
        worker_parked = false;
        Reply(worker, (char *)&res, sizeof(controlserver_response));
      } else {
        task = &task_backing;
        task->type = TASK_PATHFIND;
        task->pathfind.src_num = src_num;
        task->pathfind.dest_num = dest_num;
        task->pathfind.client = client;
        task->pathfind.offset = offset;
      }
    } else if (req.type == CONTROL_WORKER) {

      if (task == NULL) {
        worker_parked = true;
      } else {
        res.type = WORKER_PATHFIND;
        res.worker.whomfor = task->pathfind.client;
        res.worker.src_num = task->pathfind.src_num;
        res.worker.dest_num = task->pathfind.dest_num;
        res.worker.offset = task->pathfind.offset;
        memcpy(res.worker.reserved, node_reserved, sizeof(bool) * TRACK_MAX);
        task = NULL;
        Reply(worker, (char *)&res, sizeof(controlserver_response));
      }
    } else if (req.type == CONTROL_WORKER_DONE) {

      if (req.worker.type == WORKER_PATHFIND_GOOD) {
        res.type = CONTROLSERVER_GOOD;
        memcpy(res.client.path, req.worker.path, sizeof(int) * TRACK_MAX * 2);
        res.client.path_dist = req.worker.path_dist;
        res.client.path_len = req.worker.path_len;
      }
      Reply(req.worker.whomfor, (char *)&res, sizeof(controlserver_response));
      Reply(worker, (char *)&res, 0);
    } else {
      KASSERT(0, "Shouldnt be here in control server");
    }
  }
}
