#include "controlserver.h"
#include <syscall.h>

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

      track_node *prev[TRACK_MAX];

      track_node *src = &(track[res.worker.src_num]);
      track_node *dest = &(track[res.worker.dest_num]);

      int result = dijkstra(track, src, dest, prev);

      if (result == -1) {
        req.type = CONTORL_WORKER_DONE;
        req.worker.type = WORKER_PATHFIND_NO_PATH;
        // printf(BW_COM2, "worker no path\r\n");
        Send(parent, (char *)&req, sizeof(req), (char *)&res, sizeof(res));
        continue;
      }

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
        req.worker.path[i] = node - track;
        node = path[node - track];
      }

      req.type = CONTORL_WORKER_DONE;
      req.worker.type = WORKER_PATHFIND_GOOD;
      req.worker.whomfor = res.worker.whomfor;
      req.worker.path_dist = path_dist;
      req.worker.path_len = path_len;

      // printf(BW_COM2, "worker good\r\n");
      Send(parent, (char *)&req, sizeof(req), (char *)&res, sizeof(res));
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

  bool worker_parked = true;

  controlserver_client_task task_backing;
  controlserver_client_task *task = NULL;

  for (;;) {
    Receive(&client, (char *)&req, sizeof(controlserver_request));

    if (req.type == PATHFIND) {
      int src_num = track_name_to_num(&track, req.client.src_name);
      int dest_num = track_name_to_num(&track, req.client.dest_name);

      if (worker_parked) {

        res.type = WORKER_PATHFIND;
        res.worker.src_num = src_num;
        res.worker.dest_num = dest_num;
        res.worker.whomfor = client;
        Reply(worker, (char *)&res, sizeof(controlserver_response));
      } else {
        task = &task_backing;
        task->type = TASK_PATHFIND;
        task->pathfind.dest_num = src_num;
        task->pathfind.src_num = dest_num;
        task->pathfind.client = client;
      }
    } else if (req.type == CONTROL_WORKER) {

      if (task == NULL) {
        worker_parked = true;
      } else {
        worker_parked = false;
        res.type = WORKER_PATHFIND;
        res.worker.whomfor = task->pathfind.client;
        res.worker.src_num = task->pathfind.src_num;
        res.worker.dest_num = task->pathfind.dest_num;
        task = NULL;
        Reply(worker, (char *)&res, sizeof(controlserver_response));
      }
    } else if (req.type == CONTORL_WORKER_DONE) {

      if (req.worker.type == WORKER_PATHFIND_GOOD) {
        res.type = CONTROLSERVER_GOOD;
        memcpy(res.client.path, req.worker.path, sizeof(int) * TRACK_MAX);
        res.client.path_dist = req.worker.path_dist;
        res.client.path_len = req.worker.path_len;
      } else if (req.worker.type == WORKER_PATHFIND_NO_PATH) {
        res.type = CONTROLSERVER_NO_PATH;
      }

      Reply(req.worker.whomfor, (char *)&res, sizeof(controlserver_response));
    } else {
      KASSERT(0, "Shouldnt be here in control server");
    }
  }
}
