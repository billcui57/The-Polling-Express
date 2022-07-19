#include "navigationserver.h"

v_train_num reserved_nodes[TRACK_MAX];
task_tid reservation_client;
task_tid pathprint_client;
int dests[MAX_NUM_TRAINS];
int srcs[MAX_NUM_TRAINS];

typedef struct straightpath_task_t {
  v_train_num train;
  int speed;
  int path[TRACK_MAX / 5];
  int path_len;
  int path_dist;
} straightpath_task_t;

typedef enum {
  IDLE,
  PATHFINDING,
  STRAIGHTPATHING,
} train_navigation_state;

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
  int delay_time;
} pathfind_task_t;

#define MAX_NAVIGATION_TASKS 10

void add_navigation_task(circular_buffer *navigation_tasks,
                         circular_buffer *segment, int train_speed,
                         int train_num, int offset) {
  navigation_task_t task;

  int segment_path[segment->count];
  cb_to_array(segment, (void *)segment_path);

  char debug_buffer[MAX_DEBUG_STRING_LEN];
  sprintf(debug_buffer, "[Segmenting] [%s] -> [%s]",
          track[segment_path[0]].name,
          track[segment_path[segment->count - 1]].name);
  debugprint(debug_buffer, 1);

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

#define TRAIN_LEN 250

void segments_fill_navigation_tasks(track_node *track, int *path, int path_len,
                                    circular_buffer *navigation_tasks,
                                    int train_speed, v_train_num train_num,
                                    int offset) {

  int segment_backing[TRACK_MAX];
  circular_buffer segment;
  cb_init(&segment, (void *)segment_backing, TRACK_MAX, sizeof(int));

  for (int i = 0; i < path_len; i++) {

    // char debug_buffer[MAX_DEBUG_STRING_LEN];
    // sprintf(debug_buffer, "%s", track[path[i]].name);
    // debugprint(debug_buffer);

    if (track[path[i]].reverse == &(track[path[i + 1]])) {
      // debugprint("found reverse");
      bool terminate = false;
      if (segment.count >= 1) {
        cb_push_back(&segment, (void *)&(path[i]), false);
        if (i == path_len - 2) {
          // debugprint("terminate");
          add_navigation_task(navigation_tasks, &segment, train_speed,
                              train_num, offset * -1);
          terminate = true;
        } else {
          // debugprint("reverse");
          add_navigation_task(navigation_tasks, &segment, train_speed,
                              train_num, TRAIN_LEN);
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
      // debugprint("straight");
      cb_push_back(&segment, (void *)&(path[i]), false);
      if (i == path_len - 1) {
        add_navigation_task(navigation_tasks, &segment, train_speed, train_num,
                            offset);
        cb_clear(&segment);
        // debugprint("end");
        break;
      }
    }
  }
}

void give_pathfinder_work(task_tid pathfind_worker, pathfind_task_t *task) {
  navigationserver_response res;
  memset(&res, 0, sizeof(navigationserver_response));
  res.type = PATHFIND_WORKER_HERES_WORK;
  res.data.pathfindworker.dest = task->destination_num;
  res.data.pathfindworker.src = task->source_num;
  res.data.pathfindworker.train = task->train;
  res.data.pathfindworker.delay_time = task->delay_time;

  for (int i = 0; i < TRACK_MAX; i++) {
    if (reserved_nodes[i] == -1) {
      res.data.pathfindworker.reserved_nodes[i] = false;
    } else {
      res.data.pathfindworker.reserved_nodes[i] = true;
    }
  }
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

bool can_reserve_path(int *path, int path_len) {
  for (int i = 0; i < path_len; i++) {
    if (reserved_nodes[path[i]] != -1) {
      return false;
    }
  }
  return true;
}

void update_reservation_printer() {
  if (reservation_client != -1) {
    navigationserver_response res;
    debugprint("Reply to reservation printer", 10);
    memset(&res, 0, sizeof(navigationserver_response));
    res.type = NAVIGATIONSERVER_GOOD;
    memcpy(res.data.get_reservations.reservations, reserved_nodes,
           sizeof(v_train_num) * TRACK_MAX);
    Reply(reservation_client, (char *)&res, sizeof(navigationserver_response));
    reservation_client = -1;
  }
}

void update_path_printer() {
  if (pathprint_client != -1) {
    navigationserver_response res;
    debugprint("Reply to path printer", 10);
    memset(&res, 0, sizeof(navigationserver_response));
    res.type = NAVIGATIONSERVER_GOOD;
    memcpy(res.data.get_path_display_info.dest_num, dests,
           sizeof(int) * MAX_NUM_TRAINS);
    memcpy(res.data.get_path_display_info.src_num, srcs,
           sizeof(int) * MAX_NUM_TRAINS);
    Reply(pathprint_client, (char *)&res, sizeof(navigationserver_response));
    pathprint_client = -1;
  }
}

void reserve_path(int *path, int path_len, v_train_num train_num) {

  char debug_buffer[MAX_DEBUG_STRING_LEN];
  sprintf(debug_buffer, "[Navigation] Reserving track for train %d",
          v_p_train_num(train_num));
  debugprint(debug_buffer, 1);

  for (int i = 0; i < path_len; i++) {
    reserved_nodes[path[i]] = train_num;
    char debug_buffer[MAX_DEBUG_STRING_LEN];
    sprintf(debug_buffer, "[Reservation] Reserved [%s] for train %d",
            track[path[i]].name, v_p_train_num(train_num));
    debugprint(debug_buffer, 1);
  }

  update_reservation_printer();
}

void clear_reserved_by_train(v_train_num train_num) {
  char debug_buffer[MAX_DEBUG_STRING_LEN];
  sprintf(debug_buffer, "[Navigation] Freeing reservations for train %d",
          v_p_train_num(train_num));
  debugprint(debug_buffer, 1);

  bool cleared = false;
  for (int i = 0; i < TRACK_MAX; i++) {
    if (reserved_nodes[i] == train_num) {
      cleared = true;
      reserved_nodes[i] = -1;
      sprintf(debug_buffer, "[Reservation] Freed [%s] from train %d",
              track[i].name, v_p_train_num(train_num));
      debugprint(debug_buffer, 1);
    }
  }
  if (cleared) {
    update_reservation_printer();
  }
}

// returns true if should redo pathfinding
bool process_straightpath_task(v_train_num train_num,
                               task_tid *straightpath_workers_parking,
                               straightpath_task_t *task) {
  clear_reserved_by_train(train_num);
  if (can_reserve_path(task->path, task->path_len)) {
    reserve_path(task->path, task->path_len, train_num);
    char debug_buffer[MAX_DEBUG_STRING_LEN];
    sprintf(debug_buffer,
            "[Navigation] Sending next straight path command for train %d",
            v_p_train_num(train_num));
    debugprint(debug_buffer, 1);
    give_straightpath_work(straightpath_workers_parking[train_num], task);
    straightpath_workers_parking[train_num] = -1;
  } else {
    char debug_buffer[MAX_DEBUG_STRING_LEN];
    sprintf(debug_buffer, "[Reservation] Cannot reserve for train %d yet",
            v_p_train_num(train_num));
    debugprint(debug_buffer, 1);
    return true;
  }
  return false;
}

bool process_next_task(circular_buffer *navigation_tasks,
                       task_tid trainserver_tid, task_tid timer_tid,
                       v_train_num train_num,
                       task_tid *straightpath_workers_parking) {
  navigation_task_t task;
  cb_pop_front(navigation_tasks, (void *)&task);

  bool should_restart = false;

  if (task.type == NAVIGATION_REVERSE) {
    debugprint("[Navigation] Reversing", 1);

    HardReverse(trainserver_tid, train_num, Time(timer_tid));

    if (!cb_is_empty(navigation_tasks)) {
      cb_pop_front(navigation_tasks, (void *)&task);
      should_restart =
          process_straightpath_task(train_num, straightpath_workers_parking,
                                    &(task.data.straightpath_task));
    }
  } else {
    should_restart =
        process_straightpath_task(train_num, straightpath_workers_parking,
                                  &(task.data.straightpath_task));
  }

  return should_restart;
}

void restart_pathfind(train_navigation_state *state,
                      circular_buffer *navigation_tasks,
                      pathfind_task_t *pathfind_task, v_train_num train_num) {
  char debug_buffer[MAX_DEBUG_STRING_LEN];
  sprintf(debug_buffer, "[Pathfind] Restarting pathfind for train %d",
          v_p_train_num(train_num));
  debugprint(debug_buffer, 1);
  *state = PATHFINDING;
  cb_clear(navigation_tasks);
  pathfind_task->destination_num = dests[train_num];
  pathfind_task->source_num = srcs[train_num];
  pathfind_task->train = train_num;
  pathfind_task->is_valid = true;
  pathfind_task->delay_time = 100;
}
void navigation_server() {

  for (int i = 0; i < TRACK_MAX; i++) {
    reserved_nodes[i] = -1;
  }

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
    char task_name_buffer[MAX_TASK_NAME_LEN];
    sprintf(task_name_buffer, "StraightPathWorker%d", v_p_train_num(train_num));
    straightpath_workers[train_num] =
        Create(10, task_name_buffer, task_straightpathworker);
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
    char task_name_buffer[MAX_TASK_NAME_LEN];
    sprintf(task_name_buffer, "PathFindWorker%d", v_p_train_num(train_num));
    pathfind_workers[train_num] = Create(10, task_name_buffer, pathfind_worker);
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

  for (int i = 0; i < MAX_NUM_TRAINS; i++) {
    srcs[i] = -1;
    dests[i] = -1;
  }

  navigationserver_request req;
  navigationserver_response res;
  memset(&res, 0, sizeof(navigationserver_response));

  task_tid client;

  task_tid clients[MAX_NUM_TRAINS];
  memset(clients, -1, sizeof(task_tid) * MAX_NUM_TRAINS);

  reservation_client = -1;
  pathprint_client = -1;

  for (;;) {

    Receive(&client, (char *)&req, sizeof(navigationserver_request));

    if (req.type == NAVIGATION_REQUEST) {

      v_train_num train_num = req.data.navigation_request.train;

      char debug_buffer[MAX_DEBUG_STRING_LEN];
      sprintf(debug_buffer,
              "[Navigation Server] Got navigation request from train %d",
              v_p_train_num(train_num));
      debugprint(debug_buffer, 1);

      if (states[train_num] != IDLE) {
        memset(&res, 0, sizeof(navigationserver_response));
        res.type = NAVIGATIONSERVER_BUSY;
        Reply(client, (char *)&res, sizeof(nameserver_response));
        continue;
      }

      if (srcs[train_num] == -1) {
        memset(&res, 0, sizeof(navigationserver_response));
        res.type = NAVIGATIONSERVER_NEED_REGISTER;
        Reply(client, (char *)&res, sizeof(nameserver_response));
        continue;
      }

      states[train_num] = PATHFINDING;
      clients[train_num] = client;

      pathfind_task_t task;
      task.destination_num = req.data.navigation_request.destination_num;
      task.source_num = srcs[train_num];
      task.train = req.data.navigation_request.train;
      task.is_valid = true;
      task.delay_time = 0;
      train_speeds[train_num] = req.data.navigation_request.speed;
      dests[train_num] = task.destination_num;
      update_path_printer();
      offsets[train_num] = req.data.navigation_request.offset;

      if (pathfind_workers_parking[train_num] != -1) {
        sprintf(
            debug_buffer,
            "[Navigation Server] Giving work to pathfind worker for train %d",
            v_p_train_num(train_num));
        debugprint(debug_buffer, 1);
        give_pathfinder_work(pathfind_workers_parking[train_num], &task);
        pathfind_workers_parking[train_num] = -1;
      } else {
        pathfind_tasks[train_num] = task;
      }

    } else if (req.type == PATHFIND_WORKER) {

      v_train_num train_num = req.data.pathfindworker.train_num;

      char debug_buffer[MAX_DEBUG_STRING_LEN];
      sprintf(debug_buffer,
              "[Navigation Server] Got pathfind worker for train %d",
              v_p_train_num(train_num));
      debugprint(debug_buffer, 1);

      pathfind_workers_parking[train_num] = client;

      if ((states[train_num] != PATHFINDING) ||
          !pathfind_tasks[train_num].is_valid) {
        continue;
      }

      sprintf(debug_buffer,
              "[Navigation Server] Giving work to pathfind worker for train %d",
              v_p_train_num(train_num));
      debugprint(debug_buffer, 1);
      give_pathfinder_work(pathfind_workers_parking[train_num],
                           &(pathfind_tasks[train_num]));
      pathfind_workers_parking[train_num] = -1;
      pathfind_tasks[train_num].is_valid = false;

    } else if (req.type == PATHFIND_WORKER_DONE) {

      memset(&res, 0, sizeof(navigationserver_response));
      res.type = NAVIGATIONSERVER_GOOD;
      Reply(client, (char *)&res, sizeof(navigationserver_response));

      v_train_num train_num = req.data.pathfindworker_done.train_num;

      char debug_buffer[MAX_DEBUG_STRING_LEN];
      sprintf(debug_buffer,
              "[Navigation Server] Pathfind worker done for train %d",
              v_p_train_num(train_num));
      debugprint(debug_buffer, 1);

      states[train_num] = STRAIGHTPATHING;

      pathfind_result_t found_path =
          req.data.pathfindworker_done.pathfind_result;
      int *path = req.data.pathfindworker_done.path;
      int path_len = req.data.pathfindworker_done.path_len;

      if (clients[train_num] != -1) {
        memset(&res, 0, sizeof(navigationserver_response));
        if (found_path == NO_PATH_AT_ALL) {
          res.type = NAVIGATIONSERVER_NO_PATH;
          Reply(clients[train_num], (char *)&res,
                sizeof(navigationserver_response));
          continue;
        } else {
          res.type = NAVIGATIONSERVER_GOOD;
          Reply(clients[train_num], (char *)&res,
                sizeof(navigationserver_response));
        }
        clients[train_num] = -1;
      }

      if (found_path == NO_PATH_WITH_RESERVE) {
        restart_pathfind(&(states[train_num]), &(navigation_tasks[train_num]),
                         &(pathfind_tasks[train_num]), train_num);
        continue;
      }

      segments_fill_navigation_tasks(
          track, path, path_len, &(navigation_tasks[train_num]),
          train_speeds[train_num], train_num, offsets[train_num]);

      if (straightpath_workers_parking[train_num] != -1) {
        process_next_task(&(navigation_tasks[train_num]), trainserver_tid,
                          timer_tid, train_num, straightpath_workers_parking);
      }

    } else if (req.type == STRAIGHTPATH_WORKER) {
      v_train_num train_num = req.data.straightpathworker.train_num;

      char debug_buffer[MAX_DEBUG_STRING_LEN];
      sprintf(debug_buffer,
              "[Navigation Server] Got straightpath worker for train %d",
              v_p_train_num(train_num));
      debugprint(debug_buffer, 1);

      straightpath_workers_parking[train_num] = client;

      if ((states[train_num] != STRAIGHTPATHING) ||
          cb_is_empty(&(navigation_tasks[train_num]))) {
        continue;
      }

      bool should_restart_pathfind =
          process_next_task(&(navigation_tasks[train_num]), trainserver_tid,
                            timer_tid, train_num, straightpath_workers_parking);
      if (should_restart_pathfind) {
        restart_pathfind(&(states[train_num]), &(navigation_tasks[train_num]),
                         &(pathfind_tasks[train_num]), train_num);
        continue;
      }
    }

    else if (req.type == STRAIGHTPATH_WORKER_DONE) {
      v_train_num train_num = req.data.straightpathworker_done.train_num;

      char debug_buffer[MAX_DEBUG_STRING_LEN];
      sprintf(debug_buffer,
              "[Navigation Server] Got straightpath worker done for train %d",
              v_p_train_num(train_num));
      debugprint(debug_buffer, 1);

      if (cb_is_empty(&(navigation_tasks[train_num]))) {
        clear_reserved_by_train(train_num);
        states[train_num] = IDLE;
        srcs[train_num] = dests[train_num];
        update_path_printer();
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
    } else if (req.type == GET_RESERVATIONS) {
      debugprint("Got reservation printer", 1);
      reservation_client = client;
    } else if (req.type == GET_PATH_DISPLAY_INFO) {
      debugprint("Got path printer", 1);
      pathprint_client = client;
    } else if (req.type == REGISTER_LOCATION) {
      debugprint("Got register location", 1);
      v_train_num train_num = req.data.register_location.train_num;
      srcs[train_num] = req.data.register_location.node_num;
      memset(&res, 0, sizeof(navigationserver_response));
      res.type = NAVIGATIONSERVER_GOOD;
      Reply(client, (char *)&res, sizeof(navigationserver_response));
      update_path_printer();
    }
  }
}
