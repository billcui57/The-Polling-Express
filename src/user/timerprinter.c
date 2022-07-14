#include "timerprinter.h"

#include <clockserver.h>
#include <syscall.h>

char status[] = "-----";

void timer_printer() {
  task_tid clock_tid = WhoIsBlock("clockserver");
  int start_time = Time(clock_tid);

  int i = 0;

  for (;;) {
    int duc = DelayUntil(clock_tid, start_time + (i + 1) * 10);

    int formatted_time[3];
    memset(formatted_time, 0, sizeof(int) * 3);
    get_formatted_curr_time(Time(clock_tid), formatted_time, 100);

    cursor_to_pos(TIME_ROW, TIME_COL, TIME_WIDTH);
    printf(COM2, "Time: %d min %d.%d secs\r\n", formatted_time[0],
           formatted_time[1], formatted_time[2]);
    cursor_to_pos(IDLE_ROW, IDLE_COL, IDLE_WIDTH);
    printf(COM2, "Idle: %d%% [%s]\r\n", idle_percentage, status);
    done_print();
    i++;
  }
}
