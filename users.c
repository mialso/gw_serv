#include "gwan.h"
#include <malloc.h>
#include <string.h>

struct user {
	char *name;
	char *passw;
	char *role;
	char *id;
};
kv_t users;

int validate_password_fail(int len, char *string, char *passw);

int main(int args, char *argv[])
{
		char str[1024] = {};
		char *req_data = (char *) get_env(argv, REQ_ENTITY);
		int content_len = (int) get_env(argv, CONTENT_LENGTH);
		xbuf_t *reply = get_reply(argv);
		struct user *new_user;
		if (0 == content_len) {
			xbuf_cat(reply,
						"0001:vasil:123:admin"
						"|0002:petro:456:manager"
						"|0003:stranger:17:manager"
			);
			s_snprintf(str, sizeof(str)-1, "req_data =%s", "content len was 0");
			log_err(argv, str);
			return HTTP_200_OK;
		}
		s_snprintf(str, sizeof(str)-1, "req_data =%s, content_len =%d", req_data, content_len);
		log_err(argv, str);
		if ('\0' == users.name[0]) {
				kv_init(&users, "users", 10, 0, 0, 0);
				log_err(argv, "users kv store initialized");
				kv_item item;
				new_user = (struct user*) malloc(sizeof(struct user));
				new_user->name = (char*) strdup("vasil");
				new_user->passw = (char*) strdup("123");
				new_user->role = (char*) strdup("admin");
				new_user->id = (char*) strdup("0001");
				item.key = new_user->name;
				item.klen = sizeof("vasil") -1;
				item.val = (char *) new_user;
				item.in_use = 0;
				kv_add(&users, &item);
		}
		new_user = (struct user *) kv_get(&users, "vasil", sizeof("vasil")-1);
		if (!new_user) {
				xbuf_cat(reply, "epic_fail");
				log_err(argv, "no user found in users store");
				return HTTP_200_OK;
		}
		if ((validate_password_fail(content_len, req_data, new_user->passw))) {
				xbuf_cat(reply, "epic_fail");
				log_err(argv, "password fail");
				return HTTP_200_OK;
		}
		s_snprintf(str, sizeof(str)-1, "%s:%s:%s:%s", new_user->id, new_user->name, new_user->passw, new_user->role);
		xbuf_cat(reply, str);

		return HTTP_200_OK;
}
int validate_password_fail(int len, char *string, char *passw)
{
	u64 counter = 0;
	u64 passw_counter = 0;
	while(counter < len) {
		if (',' != string[counter] && 0 == passw_counter) {
			++counter;
			continue;
		}
		if (',' == string[counter]) {
			++counter;
			passw_counter = counter;
			continue;
		}
		if (passw[counter-passw_counter] != string[counter]) {
			return 1;
		}
		++counter;
	}
	return 0;
}
