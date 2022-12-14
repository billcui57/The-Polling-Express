#include "rpsserver.h"

unsigned long min(unsigned int a, unsigned int b);

struct game *free_game;

void rpsserver_request_init(rpsserver_request *rq, rpsserver_request_type type,
                            int *body, unsigned int body_length) {

  KASSERT(sizeof(int) * body_length < sizeof(int) * (MAX_BODY_LENGTH),
          "command must fit in body arr");

  memset(rq, 0, sizeof(rpsserver_request));

  rq->type = type;
  rq->body_length = body_length;
  memcpy(rq->body, body,
         min(sizeof(int) * (MAX_BODY_LENGTH), sizeof(int) * body_length));
}

void rpsserver_response_init(rpsserver_response *rs,
                             rpsserver_response_type type, int *body,
                             unsigned int body_length) {
  KASSERT(sizeof(int) * body_length < sizeof(int) * (MAX_BODY_LENGTH),
          "command must fit in body arr");

  memset(rs, 0, sizeof(rpsserver_response));

  rs->type = type;
  rs->body_length = body_length;
  memcpy(rs->body, body,
         min(sizeof(int) * (MAX_BODY_LENGTH), sizeof(int) * body_length));
}

int who_won(rps_move player1_move, rps_move player2_move) {

  if ((player1_move == MOVE_PAPER) && (player2_move == MOVE_PAPER)) {
    return 0;
  } else if ((player1_move == MOVE_PAPER) && (player2_move == MOVE_ROCK)) {
    return 1;
  } else if ((player1_move == MOVE_PAPER) && (player2_move == MOVE_SCISSOR)) {
    return 2;
  } else if ((player1_move == MOVE_ROCK) && (player2_move == MOVE_PAPER)) {
    return 2;
  } else if ((player1_move == MOVE_ROCK) && (player2_move == MOVE_ROCK)) {
    return 0;
  } else if ((player1_move == MOVE_ROCK) && (player2_move == MOVE_SCISSOR)) {
    return 1;
  } else if ((player1_move == MOVE_SCISSOR) && (player2_move == MOVE_PAPER)) {
    return 1;
  } else if ((player1_move == MOVE_SCISSOR) && (player2_move == MOVE_ROCK)) {
    return 2;
  } else if ((player1_move == MOVE_SCISSOR) && (player2_move == MOVE_SCISSOR)) {
    return 0;
  }
}

int SignUp(int *game_id) {
  task_tid rpsserver_tid;
  do {
    char query_string[] = "rpsserver";
    rpsserver_tid = WhoIs(query_string);
  } while (rpsserver_tid == -1);

  rpsserver_request rq;

  int body_len = 0;
  int body[body_len];

  rpsserver_request_init(&rq, REQUEST_SIGNUP, body, body_len);

  char *request_buffer = (char *)&rq; // Serialize

  char response_buffer[sizeof(rpsserver_response)];

  Send(rpsserver_tid, request_buffer, sizeof(rpsserver_request),
       response_buffer, sizeof(rpsserver_response));

  rpsserver_response *response =
      (rpsserver_response *)response_buffer; // Deserialize

  if (response->type == RESPONSE_GOOD) {
    *game_id = response->body[0];
  }

  return response->type;
}

int Quit(int game_id) {
  task_tid rpsserver_tid;
  do {
    char query_string[] = "rpsserver";
    rpsserver_tid = WhoIs(query_string);
  } while (rpsserver_tid == -1);

  rpsserver_request rq;

  int body_len = 1;
  int body[body_len];

  body[0] = game_id;

  rpsserver_request_init(&rq, REQUEST_QUIT, body, body_len);

  char *request_buffer = (char *)&rq; // Serialize

  char response_buffer[sizeof(rpsserver_response)];

  Send(rpsserver_tid, request_buffer, sizeof(rpsserver_request),
       response_buffer, sizeof(rpsserver_response));

  rpsserver_response *response =
      (rpsserver_response *)response_buffer; // Deserialize

  return response->type;
}

int Play(int game_id, rps_move move) {
  task_tid rpsserver_tid;
  do {
    char query_string[] = "rpsserver";
    rpsserver_tid = WhoIs(query_string);
  } while (rpsserver_tid == -1);

  rpsserver_request rq;

  int body_len = 2;
  int body[body_len];

  body[0] = game_id;
  body[1] = move;

  rpsserver_request_init(&rq, REQUEST_PLAY, body, body_len);

  char *request_buffer = (char *)&rq; // Serialize

  char response_buffer[sizeof(rpsserver_response)];

  Send(rpsserver_tid, request_buffer, sizeof(rpsserver_request),
       response_buffer, sizeof(rpsserver_response));

  rpsserver_response *response =
      (rpsserver_response *)response_buffer; // Deserialize

  return response->type;
}

bool game_in_play(game *g) {
  return ((g->player1 != NULL) && (g->player2 != NULL));
}

void free_used_game(game *g) {
  g->player1 = PLAYER_NULL;
  g->player2 = PLAYER_NULL;
  g->player1_move = MOVE_NULL;
  g->player2_move = MOVE_NULL;
  g->next = free_game;
  free_game = g;
}

void rpsserver() {

  char my_name[] = "rpsserver";

  while (RegisterAs(my_name) != 0)
    ;

  task_tid who;
  char msg[sizeof(rpsserver_request)];

  struct game game_backing[MAX_NUM_GAMES];
  free_game = &(game_backing[0]);

  // initialize games
  for (unsigned int i = 0; i < MAX_NUM_GAMES; i++) {
    game_backing[i].player1 = PLAYER_NULL;
    game_backing[i].player2 = PLAYER_NULL;
    game_backing[i].player1_move = MOVE_NULL;
    game_backing[i].player2_move = MOVE_NULL;

    if (i == MAX_NUM_GAMES - 1) {
      game_backing[i].next = NULL;
    } else {
      game_backing[i].next = &(game_backing[i + 1]);
    }
  }

  int response_body[MAX_BODY_LENGTH];

  for (;;) {
    int status = Receive(&who, msg, sizeof(rpsserver_request));

    rpsserver_request *request = (rpsserver_request *)msg; // Deserialize

    rpsserver_response_type rt;
    rpsserver_response response;
    size_t body_len;

    if (request->type == REQUEST_SIGNUP) {

      if (free_game == NULL) {
        rt = RESPONSE_NO_FREE_GAMES;
        body_len = 0;
        rpsserver_response_init(&response, rt, response_body, body_len);
        char *response_buffer = (char *)&response; // serialize
        Reply(who, response_buffer, sizeof(rpsserver_response));
        continue;
      }

      if (free_game->player1 == PLAYER_NULL) {
        free_game->player1 =
            who; // still got one more player before we send off this game
      } else {
        free_game->player2 = who;

        int game_id = (free_game - &(game_backing[0]));

        // both players signed up, we can start game

        rt = RESPONSE_GOOD;

        body_len = 1;
        response_body[0] = game_id;

        game *game = free_game;
        free_game = free_game->next;

        rpsserver_response_init(&response, rt, response_body, body_len);
        char *response_buffer = (char *)&response; // serialize
        Reply(game->player1, response_buffer, sizeof(rpsserver_response));
        Reply(game->player2, response_buffer, sizeof(rpsserver_response));
      }
    } else if (request->type == REQUEST_QUIT) {

      int game_id = request->body[0];
      game *curr_game = &(game_backing[game_id]);

      if ((curr_game->player1 != who) && (curr_game->player2 != who)) {
        rt = RESPONSE_NOT_YOUR_GAME;
        body_len = 0;
        rpsserver_response_init(&response, rt, response_body, body_len);
        char *response_buffer = (char *)&response; // serialize
        Reply(who, response_buffer, sizeof(rpsserver_response));
        continue;
      }

      // if game has both players filled, then inform the other and recycle the
      // game

      task_tid other_player;

      if (who == curr_game->player1) {
        curr_game->player1 = PLAYER_NULL;
        other_player = curr_game->player2;
      } else {
        curr_game->player2 = PLAYER_NULL;
        other_player = curr_game->player1;
      }

      // has the other player gone first before I can quit? The other player
      // would be waiting for a response

      rps_move other_player_move = other_player == curr_game->player1
                                       ? curr_game->player1_move
                                       : curr_game->player2_move;

      if (other_player_move != MOVE_NULL) {
        // we can clean up this game

        free_used_game(curr_game);

        rt = RESPONSE_GAME_ENDED;
        body_len = 0;
        rpsserver_response_init(&response, rt, response_body, body_len);
        char *response_buffer = (char *)&response; // serialize
        Reply(other_player, response_buffer, sizeof(rpsserver_response));
      }

      rt = RESPONSE_GOOD;
      body_len = 0;
      rpsserver_response_init(&response, rt, response_body, body_len);
      char *response_buffer = (char *)&response; // serialize
      Reply(who, response_buffer, sizeof(rpsserver_response));
    } else if (request->type == REQUEST_PLAY) {

      int game_id = request->body[0];
      int move = request->body[1];

      game *curr_game = &(game_backing[game_id]);

      // has the other player quit?
      if ((curr_game->player1 == PLAYER_NULL) ||
          (curr_game->player2 == PLAYER_NULL)) {

        free_used_game(curr_game);

        rt = RESPONSE_GAME_ENDED;
        body_len = 0;
        rpsserver_response_init(&response, rt, response_body, body_len);
        char *response_buffer = (char *)&response; // serialize
        Reply(who, response_buffer, sizeof(rpsserver_response));
        continue;
      }

      if ((curr_game->player1 == who) &&
          (curr_game->player1_move == MOVE_NULL)) {
        curr_game->player1_move = move;
      } else if ((curr_game->player2 == who) &&
                 (curr_game->player2_move == MOVE_NULL)) {
        curr_game->player2_move = move;
      } else {
        rt = RESPONSE_NOT_YOUR_GAME;
        body_len = 0;
        rpsserver_response_init(&response, rt, response_body, body_len);
        char *response_buffer = (char *)&response; // serialize
        Reply(who, response_buffer, sizeof(rpsserver_response));
        continue;
      }

      if ((curr_game->player1_move != MOVE_NULL) &&
          (curr_game->player2_move != MOVE_NULL)) {
        int winner = who_won(curr_game->player1_move, curr_game->player2_move);

        curr_game->player1_move = MOVE_NULL;
        curr_game->player2_move = MOVE_NULL;
        if (winner == 0) {
          rt = RESPONSE_TIE;
          body_len = 0;
          rpsserver_response_init(&response, rt, response_body, body_len);
          char *response_buffer = (char *)&response; // serialize
          Reply(curr_game->player1, response_buffer,
                sizeof(rpsserver_response));
          Reply(curr_game->player2, response_buffer,
                sizeof(rpsserver_response));
        } else {
          rt = RESPONSE_YOU_WON;
          body_len = 0;
          rpsserver_response_init(&response, rt, response_body, body_len);
          char *response_buffer1 = (char *)&response; // serialize
          Reply(winner == 1 ? curr_game->player1 : curr_game->player2,
                response_buffer1, sizeof(rpsserver_response));

          rt = RESPONSE_YOU_LOST;
          body_len = 0;
          rpsserver_response_init(&response, rt, response_body, body_len);
          char *response_buffer2 = (char *)&response; // serialize
          Reply(winner == 1 ? curr_game->player2 : curr_game->player1,
                response_buffer2, sizeof(rpsserver_response));
        }
      }
    }
  }
}
