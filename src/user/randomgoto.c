#include "randomgoto.h"

void random_goto1() {

  task_tid clock_tid = WhoIsBlock("clockserver");
  task_tid navigation_server = WhoIsBlock("navigationserver");

  int dests[4] = {
      track_name_to_num(track, "D13"),
      track_name_to_num(track, "E7"),
      track_name_to_num(track, "B14"),
      track_name_to_num(track, "D11"),
  };

  task_tid client;
  v_train_num command_train_num;
  Receive(&client, (char*)&command_train_num, sizeof(v_train_num));
  Reply(client, (char*)&command_train_num, 0);

  int dest_index = 0;
  for (;;) {

    int command_dest_num = dests[dest_index % 4];
    int command_offset = 0;

    navigationserver_request req;
    navigationserver_response res;
    memset(&req, 0, sizeof(navigationserver_request));
    req.type = NAVIGATION_REQUEST;
    req.data.navigation_request.train = command_train_num;
    req.data.navigation_request.destination_num = command_dest_num;
    req.data.navigation_request.offset = command_offset;
    req.data.navigation_request.should_hang = true;

    Send(navigation_server, (char *)&req, sizeof(req), (char *)&res,
         sizeof(res));

    if (res.type == NAVIGATIONSERVER_BUSY) {
    } else if (res.type == NAVIGATIONSERVER_NO_PATH) {
      dest_index++;
    } else if (res.type == NAVIGATIONSERVER_NEED_REGISTER) {
    } else {
      dest_index++;
    }
    Delay(clock_tid, 500);
  }
}

void random_goto2() {

  task_tid clock_tid = WhoIsBlock("clockserver");
  task_tid navigation_server = WhoIsBlock("navigationserver");

  int dests[4] = {
      track_name_to_num(track, "E15"),
      track_name_to_num(track, "A5"),
      track_name_to_num(track, "D10"),
      track_name_to_num(track, "E4"),
  };

  task_tid client;
  v_train_num command_train_num;
  Receive(&client, (char*)&command_train_num, sizeof(v_train_num));
  Reply(client, (char*)&command_train_num, 0);

  int dest_index = 0;
  for (;;) {

    int command_dest_num = dests[dest_index % 4];
    int command_offset = 0;

    navigationserver_request req;
    navigationserver_response res;
    memset(&req, 0, sizeof(navigationserver_request));
    req.type = NAVIGATION_REQUEST;
    req.data.navigation_request.train = command_train_num;
    req.data.navigation_request.destination_num = command_dest_num;
    req.data.navigation_request.offset = command_offset;
    req.data.navigation_request.should_hang = true;

    Send(navigation_server, (char *)&req, sizeof(req), (char *)&res,
         sizeof(res));

    if (res.type == NAVIGATIONSERVER_BUSY) {
    } else if (res.type == NAVIGATIONSERVER_NO_PATH) {
      dest_index++;
    } else if (res.type == NAVIGATIONSERVER_NEED_REGISTER) {
    } else {
      dest_index++;
    }
    Delay(clock_tid, 500);
  }
}

void random_goto3() {

  task_tid clock_tid = WhoIsBlock("clockserver");
  task_tid navigation_server = WhoIsBlock("navigationserver");

  task_tid client;
  v_train_num command_train_num;
  Receive(&client, (char*)&command_train_num, sizeof(v_train_num));
  Reply(client, (char*)&command_train_num, 0);

  int dest_index = command_train_num;
  for (;;) {
    int sel = (43 * dest_index) % 80;

    int command_dest_num = sel;
    int command_offset = 0;

    navigationserver_request req;
    navigationserver_response res;
    memset(&req, 0, sizeof(navigationserver_request));
    req.type = NAVIGATION_REQUEST;
    req.data.navigation_request.train = command_train_num;
    req.data.navigation_request.destination_num = command_dest_num;
    req.data.navigation_request.offset = command_offset;
    req.data.navigation_request.should_hang = true;

    Send(navigation_server, (char *)&req, sizeof(req), (char *)&res,
         sizeof(res));

    if (res.type == NAVIGATIONSERVER_BUSY) {
    } else if (res.type == NAVIGATIONSERVER_NO_PATH) {
      dest_index++;
    } else if (res.type == NAVIGATIONSERVER_NEED_REGISTER) {
    } else {
      dest_index++;
    }
    Delay(clock_tid, 500);
  }
}
