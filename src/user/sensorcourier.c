#include "sensorcourier.h"

void sensor_courier() {

  task_tid trainctl = WhoIsBlock("trainctl");
  task_tid dispatchhub = WhoIsBlock("dispatchhub");

  train_event event;

  dispatchhub_request dis_req;
  memset(&dis_req, 0, sizeof(dispatchhub_request));
  dispatchhub_response dis_res;

  for (;;) {
    SensorEvent(trainctl, &event);

    dis_req.type = DISPATCHHUB_SENSOR_UPDATE;
    memcpy(dis_req.data.sensor_update.sensor_readings, event.sensors,
           NUM_SENSOR_GROUPS);
    dis_req.data.sensor_update.time = event.time;

    Send(dispatchhub, (char *)&dis_req, sizeof(dispatchhub_request),
         (char *)&dis_res, sizeof(dispatchhub_response));
    memset(&dis_req, 0, sizeof(dispatchhub_request));
  }
}
