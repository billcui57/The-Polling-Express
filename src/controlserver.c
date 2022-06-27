#include "controlserver.h"
#include <syscall.h>

void control_server() {

  RegisterAs("controlserver");

  task_tid trainserver = WhoIsBlock("trainctl");
  task_tid clock = WhoIsBlock("clockserver");

  controlserver_request req;
  controlserver_response res;
  task_tid client;

  memset(&res, 0, sizeof(res));

  for (;;) {
    Receive(&client, (char *)&req, sizeof(controlserver_request));

    if (req.type == PATHFIND) {

      int src_num = track_name_to_num(&track, req.src_name);
      int dest_num = track_name_to_num(&track, req.dest_name);

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
      while ((node != NULL) && (node != dest)) {
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

      res.type = CONTROLSERVER_GOOD;

      Reply(client, (char *)&res, sizeof(controlserver_response));
    }
  }
}
