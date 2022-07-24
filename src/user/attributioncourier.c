#include "attributioncourier.h"

void attribution_courier() {

  task_tid navigationserver_tid = WhoIsBlock("navigationserver");
  task_tid dispatchserver_tid = WhoIsBlock("dispatchserver");

  dispatchserver_request dis_req;
  dispatchserver_response dis_res;

  navigationserver_request nav_req;
  navigationserver_response nav_res;

  memset((void *)&dis_req, 0, sizeof(dispatchserver_request));
  memset((void *)&nav_req, 0, sizeof(navigationserver_request));

  dis_req.type = DISPATCHSERVER_ATTRIBUTION_COURIER;
  nav_req.type = NAVIGATIONSERVER_ATTRIBUTION_COURIER;

  for (;;) {
    Send(dispatchserver_tid, (void *)&dis_req, sizeof(dispatchserver_request),
         (void *)&dis_res, sizeof(dispatchserver_response));

    memcpy((void *)nav_req.data.attribution_courier.sensor_pool,
           dis_res.data.attribution_courier.sensor_pool,
           sizeof(v_train_num) * NUM_SENSOR_GROUPS * SENSORS_PER_GROUP);
    nav_req.data.attribution_courier.time =
        dis_res.data.attribution_courier.time;

    Send(navigationserver_tid, (void *)&nav_req,
         sizeof(navigationserver_request), (void *)&nav_res,
         sizeof(navigationserver_response));
  }
}