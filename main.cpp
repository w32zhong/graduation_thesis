#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#if defined( __linux__ )

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define SOCKET int
#define SOCKADDR_IN sockaddr_in 
#define SOCKADDR sockaddr
#define SOCKLEN_T socklen_t
#define SOCKCLOSE close

#define WIN_OPEN
#define WIN_CLOSE

#else

#include <winsock2.h>
#pragma comment(lib,"ws2_32.lib")

#define SOCKLEN_T int
#define SOCKCLOSE closesocket

#define WIN_OPEN \
	WSADATA wsa; \
	WSAStartup(MAKEWORD(1,1),&wsa)

#define WIN_CLOSE \
	WSACleanup()

#endif

int phrase_str_cmp(char *str0, int i, const char *str1) 
{
	int j = 0;
	for (; j < strlen(str1); j++) {
		if (str0[i] == '\0' ||
			str0[i] != str1[j])
			return -1;

		i++;
	}

	return j;
}

int phrase_find_pair(char *txt, int *begin,
	const char *pair_begin, const char *pair_end) 
{
	int pair_begin_pos, res, i = *begin;
	for (; txt[i] != '\0'; i++) {
		res = phrase_str_cmp(txt, i, pair_begin);
		if(res != -1) {
			i += res;
			pair_begin_pos = i;
			goto next_pair;
		}
	}

	return -1;

next_pair:

	for (; txt[i] != '\0'; i++) {
		res = phrase_str_cmp(txt, i, pair_end);
		if(res != -1) {
			i += res;
			*begin = i;
			return pair_begin_pos;
		}
	}

	return -1;
}

#define MAX_REQ_LEN 2048

int phrase_html_head(char *html_head, char *time, char *user,
		char *action, char *content)
{
	char json[MAX_REQ_LEN];
	int i = 0;
	int res = phrase_find_pair(html_head, &i, "\r\n{", "}");
	if (res == -1)
		return 0;
	memcpy(json, html_head + res, i - res);	
	json[i - res] = '\0';
	i = 0;
	
	res = phrase_find_pair(json, &i, "\"time\":", ",");
	if (res == -1)
		return 0;
	memcpy(time, json + res, i - res - 1);
	time[i - res - 1] = '\0';

	res = phrase_find_pair(json, &i, "\"user\":\"", "\",");
	if (res == -1)
		return 0;
	memcpy(user, json + res, i - res - 2);
	user[i - res - 2] = '\0';

	res = phrase_find_pair(json, &i, "\"action\":\"", "\",");
	if (res == -1)
		return 0;
	memcpy(action, json + res, i - res - 2);
	action[i - res - 2] = '\0';

	res = phrase_find_pair(json, &i, "\"content\":\"", "\"}");
	if (res == -1)
		return 0;
	memcpy(content, json + res, i - res - 2);
	content[i - res - 2] = '\0';

	return 1;
}

#if 0
int main()
{
	char html_head[]
		= "bla...bla...\r\n{\"time\":1323333,\"user\":\"xxx\",\"action\":\"answer\",\"content\":\"abcde\"}bla...bla...";
	char time[MAX_REQ_LEN/4];
	char user[MAX_REQ_LEN/4];
	char action[MAX_REQ_LEN/4];
	char content[MAX_REQ_LEN/4];
	int res;

	res = phrase_html_head(html_head, time, user, action, content);
	if (res)
		printf("%s, %s, %s, %s. \n", time, user, action, content);
	else
		printf("phrasing failed. \n");

	return 0;
	
}

#endif

int main()
{
	SOCKET server_sock;
	SOCKET client_sock;
	SOCKADDR_IN server_addr;
	SOCKADDR_IN client_addr;
	WIN_OPEN;

	if((server_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) <= 0)
		return -1;
	
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(8080);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if(bind(server_sock, (SOCKADDR *)&server_addr, sizeof(server_addr)) != 0)
		return -1;
	
	if(listen(server_sock, 1) != 0)
		return -1;

	SOCKLEN_T len = sizeof(SOCKADDR_IN);

	while(1)
	{
		if((client_sock = accept(server_sock, (SOCKADDR *)&client_addr, &len))<=0)
			return -1;

		char recv_buf[1024*512];
		int res = recv(client_sock, recv_buf, sizeof(recv_buf), 0);
		recv_buf[res] = '\0';

		printf("\n======\n%s\n=====\n", recv_buf);

		if(res <= 0) {
			printf("connection closed. \n");
			SOCKCLOSE(client_sock);
			break;
		}

		char send_buf[] = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\nPASS";
		send(client_sock, send_buf, strlen(send_buf), 0);
		SOCKCLOSE(client_sock);
	}
	
	WIN_CLOSE;
	return 0;
}

#if 0
void main()
{
	char a[] = "a dog /*has*/ a son.";
	const char pair_begin[] = "/*";
	const char pair_end[] = "*/";

#if 0
	output: "a dog /*(as*/)a son."
#endif

	int res, i = 0, j = 0;
	res = my_find_pair(a, &i, pair_begin, pair_end);

	if (res != -1) {
		a[res] = '(';
		a[i - 1] = ')';
		printf("%s \n", a);
	} else
		printf("not find. \n");
}
#endif

struct req {
	struct req *prev;
	unsigned int time;
	char user[32];
	char action[32];
	char answer[32];
};

struct req* new_req(char *time, char *user, 
		char *action, char *answer)
{
	struct req *ret = (struct req*)malloc(sizeof(struct req));
	ret->prev = NULL;
	sscanf(time, "%u", &ret->time);
	strcpy(ret->user, user);
	strcpy(ret->action, action);
	strcpy(ret->answer, answer);

	return ret;
}

void print_reqs(struct req *list_tail)
{
	if (list_tail == NULL)
		return;

	do {
		printf("%d, %s, %s, %s \n", list_tail->time, list_tail->user, 
				list_tail->action, list_tail->answer);
		list_tail = list_tail->prev;
	} while (list_tail != NULL);
}

struct req* user_prev_req(struct req *r)
{
	struct req tpl;
	if (r == NULL)
		return NULL;

	strcpy(tpl.user, r->user);

	while (r->prev) {
		r = r->prev;
		if (strcmp(r->user, tpl.user) == 0)
			return r;
	}

	return NULL;
}

/* r0 is older than r1 */
#define COUNT_PERMIT_IF \
	if  \
		strcpy(alert_reason, "request too frequently"); \
		break_flag = 1; \
	else if (r1->time - r0-time > 60000) \
#define COUNT_PERMIT_MAX 4

#define ALERT0_NAME       "continuously similar comments"
#define ALERT0_COUNT_IF  (strcmp(r0->action, r1->action) == 0 && strcmp(r0->answer, r1->answer) == 0)
#define ALERT0_COUNT_MAX  4 

typedef int permit_fun(struct req*, struct req*, struct req*);

int check_legal(struct req *list_tail, 
		permit_fun *alert_count_if, 
		char *alert_name,
		int alert_count_max,
		)
{
	struct req *r0, *r1 = list_tail;
	int count = 0;
	int break_flag = 0;
	char alert[64];

	while (r1) {
		count++;
		r0 = user_prev_req(r1);
		if (r0 == NULL || !(*permit)(r0, r1))
			break;

		r1 = r0;
	}

	if (count >= permit_max)
		return 0;
	else
		return 1;
}

#if 0
int main()
{
	int i = 0;
	char str_tmp_time[64];
	char str_tmp_user[64];
	char str_tmp_action[64];
	char str_tmp_answer[64];
	struct req *tmp, *list_tail = NULL;

	while (i < 15) {
		sprintf(str_tmp_time, "%d", i*2);
		sprintf(str_tmp_user, "user%d", i%4);
		sprintf(str_tmp_action, "action%d", 457);
		sprintf(str_tmp_answer, "answer%d", i);
		
		tmp = new_req(str_tmp_time, str_tmp_user,
				str_tmp_action, str_tmp_answer);
		tmp->prev = list_tail;
		list_tail = tmp;
		 
		i++;
	}

	if (check_legal(list_tail))
		printf("PASS \n");
	else
		printf("STOP \n");

	printf("\033[2J");
	printf("\33[0;0H");
	print_reqs(list_tail);
	printf("\33[0;0H");
}
#endif
