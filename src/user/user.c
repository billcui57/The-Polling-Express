#include <user.h>

void task_k4_test();

void clear();
void shell();
void sensor_reader();

char status[] = "-----";

// track a
// 156,16,3,

// sensor D1,

// track b
// 155,156,5

// sensor C13

void task_k4_init() {
  clear_screen(BW_COM2);
  while (true) {

    printf(BW_COM2, "Which track am I on (a,b)?\r\n");
    char c = bw_uart_get_char(COM2);
    which_track = c;

    if (c == 'a') {
      init_tracka(track);
      mark_switch_broken(track, track_name_to_num(track, "BR156"), DIR_CURVED);
      mark_switch_broken(track, track_name_to_num(track, "BR155"),
                         DIR_STRAIGHT);
      mark_switch_broken(track, track_name_to_num(track, "BR16"), DIR_CURVED);
      mark_switch_broken(track, track_name_to_num(track, "BR3"), DIR_CURVED);
      mark_sensor_broken(track, track_name_to_num(track, "D1"));
      break;
    } else if (c == 'b') {
      init_trackb(track);
      mark_switch_broken(track, track_name_to_num(track, "BR156"),
                         DIR_STRAIGHT);
      mark_switch_broken(track, track_name_to_num(track, "BR155"), DIR_CURVED);
      mark_switch_broken(track, track_name_to_num(track, "BR5"), DIR_CURVED);
      mark_sensor_broken(track, track_name_to_num(track, "C13"));
      break;
    } else {
      printf(BW_COM2, "Please enter a valid track\r\n");
    }
  }

  Create(20, nameserver);
  Create(10, clockserver);
  Create(10, uart_com2_tx_server);
  Create(10, uart_com1_server);
  Create(10, uart_com2_rx_server);
  Create(10, task_trainserver);
  Create(10, navigation_server);
  Create(10, shell_init);
  Create(10, dispatchserver);
}

#define SENSOR_CB_BACK_CAPACITY 10
