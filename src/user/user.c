#include <user.h>

void task_k4_test();

void clear();
void shell();
void sensor_reader();

// track a
// 156,16,3,

// sensor D1,

// track b
// 155,156,5

// sensor C13

// gt 78 10 A13 D13 0
void task_k4_init() {
  clear_screen(BW_COM2);
  while (true) {
    if (which_track == 'a' || which_track == '?') {
      init_tracka(track);
      // BR3 - c, 7 - c, 156 , 155, 16 -c

      mark_switch_broken(track, track_name_to_num(track, "BR7"), DIR_CURVED);
      mark_switch_broken(track, track_name_to_num(track, "BR156"), DIR_CURVED);
      mark_switch_broken(track, track_name_to_num(track, "BR155"),
                         DIR_STRAIGHT);
      mark_switch_broken(track, track_name_to_num(track, "BR16"), DIR_STRAIGHT);
      mark_switch_broken(track, track_name_to_num(track, "BR3"), DIR_CURVED);
      mark_switch_broken(track, track_name_to_num(track, "BR15"), DIR_STRAIGHT);
      break;
    } else if (which_track == 'b') {
      init_trackb(track);

      // 4 -s
      mark_switch_broken(track, track_name_to_num(track, "BR156"),
                         DIR_STRAIGHT);
      mark_switch_broken(track, track_name_to_num(track, "BR155"), DIR_CURVED);
      mark_switch_broken(track, track_name_to_num(track, "BR5"), DIR_CURVED);
      mark_switch_broken(track, track_name_to_num(track, "BR4"), DIR_STRAIGHT);
      break;
    } else {
      printf(BW_COM2, "Please enter a valid track\r\n");
    }
  }
  generate_buffer_nodes(track);

  Create(20, "Nameserver", nameserver);
  Create(10, "Clockserver", clockserver);
  Create(10, "UartCom2TxServer", uart_com2_tx_server);
  Create(10, "UartCom1Server", uart_com1_server);
  Create(10, "UartCom2RxServer", uart_com2_rx_server);
  Create(10, "Trainserver", task_trainserver);
  Create(10, "NavigationServer", navigation_server);
  Create(10, "DispatchServer", dispatchserver);
  Create(10, "ShellInit", shell_init);
}

#define SENSOR_CB_BACK_CAPACITY 10
