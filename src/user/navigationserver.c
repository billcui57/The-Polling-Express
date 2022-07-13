#include "navigationserver.h"

typedef struct straightpath_task_t {
  v_train_num train;
  int speed;
  int path[TRACK_MAX / 5];
  int path_len;
  int path_dist;
} straightpath_task_t;

typedef struct navigation_task_t {
  enum { NAVIGATION_REVERSE, NAVIGATION_STRAIGHT } type;
  union {
    struct straightpath_task_t straightpath_task;
  } data;
} navigation_task_t;

typedef struct pathfind_task_t {
  bool is_valid;
  v_train_num train;
  char source_num;
  char destination_num;
} pathfind_task_t;

#define MAX_NAVIGATION_TASKS 10

void add_navigation_task(circular_buffer *navigation_tasks,
                         circular_buffer *segment, int train_speed,
                         int train_num, int offset) {
  navigation_task_t task;

  int segment_path[segment->count];
  cb_to_array(segment, (void *)segment_path);

  memset(&task, 0, sizeof(navigation_task_t));
  memcpy(task.data.straightpath_task.path, segment_path,
         sizeof(int) * segment->count);
  task.type = NAVIGATION_STRAIGHT;
  task.data.straightpath_task.path_len = segment->count;
  task.data.straightpath_task.speed = train_speed;
  task.data.straightpath_task.train = train_num;
  task.data.straightpath_task.path_dist =
      get_path_dist(track, segment_path, segment->count) + offset;
  cb_push_back(navigation_tasks, (void *)&task, false);
}

void segments_fill_navigation_tasks(track_node *track, int *path, int path_len,
                                    circular_buffer *navigation_tasks,
                                    int train_speed, v_train_num train_num,
                                    int offset) {

  int segment_backing[TRACK_MAX];
  circular_buffer segment;
  cb_init(&segment, (void *)segment_backing, TRACK_MAX, sizeof(int));

  for (int i = 0; i < path_len; i++) {

    if (track[path[i]].reverse == &(track[path[i + 1]])) {

      bool terminate = false;
      if (segment.count > 1) {
        cb_push_back(&segment, (void *)&(path[i]), false);
        if (i == path_len - 2) {
          add_navigation_task(navigation_tasks, &segment, train_speed,
                              train_num, offset * -1);
          terminate = true;
        } else {
          add_navigation_task(navigation_tasks, &segment, train_speed,
                              train_num, 0);
        }

        cb_clear(&segment);
      }

      navigation_task_t task;
      memset(&task, 0, sizeof(navigation_task_t));
      task.type = NAVIGATION_REVERSE;
      cb_push_back(navigation_tasks, (void *)&task, false);

      if (terminate) {
        break;
      }

    } else {

      cb_push_back(&segment, (void *)&(path[i]), false);
      if (i == path_len - 1) {
        add_navigation_task(navigation_tasks, &segment, train_speed, train_num,
                            offset);
        cb_clear(&segment);
        break;
      }
    }
  }
}

void give_pathfinder_work(task_tid pathfind_worker, pathfind_task_t *task,
                          bool *reserved_nodes) {
  navigationserver_response res;
  memset(&res, 0, sizeof(navigationserver_response));
  res.type = PATHFIND_WORKER_HERES_WORK;
  res.data.pathfindworker.dest = task->destination_num;
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
  res.data.straightpathworker.path_len = task->path_len;
  res.data.straightpathworker.speed = task->speed;
  res.data.straightpathworker.path_dist = task->path_dist;
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
  task_tid trainserver_tid = WhoIsBlock("trainctl");
  task_tid timer_tid = WhoIsBlock("clockserver");

  task_tid straightpath_workers[MAX_NUM_TRAINS];
  task_tid straightpath_workers_parking[MAX_NUM_TRAINS];
  for (v_train_num train_num = 0; train_num < MAX_NUM_TRAINS; train_num++) {
    straightpath_workers_parking[train_num] = -1;
    straightpath_workers[train_num] = Create(10, task_straightpathworker);
  }

  navigation_task_t navigation_tasks_backing[MAX_NUM_TRAINS]
                                            [MAX_NAVIGATION_TASKS];
  circular_buffer navigation_tasks[MAX_NUM_TRAINS];
  for (v_train_num train_num = 0; train_num < MAX_NUM_TRAINS; train_num++) {
    cb_init(&(navigation_tasks[train_num]),
            (void *)navigation_tasks_backing[train_num], MAX_NAVIGATION_TASKS,
            sizeof(navigation_task_t));
  }

  task_tid pathfind_workers[MAX_NUM_TRAINS];
  task_tid pathfind_workers_parking[MAX_NUM_TRAINS];
  for (v_train_num train_num = 0; train_num < MAX_NUM_TRAINS; train_num++) {
    pathfind_workers_parking[train_num] = -1;
    pathfind_workers[train_num] = Create(10, pathfind_worker);
  }
  pathfind_task_t pathfind_tasks[MAX_NUM_TRAINS];
  memset(pathfind_tasks, 0, sizeof(pathfind_task_t) * MAX_NUM_TRAINS);
  for (v_train_num train_num = 0; train_num < MAX_NUM_TRAINS; train_num++) {
    pathfind_tasks[train_num].is_valid = false;
  }

  int train_speeds[MAX_NUM_TRAINS];
  memset(train_speeds, 0, sizeof(int) * MAX_NUM_TRAINS);
  int offsets[MAX_NUM_TRAINS];
  memset(offsets, 0, sizeof(int) * MAX_NUM_TRAINS);

  navigationserver_request req;
  navigationserver_response res;
  memset(&res, 0, sizeof(navigationserver_response));

  bool reserved_nodes[TRACK_MAX];
  memset(reserved_nodes, 0, sizeof(bool) * TRACK_MAX);

  task_tid client;

  for (;;) {

    Receive(&client, (char *)&req, sizeof(navigationserver_request));

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
      task.source_num = req.data.navigation_request.source_num;
      task.train = req.data.navigation_request.train;
      task.is_valid = true;
      train_speeds[train_num] = req.data.navigation_request.speed;
      offsets[train_num] = req.data.navigation_request.offset;

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

      // memcpy(reserved_nodes,
      //        req.data.pathfindworker_done.updated_reserved_nodes,
      //        sizeof(bool) * TRACK_MAX);

      segments_fill_navigation_tasks(
          track, path, path_len, &(navigation_tasks[train_num]),
          train_speeds[train_num], train_num, offsets[train_num]);

      if (straightpath_workers_parking[train_num] != -1) {
        navigation_task_t task;
        cb_pop_front(&(navigation_tasks[train_num]), (void *)&task);

        if (task.type == NAVIGATION_REVERSE) {
          TrainCommand(trainserver_tid, Time(timer_tid), REVERSE, train_num, 0);
          if (!cb_is_empty(&(navigation_tasks[train_num]))) {
            cb_pop_front(&(navigation_tasks[train_num]), (void *)&task);
            give_straightpath_work(straightpath_workers_parking[train_num],
                                   &(task.data.straightpath_task));
            straightpath_workers_parking[train_num] = -1;
          }
        } else {
          give_straightpath_work(straightpath_workers_parking[train_num],
                                 &(task.data.straightpath_task));
          straightpath_workers_parking[train_num] = -1;
        }
      }

    } else if (req.type == STRAIGHTPATH_WORKER) {
      v_train_num train_num = req.data.straightpathworker.train_num;

      straightpath_workers_parking[train_num] = client;

      if ((states[train_num] != STRAIGHTPATHING) ||
          cb_is_empty(&(navigation_tasks[train_num]))) {
        continue;
      }

      navigation_task_t task;
      cb_pop_front(&(navigation_tasks[train_num]), (void *)&task);

      if (task.type == NAVIGATION_REVERSE) {
        TrainCommand(trainserver_tid, Time(timer_tid), REVERSE, train_num, 0);
        if (!cb_is_empty(&(navigation_tasks[train_num]))) {
          cb_pop_front(&(navigation_tasks[train_num]), (void *)&task);
          give_straightpath_work(straightpath_workers_parking[train_num],
                                 &(task.data.straightpath_task));
          straightpath_workers_parking[train_num] = -1;
        }
      } else {
        give_straightpath_work(straightpath_workers_parking[train_num],
                               &(task.data.straightpath_task));
        straightpath_workers_parking[train_num] = -1;
      }
    }

    else if (req.type == STRAIGHTPATH_WORKER_DONE) {
      v_train_num train_num = req.data.straightpathworker_done.train_num;
      if (cb_is_empty(&(navigation_tasks[train_num]))) {
        states[train_num] = IDLE;
      }
      memset(&res, 0, sizeof(navigationserver_response));
      res.type = NAVIGATIONSERVER_GOOD;
      Reply(client, (char *)&res, sizeof(navigationserver_response));
    } else if (req.type == WHOAMI) {
      memset(&res, 0, sizeof(navigationserver_response));

      bool found = false;

      for (v_train_num train_num = 0; train_num < MAX_NUM_TRAINS; train_num++) {
        if (req.data.whoami.worker_type == STRAIGHTPATH
                ? straightpath_workers[train_num] == client
                : pathfind_workers[train_num] == client) {
          found = true;
          res.type = NAVIGATIONSERVER_GOOD;
          res.data.whoami.train = train_num;
          Reply(client, (char *)&res, sizeof(navigationserver_response));
          break;
        }
      }

      if (!found) {
        KASSERT(0, "Invalid worker");
      }
    }
  }
}
