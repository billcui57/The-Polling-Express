#pragma once

#include <my_assert.h>
#include <syscall.h>
#include <task.h>

#define MAX_NUM_GAMES 1000

#define REQUEST_SIGNUP 1
#define REQUEST_PLAY 2
#define REQUEST_QUIT 3

#define PLAYER_NULL -1

#define MOVE_NULL 1
#define MOVE_ROCK 2
#define MOVE_PAPER 3
#define MOVE_SCISSOR 4

typedef char rpsserver_request_type;

#define RESPONSE_GOOD 0

typedef char rpsserver_response_type;

typedef char rps_move;

typedef struct game {
  task_tid player1;
  task_tid player2;

  rps_move player1_move;
  rps_move player2_move;
  struct game *next;
} game;

typedef struct rpsserver_request {
  rpsserver_request_type type;
  char *body;
} rpsserver_request;

typedef struct rpsserver_response {
  rpsserver_response_type type;
  char *body;
} rpsserver_response;

void rpsserver_request_init(rpsserver_request *rq, rpsserver_request_type type,
                            char *body);

void rpsserver_response_init(rpsserver_response *rs,
                             rpsserver_response_type type, char *body);

void rpsserver();