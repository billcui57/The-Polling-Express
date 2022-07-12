#include "sensorcourier.h"

void sensor_courier() {

  task_tid trainctl = WhoIsBlock("trainctl");
  task_tid dispatchserver = WhoIsBlock("dispatchserver");

  train_event event;

  dispatchserver_request dis_req;
  memset(&dis_req, 0, sizeof(dispatchserver_request));
  dispatchserver_response dis_res;

  for (;;) {
    SensorEvent(trainctl, &event);

    dis_req.type = DISPATCHSERVER_SENSOR_UPDATE;
    memcpy(dis_req.data.sensor_update.sensor_readings, event.sensors,
           sizeof(char) * NUM_SENSOR_GROUPS);
    dis_req.data.sensor_update.time = event.time;

    Send(dispatchserver, (char *)&dis_req, sizeof(dispatchserver_request),
         (char *)&dis_res, sizeof(dispatchserver_response));
    memset(&dis_req, 0, sizeof(dispatchserver_request));
  }
}
