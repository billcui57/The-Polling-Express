#include <nameserver.h>

unsigned long min(unsigned int a, unsigned int b) { return a < b ? a : b; }

void nameserver_request_init(nameserver_request *rq,
                             nameserver_request_type type, const char *body,
                             unsigned int body_length) {
  KASSERT(sizeof(char) * body_length < sizeof(char) * (MAX_BODY_LENGTH),
          "name must fit in body arr");

  memset(rq, 0, sizeof(nameserver_request));

  rq->type = type;
  rq->body_length = body_length;

  memcpy(rq->body, body,
         min(sizeof(char) * (MAX_BODY_LENGTH), sizeof(char) * body_length));
}

void nameserver_response_init(nameserver_response *rs,
                              nameserver_response_type type, const char *body,
                              unsigned int body_length) {
  KASSERT(sizeof(char) * body_length < sizeof(char) * (MAX_BODY_LENGTH),
          "name must fit in body arr");

  memset(rs, 0, sizeof(nameserver_request));

  rs->type = type;
  rs->body_length = body_length;
  memcpy(rs->body, body,
         min(sizeof(char) * (MAX_BODY_LENGTH), sizeof(char) * body_length));
}

int nameserver_tid = -1;

void nameserver() {

  nameserver_tid = MyTid();

  void *backing[MAX_NAMES];
  struct hashtable ht;
  ht_init(&ht, MAX_NAMES, backing);

  task_tid who;
  char msg[sizeof(nameserver_request)];

  char response_body[MAX_BODY_LENGTH];

  for (;;) {
    int status = Receive(&who, msg, sizeof(nameserver_request));

    nameserver_request *request = (nameserver_request *)msg; // Deserialize

    int ht_status = 0;

    nameserver_response_type rt;
    nameserver_response response;
    size_t body_len;

    if (request->type == REQUEST_REGISTER_AS) {

      ht_status = ht_insert(&ht, request->body, (void *)who);

      switch (ht_status) {
      case 0:
      case E_COLLISION: // collisions are fine
        rt = RESPONSE_GOOD;
        break;
      default:
        break;
      }

      body_len = 0;

    } else if (request->type == REQUEST_WHO_IS) {

      void *lookup_tid_void;

      ht_status = ht_get(&ht, request->body, &lookup_tid_void);

      task_tid lookup_tid = (task_tid)lookup_tid_void;

      switch (ht_status) {
      case 0:
        rt = RESPONSE_GOOD;
        break;
      case E_KEY_MISSING:
        rt = RESPONSE_NAME_DNE;
        break;
      default:
        break;
      }
      body_len = 1;
      response_body[0] = lookup_tid;
    }
    nameserver_response_init(&response, rt, response_body, body_len);
    char *response_buffer = (char *)&response; // serialize
    Reply(who, response_buffer, sizeof(nameserver_response));
  }
}
