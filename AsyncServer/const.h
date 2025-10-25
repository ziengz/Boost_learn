#pragma once
#define MAX_LENGTH 2048
#define HEAD_TOTAL_LEN 4
#define HEAD_ID_LEN 2
#define HEAD_DATA_LEN 2
#define MAX_SENDQUE 1000  //消息队列

enum MSG_IDS {
	MSG_HELLO_WORLD = 1001,
};