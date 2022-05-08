#pragma once

#define MAX_BUFF_SIZE 4096
#define MAX_PACKET_SIZE 256

#define MY_SERVER_PORT 9240

#define MAX_STR_SIZE 50

#define ID_STR_LENGTH 10

#define IOT_RECV 0
#define IOT_SEND 1
#define IOT_MONSTER_MOVE 2
#define IOT_MONSTER_ATTACK 3
#define IOT_MONSTER_RESPAWN 4
#define IOT_USER_HEAL 5

#define CS_LOGIN 1
#define CS_LOGOUT 2
#define CS_MOVE 3
#define CS_ATTACK 4
#define CS_CHAT 5

#define SC_LOGIN_OK 1
#define SC_LOGIN_FAIL 2
#define SC_POSITION_INFO 3
#define SC_CHAT 4
#define SC_STAT_CHANGE 5
#define SC_REMOVE_OBJECT 6
#define SC_ADD_OBJECT 7

#pragma pack (push, 1)

struct cs_login
{
	unsigned char size;
	unsigned char type;
	wchar_t id_str[ID_STR_LENGTH];
};

struct cs_logout
{
	unsigned char size;
	unsigned char type;
};

struct cs_move
{
	unsigned char size;
	unsigned char type;
	unsigned char dir;
};

struct cs_attack
{
	unsigned char size;
	unsigned char type;
};

struct cs_chat
{
	unsigned char size;
	unsigned char type;
	wchar_t chat_str[MAX_STR_SIZE];
};

struct sc_login_ok
{
	unsigned char size;
	unsigned char type;
	int myKey;
	int level;
	int x;
	int y;
	int exp;
	int hp;
};

struct sc_login_fail
{
	unsigned char size;
	unsigned char type;
};

struct sc_position_info
{
	unsigned char size;
	unsigned char type;
	int targetKey;
	int objectType;
	int x;
	int y;
};

struct sc_chat
{
	unsigned char size;
	unsigned char type;
	int speakerKey;
	wchar_t chat_str[MAX_STR_SIZE];
};

struct sc_stat_change
{
	unsigned char size;
	unsigned char type;
	int targetKey;
	int objectType;
	int level;
	int exp; 
	int hp;
};

struct sc_remove_object
{
	unsigned char size;
	unsigned char type;
	int targetKey;
	int objectType;
};

struct sc_add_object
{
	unsigned char size;
	unsigned char type;
	int targetKey;
	int objectType;
	int hp;
};

#pragma pack (pop)