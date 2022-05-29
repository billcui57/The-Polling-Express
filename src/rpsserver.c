#include "rpsserver.h"

struct game *free_game;

void rpsserver_request_init(rpsserver_request *rq, rpsserver_request_type type,
                            char body[MAX_BODY_LENGTH],
                            unsigned int body_length) {

  KASSERT(sizeof(char) * body_length < sizeof(char) * (MAX_BODY_LENGTH),
          "command must fit in body arr");

  memset(rq, sizeof(rpsserver_request), 0);

  rq->type = type;
  rq->body_length = body_length;
  memcpy(rq->body, body,
         min(sizeof(char) * (MAX_BODY_LENGTH), sizeof(char) * body_length));
}

void rpsserver_response_init(rpsserver_response *rs,
                             rpsserver_response_type type,
                             char body[MAX_BODY_LENGTH],
                             unsigned int body_length) {
  KASSERT(sizeof(char) * body_length < sizeof(char) * (MAX_BODY_LENGTH),
          "command must fit in body arr");

  memset(rs, sizeof(rpsserver_response), 0);

  rs->type = type;
  rs->body_length = body_length;
  memcpy(rs->body, body,
         min(sizeof(char) * (MAX_BODY_LENGTH), sizeof(char) * body_length));
}

void rpsserver() {

  while (RegisterAs("rpsserver") != 0)
    ;

  // KASSERT(WhoIs("rpsserver") == MyTid(), "Rpsserver should be registered");

  task_tid who;
  char msg[sizeof(rpsserver_request)];
  char response_body[MAX_BODY_LENGTH];

  struct game game_backing[MAX_NUM_GAMES];
  free_game = &(game_backing[0]);

  // initialize games
  for (unsigned int i = 0; i < MAX_NUM_GAMES; i++) {
    game_backing[i].player1 = PLAYER_NULL;
    game_backing[i].player2 = PLAYER_NULL;
    game_backing[i].player1_move = MOVE_NULL;

    if (i == MAX_NUM_GAMES - 1) {
      game_backing[i].next = NULL;
    } else {
      game_backing[i].next = &(game_backing[i + 1]);
    }
  }

  for (;;) {
    int status = Receive(&who, &msg, sizeof(rpsserver_request));

    rpsserver_request *request = (rpsserver_request *)msg; // Deserialize

    rpsserver_response_type rt;
    rpsserver_response response;
    size_t body_len;

    if (request->type == REQUEST_SIGNUP) {

      if (free_game->player1 == PLAYER_NULL) {
        free_game->player1 =
            who; // still got one more player before we send off this game
      } else {
        free_game->player2 = who;

        unsigned int game_id = (free_game - game_backing) / sizeof(game);

        // both players signed up, we can start game

        rt = RESPONSE_GOOD;

        body_len = 1;
        response_body[0] = game_id;

        rpsserver_response_init(&response, rt, response_body, body_len);
        char *response_buffer = (char *)&response; // serialize
        Reply(free_game->player1, response_buffer, sizeof(rpsserver_response));
        Reply(free_game->player2, response_buffer, sizeof(rpsserver_response));
      }
    }
  }
}