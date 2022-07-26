#include "navigationserver.h"

typedef enum {
  IDLE,
  PATHFINDING,
  STRAIGHTPATHING,
} train_navigation_state;

train_navigation_state states[MAX_NUM_TRAINS];
v_train_num reserved_nodes[TRACK_MAX];
task_tid reservation_client;
task_tid trainstate_print_client;
bool reservation_dirty;
bool train_states_dirty;
train_state_t train_states[MAX_NUM_TRAINS];

typedef struct straightpath_task_t {
  v_train_num train;
  int path[TRACK_MAX / 5];
  int path_len;
  int path_dist;
  bool is_startup_task;
} straightpath_task_t;

typedef struct navigation_task_t {
  enum { NAVIGATION_REVERSE, NAVIGATION_STRAIGHT } type;
  union {
    struct straightpath_task_t straightpath_task;
  } data;
} navigation_task_t;

straightpath_task_t cur_straightpath_tasks[MAX_NUM_TRAINS];

typedef struct pathfind_task_t {
  bool is_valid;
  v_train_num train;
  int source_num;
  int destination_num;
  int delay_time;
} pathfind_task_t;

#define MAX_NAVIGATION_TASKS 10

void add_navigation_task(circular_buffer *navigation_tasks,
                         circular_buffer *segment, int train_num, int offset,
                         bool is_first_task) {
  navigation_task_t task;

  int segment_path[segment->count];
  cb_to_array(segment, (void *)segment_path);

  char debug_buffer[MAX_DEBUG_STRING_LEN];
  sprintf(debug_buffer, "Segmenting [%s] -> [%s]", track[segment_path[0]].name,
          track[segment_path[segment->count - 1]].name);
  debugprint(debug_buffer, NAVIGATION_SERVER_DEBUG);

  memset(&task, 0, sizeof(navigation_task_t));
  memcpy(task.data.straightpath_task.path, segment_path,
         sizeof(int) * segment->count);
  task.type = NAVIGATION_STRAIGHT;
  task.data.straightpath_task.path_len = segment->count;
  task.data.straightpath_task.train = train_num;
  task.data.straightpath_task.path_dist =
      get_path_dist(track, segment_path, segment->count) + offset;
  task.data.straightpath_task.is_startup_task = is_first_task;
  cb_push_back(navigation_tasks, (void *)&task, false);
}

#define TRAIN_LEN 250

// int buffer_nodes[][]={
//   {MR2,MR3,C7},//MR1
//   {MR3,C7},//MR2
//   {C7},//MR3
//   {MR12,MR11,C13},//MR4
//   {BR18,C8},//MR5
//   {C6},//MR6
//   {E11},//MR7
//   {BR9,D8}, //MR8
//   {BR8,D9}, //MR9
//   {E5}, //MR10
//   {C13},//MR11
//   {MR11,C13},//MR12
//   {C12}, //MR13
//   {A4}, //MR14
//   {B15}, //MR15
//   {C9},//MR16
//   {E14}, //MR17
//   {BR5,C3},//MR18
//   {MR154,BR156,E2}, //MR153
//   {BR156,BR155,D2}, //MR154
//   {MR156,BR154,B14}, //MR155
//   {BR154,BR153,C1}, //MR156
// }

// int buffer_nodes[22][4] = {
//     {3, 83, 85, 38},   // MR1
//     {2, 85, 38, -1},   // MR2
//     {1, 38, -1, -1},   // MR3
//     {3, 103, 101, 44}, // MR4
//     {2, 114, 39, -1},  // MR5
//     {1, 37, -1, -1},   // MR6
//     {1, 74, -1, -1},   // MR7
//     {2, 96, 55, -1},   // MR8
//     {2, 94, 56, -1},   // MR9
//     {1, 68, -1, -1},   // MR10
//     {1, 44, -1, -1},   // MR11
//     {2, 101, 44, -1},  // MR12
//     {1, 43, -1, -1},   // MR13
//     {1, 3, -1, -1},    // MR14
//     {1, 30, -1, -1},   // MR15
//     {1, 40, -1, -1},   // MR16
//     {1, 77, -1, -1},   // MR17
//     {2, 88, 34, -1},   // MR18
//     {3, 119, 122, 65}, // MR153
//     {3, 122, 120, 49}, // MR154
//     {3, 123, 118, 29}, // MR155
//     {3, 118, 116, 32}, // MR156
// };

void add_buffer_nodes(circular_buffer *segment, int switch_node_num,
                      bool forward) {
  int switch_index;

  if (track[switch_node_num].num < 153) {
    switch_index = track[switch_node_num].num - 1;
  } else {
    switch_index = track[switch_node_num].num - 153 + 18;
  }

  int num_buffer_nodes = buffer_nodes[switch_index][0];

  if (forward) {
    for (int i = 1; i < 1 + num_buffer_nodes; i++) {
      cb_push_back(segment, (void *)&(buffer_nodes[switch_index][i]), false);
    }
  } else {

    for (int i = num_buffer_nodes; i > 0; i--) {

      int flipped_first_node_num =
          (track[buffer_nodes[switch_index][i]].reverse) - track;

      cb_push_back(segment, (void *)&flipped_first_node_num, false);
    }
  }
}

void segments_fill_navigation_tasks(track_node *track, int *path, int path_len,
                                    circular_buffer *navigation_tasks,
                                    v_train_num train_num) {

  int segment_backing[TRACK_MAX];
  circular_buffer segment;
  cb_init(&segment, (void *)segment_backing, TRACK_MAX, sizeof(int));

  bool came_from_reverse = false;
  int merge_node_num = -1;
  bool had_first_task = false;

  for (int i = 0; i < path_len; i++) {

    if (track[path[i]].reverse == &(track[path[i + 1]])) {
      bool terminate = false;
      if (segment.count >= 1) {
        cb_push_back(&segment, (void *)&(path[i]), false);
        if (i == path_len - 2) {
          add_navigation_task(navigation_tasks, &segment, train_num,
                              train_states[train_num].dest_offset * -1,
                              !had_first_task);
          had_first_task = true;
          terminate = true;
        } else {
          merge_node_num = path[i];
          add_buffer_nodes(&segment, merge_node_num, true);
          came_from_reverse = true;
          add_navigation_task(navigation_tasks, &segment, train_num, 0,
                              !had_first_task);
          had_first_task = true;
        }

        cb_clear(&segment);
      } else {
        if (i == path_len - 2) {
          terminate = true;
        }
      }

      navigation_task_t task;
      memset(&task, 0, sizeof(navigation_task_t));
      task.type = NAVIGATION_REVERSE;
      cb_push_back(navigation_tasks, (void *)&task, false);

      if (terminate) {
        break;
      }

    } else {
      if (came_from_reverse) {
        // augment
        add_buffer_nodes(&segment, merge_node_num, false);
        came_from_reverse = false;
      }
      cb_push_back(&segment, (void *)&(path[i]), false);
      if (i == path_len - 1) {
        add_navigation_task(navigation_tasks, &segment, train_num,
                            train_states[train_num].dest_offset,
                            !had_first_task);
        had_first_task = true;
        cb_clear(&segment);
        break;
      }
    }
  }
}

void give_pathfinder_work(task_tid pathfind_worker, pathfind_task_t *task) {
  navigationserver_response res;
  memset(&res, 0, sizeof(navigationserver_response));
  res.type = PATHFIND_WORKER_HERES_WORK;
  res.data.pathfindworker.dest_num = task->destination_num;
  res.data.pathfindworker.src_num = task->source_num;
  res.data.pathfindworker.train = task->train;
  res.data.pathfindworker.delay_time = task->delay_time;

  for (int i = 0; i < TRACK_MAX; i++) {
    if ((reserved_nodes[i] == -1) || (reserved_nodes[i] == task->train)) {
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
  res.data.straightpathworker.path_dist =
      task->path_dist - train_states[task->train].reged_at_offset;
  res.data.straightpathworker.delay_time = task->is_startup_task ? 0 : 30;
  res.type = STRAIGHTPATH_WORKER_HERES_WORK;
  Reply(straightpath_worker, (char *)&res, sizeof(navigationserver_response));
}

bool can_reserve_segment(int *path, int path_len, v_train_num train_num) {
  for (int i = 0; i < path_len; i++) {

    if ((reserved_nodes[path[i]] != -1) &&
        (reserved_nodes[path[i]] != train_num)) {
      return false;
    }
  }
  return true;
}

void reserve_node(int node_num, v_train_num train_num) {
  char debug_buffer[MAX_DEBUG_STRING_LEN];
  sprintf(debug_buffer, "Reserved [%s] for train %d", track[node_num].name,
          v_p_train_num(train_num));
  debugprint(debug_buffer, RESERVATION_DEBUG);
  reserved_nodes[node_num] = train_num;

  int reverse_node_num = track[node_num].reverse - track;

  sprintf(debug_buffer, "Reserved [%s] for train %d",
          track[reverse_node_num].name, v_p_train_num(train_num));
  debugprint(debug_buffer, RESERVATION_DEBUG);

  reserved_nodes[reverse_node_num] = train_num;
  reservation_dirty = true;
}

void reserve_path(int *path, int path_len, v_train_num train_num) {

  char debug_buffer[MAX_DEBUG_STRING_LEN];
  sprintf(debug_buffer, "Reserving track for train %d",
          v_p_train_num(train_num));
  debugprint(debug_buffer, RESERVATION_DEBUG);

  for (int i = 0; i < path_len; i++) {
    reserve_node(path[i], train_num);
  }
}

void unreserve_node(int node_num, v_train_num train_num) {
  int last_at_node_num = train_states[train_num].reged_at_num;
  char debug_buffer[MAX_DEBUG_STRING_LEN];

  int reverse_node_num = track[node_num].reverse - track;

  if ((reserved_nodes[node_num] == train_num) &&
      (node_num != last_at_node_num) &&
      (reverse_node_num != last_at_node_num)) {
    reservation_dirty = true;
    reserved_nodes[node_num] = -1;
    sprintf(debug_buffer, "Freed [%s] from train %d", track[node_num].name,
            v_p_train_num(train_num));
    debugprint(debug_buffer, RESERVATION_DEBUG);
    reserved_nodes[reverse_node_num] = -1;
    sprintf(debug_buffer, "Freed [%s] from train %d",
            track[reverse_node_num].name, v_p_train_num(train_num));

    debugprint(debug_buffer, RESERVATION_DEBUG);
  }
}

void unreserve_path_src_up_to_node(int node_num, v_train_num train_num) {

  straightpath_task_t cur_straightpath_task = cur_straightpath_tasks[train_num];

  int *path = cur_straightpath_task.path;
  int path_len = cur_straightpath_task.path_len;
  for (int i = 0; i < path_len; i++) {
    unreserve_node(path[i], train_num);
    if (path[i] == node_num) {
      break;
    }
  }
}

void clear_reserved_by_train(v_train_num train_num) {
  char debug_buffer[MAX_DEBUG_STRING_LEN];
  sprintf(debug_buffer, "Freeing reservations for train %d",
          v_p_train_num(train_num));
  debugprint(debug_buffer, RESERVATION_DEBUG);

  for (int i = 0; i < TRACK_MAX; i++) {
    unreserve_node(i, train_num);
  }
}

// returns true if should redo pathfinding
bool process_straightpath_task(v_train_num train_num,
                               task_tid *straightpath_workers_parking,
                               straightpath_task_t *task) {
  memcpy((void *)&(cur_straightpath_tasks[train_num]), (void *)task,
         sizeof(straightpath_task_t));

  clear_reserved_by_train(train_num);
  if (can_reserve_segment(task->path, task->path_len, train_num)) {
    reserve_path(task->path, task->path_len, train_num);
    char debug_buffer[MAX_DEBUG_STRING_LEN];
    sprintf(debug_buffer, "Sending next straight path command for train %d",
            v_p_train_num(train_num));
    debugprint(debug_buffer, NAVIGATION_SERVER_DEBUG);
    give_straightpath_work(straightpath_workers_parking[train_num], task);
    straightpath_workers_parking[train_num] = -1;
    train_states[train_num].reged_at_num = task->path[task->path_len - 1];
    train_states[train_num].reged_at_offset = 0;
  } else {
    char debug_buffer[MAX_DEBUG_STRING_LEN];
    sprintf(debug_buffer, "Cannot reserve for train %d yet",
            v_p_train_num(train_num));
    debugprint(debug_buffer, NAVIGATION_SERVER_DEBUG);
    return true;
  }
  return false;
}

void done_navigation(v_train_num train_num) {
  clear_reserved_by_train(train_num);
  states[train_num] = IDLE;
}

// assumes there is a task
bool process_next_task(circular_buffer *navigation_tasks,
                       task_tid trainserver_tid, task_tid timer_tid,
                       v_train_num train_num,
                       task_tid *straightpath_workers_parking) {

  navigation_task_t task;
  cb_pop_front(navigation_tasks, (void *)&task);

  bool should_restart = false;

  if (task.type == NAVIGATION_REVERSE) {
    debugprint("Reversing", NAVIGATION_SERVER_DEBUG);

    HardReverse(trainserver_tid, train_num, Time(timer_tid));

    train_states[train_num].reversed = !train_states[train_num].reversed;

    train_states[train_num].reged_at_num =
        track[train_states->reged_at_num].reverse - track;
    // train_states[train_num].reged_at_offset = TRAIN_LEN;
    train_states[train_num].reged_at_offset = 0; // TODO: Should be TRAIN_LEN
    train_states_dirty = true;

    if (!cb_is_empty(navigation_tasks)) {
      cb_pop_front(navigation_tasks, (void *)&task);
      should_restart =
          process_straightpath_task(train_num, straightpath_workers_parking,
                                    &(task.data.straightpath_task));
    } else {
      done_navigation(train_num);
    }
  } else {
    should_restart =
        process_straightpath_task(train_num, straightpath_workers_parking,
                                  &(task.data.straightpath_task));
  }

  return should_restart;
}

void restart_pathfind(circular_buffer *navigation_tasks,
                      pathfind_task_t *pathfind_task, v_train_num train_num) {
  char debug_buffer[MAX_DEBUG_STRING_LEN];
  sprintf(debug_buffer, "Restarting pathfind for train %d",
          v_p_train_num(train_num));
  debugprint(debug_buffer, NAVIGATION_SERVER_DEBUG);
  states[train_num] = PATHFINDING;
  cb_clear(navigation_tasks);

  pathfind_task->destination_num = train_states[train_num].dest_num;
  pathfind_task->source_num = train_states[train_num].reged_at_num;
  pathfind_task->train = train_states[train_num].train_num;
  pathfind_task->is_valid = true;
  pathfind_task->delay_time = 100;
}
void navigation_server() {

  reservation_dirty = false;
  train_states_dirty = false;

  for (int i = 0; i < TRACK_MAX; i++) {
    reserved_nodes[i] = -1;
  }

  memset((void *)train_states, 0, sizeof(train_state_t) * MAX_NUM_TRAINS);
  memset((void *)cur_straightpath_tasks, 0,
         sizeof(straightpath_task_t) * MAX_NUM_TRAINS);

  for (v_train_num train_num = 0; train_num < MAX_NUM_TRAINS; train_num++) {
    train_states[train_num].dest_num = -1;
    train_states[train_num].reged_at_num = -1;
    train_states[train_num].reversed = false;
    train_states[train_num].train_num = train_num;
    train_states[train_num].dest_offset = -1;
    train_states[train_num].reged_at_offset = -1;
  }

  train_states_dirty = true;

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

  navigationserver_request req;
  navigationserver_response res;
  memset(&res, 0, sizeof(navigationserver_response));

  task_tid client;

  task_tid clients[MAX_NUM_TRAINS];
  memset(clients, -1, sizeof(task_tid) * MAX_NUM_TRAINS);

  reservation_client = -1;
  trainstate_print_client = -1;

  for (;;) {

    Receive(&client, (char *)&req, sizeof(navigationserver_request));

    if (req.type == NAVIGATION_REQUEST) {

      v_train_num train_num = req.data.navigation_request.train;

      char debug_buffer[MAX_DEBUG_STRING_LEN];
      sprintf(debug_buffer, "Got navigation request from train %d",
              v_p_train_num(train_num));
      debugprint(debug_buffer, VERBOSE_DEBUG);

      if (states[train_num] != IDLE) {
        memset(&res, 0, sizeof(navigationserver_response));
        res.type = NAVIGATIONSERVER_BUSY;
        Reply(client, (char *)&res, sizeof(nameserver_response));
        continue;
      }

      if (train_states[train_num].reged_at_num == -1) {
        memset(&res, 0, sizeof(navigationserver_response));
        res.type = NAVIGATIONSERVER_NEED_REGISTER;
        Reply(client, (char *)&res, sizeof(nameserver_response));
        continue;
      }

      states[train_num] = PATHFINDING;
      clients[train_num] = client;

      pathfind_task_t task;
      task.destination_num = req.data.navigation_request.destination_num;
      task.source_num = train_states[train_num].reged_at_num;
      task.train = req.data.navigation_request.train;
      task.is_valid = true;
      task.delay_time = 0;
      train_states[train_num].dest_num = task.destination_num;
      train_states_dirty = true;
      train_states[train_num].dest_offset = req.data.navigation_request.offset;

      if (pathfind_workers_parking[train_num] != -1) {
        sprintf(debug_buffer, "Giving work to pathfind worker for train %d",
                v_p_train_num(train_num));
        debugprint(debug_buffer, NAVIGATION_SERVER_DEBUG);
        give_pathfinder_work(pathfind_workers_parking[train_num], &task);
        pathfind_workers_parking[train_num] = -1;
      } else {
        pathfind_tasks[train_num] = task;
      }

    } else if (req.type == PATHFIND_WORKER) {

      v_train_num train_num = req.data.pathfindworker.train_num;

      char debug_buffer[MAX_DEBUG_STRING_LEN];
      sprintf(debug_buffer, "Got pathfind worker for train %d",
              v_p_train_num(train_num));
      debugprint(debug_buffer, VERBOSE_DEBUG);

      pathfind_workers_parking[train_num] = client;

      if ((states[train_num] != PATHFINDING) ||
          !pathfind_tasks[train_num].is_valid) {
        continue;
      }

      sprintf(debug_buffer, "Giving work to pathfind worker for train %d",
              v_p_train_num(train_num));
      debugprint(debug_buffer, VERBOSE_DEBUG);
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
      sprintf(debug_buffer, "Pathfind worker done for train %d",
              v_p_train_num(train_num));
      debugprint(debug_buffer, NAVIGATION_SERVER_DEBUG);

      states[train_num] = STRAIGHTPATHING;

      pathfind_result_t found_path =
          req.data.pathfindworker_done.pathfind_result;
      int *path = req.data.pathfindworker_done.path;
      int path_len = req.data.pathfindworker_done.path_len;

      if (clients[train_num] != -1) {
        memset(&res, 0, sizeof(navigationserver_response));
        if (found_path == NO_PATH_AT_ALL) {
          states[train_num] = IDLE;
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
        restart_pathfind(&(navigation_tasks[train_num]),
                         &(pathfind_tasks[train_num]), train_num);
        continue;
      }

      segments_fill_navigation_tasks(track, path, path_len,
                                     &(navigation_tasks[train_num]), train_num);

      if (straightpath_workers_parking[train_num] != -1) {
        process_next_task(&(navigation_tasks[train_num]), trainserver_tid,
                          timer_tid, train_num, straightpath_workers_parking);
      }

    } else if (req.type == STRAIGHTPATH_WORKER) {
      v_train_num train_num = req.data.straightpathworker.train_num;

      char debug_buffer[MAX_DEBUG_STRING_LEN];
      sprintf(debug_buffer, "Got straightpath worker for train %d",
              v_p_train_num(train_num));
      debugprint(debug_buffer, VERBOSE_DEBUG);

      straightpath_workers_parking[train_num] = client;

      if ((states[train_num] != STRAIGHTPATHING) ||
          cb_is_empty(&(navigation_tasks[train_num]))) {
        continue;
      }

      bool should_restart_pathfind =
          process_next_task(&(navigation_tasks[train_num]), trainserver_tid,
                            timer_tid, train_num, straightpath_workers_parking);
      if (should_restart_pathfind) {
        restart_pathfind(&(navigation_tasks[train_num]),
                         &(pathfind_tasks[train_num]), train_num);
        continue;
      }
    }

    else if (req.type == STRAIGHTPATH_WORKER_DONE) {
      v_train_num train_num = req.data.straightpathworker_done.train_num;

      char debug_buffer[MAX_DEBUG_STRING_LEN];
      sprintf(debug_buffer, "Got straightpath worker done for train %d",
              v_p_train_num(train_num));
      debugprint(debug_buffer, VERBOSE_DEBUG);

      if (cb_is_empty(&(navigation_tasks[train_num]))) {
        done_navigation(train_num);
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
      debugprint("Got reservation printer", VERBOSE_DEBUG);
      reservation_client = client;
    } else if (req.type == GET_TRAIN_STATE) {
      debugprint("Got trainstate printer", VERBOSE_DEBUG);
      trainstate_print_client = client;
    } else if (req.type == REGISTER_LOCATION) {
      debugprint("Got register location", VERBOSE_DEBUG);
      v_train_num train_num = req.data.register_location.train_num;
      int node_num = req.data.register_location.node_num;

      if ((track[node_num].type == NODE_ENTER)) {
        train_states[train_num].reged_at_offset = TRAIN_LEN;
      } else {
        train_states[train_num].reged_at_offset = 0;
      }

      train_states[train_num].reged_at_num = node_num;

      clear_reserved_by_train(train_num);
      reserve_node(node_num, train_num);

      memset(&res, 0, sizeof(navigationserver_response));
      res.type = NAVIGATIONSERVER_GOOD;
      Reply(client, (char *)&res, sizeof(navigationserver_response));
      train_states_dirty = true;
    } else if (req.type == NAVIGATIONSERVER_ATTRIBUTION_COURIER) {
      debugprint("Got attribution courier", VERBOSE_DEBUG);
      v_train_num *sensor_attributions =
          req.data.attribution_courier.sensor_pool;

      for (int i = 0; i < NUM_SENSOR_GROUPS * SENSORS_PER_GROUP; i++) {
        if ((sensor_attributions[i] != UNATTRIBUTED) &&
            (sensor_attributions[i] != NOT_TRIGGERED)) {
          v_train_num attributed_train_num = sensor_attributions[i];
          unreserve_path_src_up_to_node(i, attributed_train_num);
        }
      }
      navigationserver_response res;
      memset(&res, 0, sizeof(navigationserver_response));
      res.type = NAVIGATIONSERVER_GOOD;
      Reply(client, (char *)&res, sizeof(navigationserver_response));
    }

    if ((reservation_client != -1) && (reservation_dirty)) {
      navigationserver_response res;
      memset(&res, 0, sizeof(navigationserver_response));
      res.type = NAVIGATIONSERVER_GOOD;
      memcpy(res.data.get_reservations.reservations, reserved_nodes,
             sizeof(v_train_num) * TRACK_MAX);
      Reply(reservation_client, (char *)&res,
            sizeof(navigationserver_response));
      reservation_client = -1;
      reservation_dirty = false;
    }

    if ((trainstate_print_client != -1) && (train_states_dirty)) {
      navigationserver_response res;
      memset(&res, 0, sizeof(navigationserver_response));
      res.type = NAVIGATIONSERVER_GOOD;

      memcpy(&res.data.get_train_state.train_states, train_states,
             sizeof(train_states) * MAX_NUM_TRAINS);

      Reply(trainstate_print_client, (char *)&res,
            sizeof(navigationserver_response));
      trainstate_print_client = -1;
      train_states_dirty = false;
    }
  }
}
