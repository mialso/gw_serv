#include "gwan.h"
#include <malloc.h>
#include <string.h>
#include <stdio.h>

#include "f_store.h"

#pragma link "/home/mialso/projects/vs_pm/gwan_linux64-bit/libraries/libfstore.so"

#define STORE_PATH "data/user.fstr"
#define ADM "vasil"
#define ADM_KEY "123"

#define MAX_REQ_DATA 63
#define USER_STRING_MAX_LEN 512
#define MAX_REPLY 4096

enum Roles {
	R_GUEST,
	R_MANAGER,
	R_ADMIN
};

struct F_store fs_store;
kv_t users;

static int validate_key_fail(char *string, char *key);
static int add_admin(kv_t *kv_store, struct F_store *fs_store);
static int add_fsitem_to_kv(kv_t *kv_store, struct F_item *fs_item);
static int init_kv_store(struct F_store *fs_store, kv_t *kv_store);
static int print_each(const kv_item *item, const void *string);
//static int user_to_string(char *res_string, struct F_item *user);
static void user_to_buf(xbuf_t *buf, struct F_item *user);
static int get_user_from_req(char *data, u64 data_len, struct F_item *user);

int main(int args, char *argv[])
{
		char str[MAX_REPLY] = {0};
		char store_path[1024] = {0};
		char *req_data = (char *) get_env(argv, REQ_ENTITY);
		u64 content_len = (u64) get_env(argv, CONTENT_LENGTH);
		xbuf_t *reply = get_reply(argv);

		struct F_item req_user = {0};
		struct F_item *cur_user = NULL;

		if (MAX_REQ_DATA < content_len) {
			xbuf_cat(reply, "[ERROR]: too long data, max len is 63");
			return HTTP_200_OK;
		}
		if (0 == content_len) {
			kv_do(&users, NULL, 0, print_each, (void*)reply);
			//xbuf_cat(reply, str);
			s_snprintf(str, sizeof(str)-1, "req_data =%s", "content len was 0");
			log_err(argv, str);
			return HTTP_200_OK;
		}
		s_snprintf(str, sizeof(str)-1, "[INFO]: req_data =%s, content_len =%d", req_data, content_len);
		log_err(argv, str);
		if ('\0' == users.name[0]) {
				// init f_store
				//strcpy(store_path, (char *)get_env(argv, VHOST_ROOT);
				s_snprintf(store_path, sizeof(store_path)-1, "%s/%s",
					   	(char *)get_env(argv, VHOST_ROOT),
						STORE_PATH
				);
				s_snprintf(fs_store.name, 7, "user");
				if (init_store(store_path, &fs_store)) {
					s_snprintf(str, sizeof(str)-1, "[ERROR]: users f_store initialization failed, path =%s", store_path);
					log_err(argv, str);
					return HTTP_500_INTERNAL_SERVER_ERROR;
				}
				s_snprintf(str, sizeof(str)-1, "[OK]: user f_store of <%d> size initialized", fs_store.size);
				log_err(argv, str);
				// init kv store
				kv_init(&users, "users", fs_store.size, 0, 0, 0);
				if(init_kv_store(&fs_store, &users)) {
					log_err(argv, "[ERROR]: init_kv_store(): unable to init");
					return HTTP_500_INTERNAL_SERVER_ERROR;
				}
				log_err(argv, "[OK]: {users} kv store initialized");

				if (NULL == (struct F_item *) kv_get(&users, ADM, sizeof(ADM)-1)) {
					if (add_admin(&users, &fs_store)) {
						return HTTP_500_INTERNAL_SERVER_ERROR;
					}
				}
		}
		if (get_user_from_req(req_data, content_len, &req_user)) {
				xbuf_cat(reply, "wrd");
				log_err(argv, "unable to get user name from request");
				return HTTP_200_OK;
		}
		cur_user = (struct F_item *) kv_get(&users, req_user.name, strlen(req_user.name));
		if (!cur_user) {
				xbuf_cat(reply, "epic_fail");
				log_err(argv, "no user found in users store");
				return HTTP_200_OK;
		}
		if ((validate_key_fail(cur_user->key, req_user.key))) {
				xbuf_cat(reply, "epic_fail");
				log_err(argv, "password fail");
				return HTTP_200_OK;
		}
		//user_to_string(str, cur_user);
		user_to_buf(reply, cur_user);
		//xbuf_cat(reply, str);

		return HTTP_200_OK;
}
//int user_to_string(xbuf_t *buf, struct F_item *user)
void user_to_buf(xbuf_t *buf, struct F_item *user)
{
	//return s_snprintf(res_string, USER_STRING_MAX_LEN, "%d:%s:%s:%d|",
	xbuf_xcat(buf, "|%d:%s:%s:%d",
						user->id,
						user->name,
						user->key,
						user->role
	);
}
int get_user_from_req(char *data, u64 data_len, struct F_item *user)
{
	u64 counter = 0;
	u64 key_counter = 0;
	char cur_char;
	while(counter < data_len) {
		cur_char = *(data+counter);
		if ((',' != cur_char) && (!key_counter)) {
			if (NAME_SIZE <= counter) {
				fprintf(stderr, "get_user_from_req(): name is too long\n");
				return 1;
			}
			*(user->name+counter) = cur_char;
			++counter;
			continue;
		}
		if ((',' == cur_char) && (!key_counter)) {
			++counter;
			key_counter = counter;
			continue;
		}
		if (KEY_SIZE <= (counter-key_counter)) {
			fprintf(stderr, "get_user_from_req(): key is too log\n");
			return 2;
		}
		*(user->key+(counter-key_counter)) = cur_char;
		++counter;
	}
	return 0;
}	
int validate_key_fail(char *string, char *key)
{
	return 0;
}
int init_kv_store(struct F_store *fs_store, kv_t *kv_store)
{
	struct F_item *fs_item;
	u64 counter = 1;
	while (counter <= fs_store->size) {
		fs_item = get_item(fs_store, counter);
		if (fs_item) {
			if(add_fsitem_to_kv(kv_store, fs_item)) {
				fprintf(stderr, "[ERROR]: unable to add item to kv\n");
				return 1;
			}
		}
		++counter;
	}
	return 0;
}
int add_fsitem_to_kv(kv_t *kv_store, struct F_item *fs_item)
{
	kv_item kvitem = {0};

	kvitem.key = fs_item->name;
	kvitem.klen = strlen(fs_item->name);
	kvitem.val = (char *) fs_item;
	kvitem.in_use = 0;
	if(!kv_add(kv_store, &kvitem)) {
		fprintf(stderr, "[ERROR]: kv_add(): out_of_memory\n");
		return 1;
	}
	return 0;
}
int add_admin(kv_t *kv_store, struct F_store *fs_store)
{
	struct F_item user = {0};
	struct F_item *fs_user = NULL;
	uint s_id = 0;
	int res = 0;

	fprintf(stderr, "[INFO]: adding admin user\n");
	strcpy(user.name, ADM); 
	strcpy(user.key, ADM_KEY);
	user.role = R_ADMIN;
	user.id = 23;
	s_id = add_instance(fs_store, &user);
	if (!s_id) {
		fprintf(stderr, "[ERROR]: unable to add admin user during init\n");
		return 1;
	}
	fs_user = get_item(fs_store, s_id);
	if(!fs_user) {
		fprintf(stderr, "[ERROR]: unable to get admin after creation\n");
		return 2;
	}
	if ((res = add_fsitem_to_kv(kv_store, fs_user))) {
		return 3;
	}
	return 0;
}
int print_each(const kv_item *item, const void *reply)
{
		/*
	char *res_str = (char *)string;
	u64 len = strlen(res_str);
	if (len > MAX_REPLY) {
		// unable to print more than buffer size
		return 0;
	}
	*/
	struct F_item *user = (struct F_item *)item->val;
	//user_to_string(res_str+len, user);
	user_to_buf((xbuf_t *)reply, user);
	return 1;
}
