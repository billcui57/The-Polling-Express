#include "navigationserver.h"

typedef struct straightpath_task_t {
  v_train_num train;
  int speed;
  int *path;
  int path_len;
  int offset;
} straightpath_task_t;

typedef struct pathfind_task_t {
  bool is_valid;
  v_train_num train;
  char source_num;
  char destination_num;
  int offset;
} pathfind_task_t;

#define MAX_STRAIGHTPATH_TASKS 5

int segments_fill_straightpath_tasks(track_node *track, int *path, int path_len,
                                     circular_buffer *straightpath_tasks,
                                     int *train_speeds) {}

void give_pathfinder_work(task_tid pathfind_worker, pathfind_task_t *task,
                          bool *reserved_nodes) {
  navigationserver_response res;
  memset(&res, 0, sizeof(navigationserver_response));
  res.type = PATHFIND_WORKER_HERES_WORK;
  res.data.pathfindworker.dest = task->destination_num;
  res.data.pathfindworker.offset = task->offset;
  res.data.pathfindworker.src = task->source_num;
  res.data.pathfindworker.train = task->train;
  memcpy(res.data.pathfindworker.reserved_nodes, reserved_nodes,
         sizeof(bool) * TRACK_MAX);
  Reply(pathfind_worker, (char *)&res, sizeof(navigationserver_response));
}

void give_straightpath_work(task_tid straightpath_worker,
                            straightpath_task_t *task) {
  navigationserver_response res;
  memset(&res, 0, sizeof(navigationserver_response));
  memcpy(res.data.straightpathworker.path, task->path,
         sizeof(int) * task->path_len);
  res.data.straightpathworker.offset = task->offset;
  res.data.straightpathworker.path_len = task->path_len;
  res.data.straightpathworker.speed = task->speed;
  res.data.straightpathworker.train = task->train;
  res.type = STRAIGHTPATH_WORKER_HERES_WORK;
  Reply(straightpath_worker, (char *)&res, sizeof(navigationserver_response));
}

void navigation_server() {

  typedef enum {
    IDLE,
    PATHFINDING,
    STRAIGHTPATHING,
  } train_navigation_state;

  train_navigation_state states[MAX_NUM_TRAINS];
  for (v_train_num train_num = 0; train_num < MAX_NUM_TRAINS; train_num++) {
    states[train_num] = IDLE;
  }

  RegisterAs("navigationserver");

  task_tid straightpath_workers_parking[MAX_NUM_TRAINS];
  for (v_train_num train_num = 0; train_num < MAX_NUM_TRAINS; train_num++) {
    straightpath_workers_parking[train_num] = -1;
    Create(10, task_straightpathworker);
  }

  straightpath_task_t straightpath_tasks_backing[MAX_NUM_TRAINS]
                                                [MAX_STRAIGHTPATH_TASKS];
  circular_buffer straightpath_tasks[MAX_NUM_TRAINS];
  for (v_train_num train_num = 0; train_num < MAX_NUM_TRAINS; train_num++) {
    cb_init(&(straightpath_tasks[0]),
            (void *)straightpath_tasks_backing[train_num],
            MAX_STRAIGHTPATH_TASKS, sizeof(straightpath_task_t));
  }

  task_tid pathfind_workers_parking[MAX_NUM_TRAINS];
  for (v_train_num train_num = 0; train_num < MAX_NUM_TRAINS; train_num++) {
    pathfind_workers_parking[train_num] = -1;
    Create(10, pathfind_worker);
  }
  pathfind_task_t pathfind_tasks[MAX_NUM_TRAINS];
  memset(pathfind_tasks, 0, sizeof(pathfind_task_t) * MAX_NUM_TRAINS);
  for (v_train_num train_num = 0; train_num < MAX_NUM_TRAINS; train_num++) {
    pathfind_tasks[train_num].is_valid = false;
  }

  int train_speeds[MAX_NUM_TRAINS];
  memset(train_speeds, 0, sizeof(int) * MAX_NUM_TRAINS);

  navigationserver_request req;
  navigationserver_response res;
  memset(&res, 0, sizeof(navigationserver_response));

  bool reserved_nodes[TRACK_MAX];
  memset(reserved_nodes, 0, sizeof(bool) * TRACK_MAX);

  task_tid client;

  for (;;) {

    Receive(&client, &req, sizeof(navigationserver_request));

    if (req.type == NAVIGATION_REQUEST) {

      v_train_num train_num = req.data.navigation_request.train;

      if (states[train_num] != IDLE) {
        memset(&res, 0, sizeof(navigationserver_response));
        res.type = NAVIGATIONSERVER_BUSY;
        Reply(client, (char *)&res, sizeof(nameserver_response));
        continue;
      }

      states[train_num] = PATHFINDING;

      pathfind_task_t task;
      task.destination_num = req.data.navigation_request.destination_num;
      task.offset = req.data.navigation_request.offset;
      task.source_num = req.data.navigation_request.source_num;
      task.train = req.data.navigation_request.train;
      task.is_valid = true;
      train_speeds[train_num] = req.data.navigation_request.speed;

      if (pathfind_workers_parking[train_num] != -1) {
        give_pathfinder_work(pathfind_workers_parking[train_num], &task,
                             reserved_nodes);
        pathfind_workers_parking[train_num] = -1;
      } else {
        pathfind_tasks[train_num] = task;
      }

      memset(&res, 0, sizeof(navigationserver_response));
      res.type = NAVIGATIONSERVER_GOOD;
      Reply(client, (char *)&res, sizeof(navigationserver_response));

    } else if (req.type == PATHFIND_WORKER) {

      v_train_num train_num = req.data.pathfindworker.train_num;
      pathfind_workers_parking[train_num] = client;

      if ((states[train_num] != PATHFINDING) ||
          !pathfind_tasks[train_num].is_valid) {
        continue;
      }

      give_pathfinder_work(pathfind_workers_parking[train_num],
                           &(pathfind_tasks[train_num]), reserved_nodes);
      pathfind_workers_parking[train_num] = -1;
      pathfind_tasks[train_num].is_valid = false;

    } else if (req.type == PATHFIND_WORKER_DONE) {

      memset(&res, 0, sizeof(navigationserver_response));
      res.type = NAVIGATIONSERVER_GOOD;
      Reply(client, (char *)&res, sizeof(navigationserver_response));

      v_train_num train_num = req.data.pathfindworker_done.train_num;

      states[train_num] = STRAIGHTPATHING;

      int *path = req.data.pathfindworker_done.path;
      int path_len = req.data.pathfindworker_done.path_len;

      // TODO: reservation

      segments_fill_straightpath_tasks(track, path, path_len,
                                       &(straightpath_tasks[train_num]),
                                       train_speeds);

      if (straightpath_workers_parking[train_num] != -1) {
        straightpath_task_t first_task;
        cb_pop_front(&(straightpath_tasks[train_num]), (void *)&first_task);
        give_straightpath_work(straightpath_workers_parking[train_num],
                               &first_task);
        straightpath_workers_parking[train_num] = -1;
      }

    } else if (req.type == STRAIGHTPATH_WORKER) {
      v_train_num train_num = req.data.straightpathworker.train_num;

      straightpath_workers_parking[train_num] = client;

      if ((states[train_num] != STRAIGHTPATHING) ||
          cb_is_empty(&(straightpath_tasks[train_num]))) {
        continue;
      }

      straightpath_task_t next_task;
      cb_pop_front(&(straightpath_tasks[train_num]), (void *)&next_task);

      give_straightpath_work(straightpath_workers_parking[train_num],
                             &next_task);
      straightpath_workers_parking[train_num] = -1;
    }

    else if (req.type == STRAIGHTPATH_WORKER_DONE) {
      v_train_num train_num = req.data.straightpathworker_done.train_num;
      if (cb_is_empty(&(straightpath_tasks[train_num]))) {
        states[train_num] = IDLE;
        continue;
      }
      memset(&res, 0, sizeof(navigationserver_response));
      res.type = NAVIGATIONSERVER_GOOD;
      Reply(client, (char *)&res, sizeof(navigationserver_response));
    }
  }
}

// void navigation_server() {

//   RegisterAs("navigationserver");

//   for (v_train_num train_num = 0; train_num < MAX_NUM_TRAINS; train_num++) {
//     Create(10, task_straightpathworker);
//   }

//   straightpath_task_t straightpath_tasks_backing[MAX_NUM_TRAINS]
//                                                 [MAX_STRAIGHTPATH_TASKS];
//   circular_buffer straightpath_tasks[MAX_NUM_TRAINS];
//   for (v_train_num train_num = 0; train_num < MAX_NUM_TRAINS; train_num++) {
//     cb_init(&(straightpath_tasks[0]),
//             (void *)straightpath_tasks_backing[train_num],
//             MAX_STRAIGHTPATH_TASKS, sizeof(straightpath_task_t));
//   }

//   task_tid pathfind_workers[MAX_NUM_TRAINS];
//   for (v_train_num train_num = 0; train_num < MAX_NUM_TRAINS; train_num++) {
//     pathfind_workers[train_num] = Create(10, pathfind_worker);
//   }
//   pathfind_task_t pathfind_tasks[MAX_NUM_TRAINS];

//   bool pathfind_worker_parked[MAX_NUM_TRAINS];
//   bool straightpath_worker_parked[MAX_NUM_TRAINS];
//   for (v_train_num train_num = 0; train_num < MAX_NUM_TRAINS; train_num++) {
//     pathfind_worker_parked[train_num] = false;
//     straightpath_worker_parked[train_num] = false;
//   }

//   int train_speeds[MAX_NUM_TRAINS];
//   memset(train_speeds, 0, sizeof(int) * MAX_NUM_TRAINS);

//   navigationserver_request req;
//   navigationserver_response res;
//   memset(&res, 0, sizeof(navigationserver_response));

//   task_tid client;

//   for (;;) {

//     Receive(&client, &req, sizeof(navigationserver_request));

//     if (req.type == NAVIGATION_REQUEST) {

//       v_train_num train_num = req.data.navigation_request.train;

//       if (!pathfind_worker_parked[train_num] ||
//           !straightpath_worker_parked[train_num]) {
//         memset(&res, 0, sizeof(navigationserver_response));
//         res.type = NAVIGATIONSERVER_BUSY;
//         Reply(client, &res, sizeof(nameserver_response));
//         continue;
//       }

//       memset(&res, 0, sizeof(navigationserver_response));
//       res.type = PATHFIND_WORKER_HERES_WORK;
//       res.data.pathfindworker.offset = req.data.navigation_request.offset;
//       res.data.pathfindworker.train = train_num;
//       res.data.pathfindworker.dest =
//           req.data.navigation_request.destination_num;
//       res.data.pathfindworker.src = req.data.navigation_request.source_num;
//       train_speeds[train_num] = req.data.navigation_request.speed;

//       pathfind_worker_parked[train_num] = false;

//       Reply(pathfind_workers[train_num], &res,
//             sizeof(navigationserver_response));

//     } else if (req.type == PATHFIND_WORKER) {
//       v_train_num train_num = req.data.pathfindworker.train_num;
//       pathfind_worker_parked[train_num] = true;
//     } else if (req.type == PATHFIND_WORKER_DONE) {
//       v_train_num train_num = req.data.pathfindworker_done.train_num;
//       int *path = req.data.pathfindworker_done.path;
//       int path_len = req.data.pathfindworker_done.path_len;

//       segments_fill_straightpath_tasks(track, path, path_len,
//                                        &(straightpath_tasks[train_num]));
//       // break it up in to segments and add to straightpath cb
//     }
//   }
// }
