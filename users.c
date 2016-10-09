#include "gwan.h"
#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

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

static int add_admin(kv_t *kv_store, struct F_store *fs_store);
static int add_fsitem_to_kv(kv_t *kv_store, struct F_item *fs_item);
static int init_kv_store(struct F_store *fs_store, kv_t *kv_store);
static int print_each(const kv_item *item, const void *string);
static void user_to_buf(xbuf_t *buf, struct F_item *user);
static int get_user_from_req(char *data, u64 data_len, struct F_item *user);

int main(int argc, char *argv[])
{
		char str[MAX_REPLY] = {0};
		char *tmp_str = NULL;
		char store_path[1024] = {0};
		xbuf_t *reply = get_reply(argv);

		u64 method = (u64) get_env(argv, REQUEST_METHOD);

		struct F_item req_user = {0};
		struct F_item *cur_user = NULL;

		if ('\0' == users.name[0]) {
				// init f_storee
				printf("store initialized\n");
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

		// TODO refactor to switch
		if (1 == method) 
		{
			// serve GET request
			get_arg("name=", &tmp_str, argc, argv);
			if (NULL == tmp_str || ('\0' == *(tmp_str))) {
				// no user name provided
				xbuf_cat(reply, "1:[ERROR]: no user name provided");
				return HTTP_200_OK;
			} else if (31 < strlen(tmp_str)) {
				// user name is tooooo long
				xbuf_cat(reply, "2:[ERROR]: user name is too long");
				return HTTP_200_OK;
			}
			if (!strcmp(tmp_str, "all")) {
				// reply all users
				printf("D0\n");
				kv_do(&users, NULL, 0, print_each, (void*)reply);
				return HTTP_200_OK;
			}
			strncpy(req_user.name, tmp_str, NAME_SIZE);
			tmp_str = NULL;
			get_arg("key=", &tmp_str, argc, argv);
			if (NULL == tmp_str || ('\0' == *(tmp_str))) {
				xbuf_cat(reply, "3:[ERROR]: no key provided");
				return HTTP_200_OK;
			} else if (31 < strlen(tmp_str)) {
				xbuf_cat(reply, "4:[ERROR]: key is too long");
				return HTTP_200_OK;
			}
			cur_user = (struct F_item *) kv_get(&users, req_user.name, strlen(req_user.name));
			if (!cur_user) {
				xbuf_cat(reply, "5:[ERROR]: no such user in store");
				return HTTP_200_OK;
			}
			if (strcmp(cur_user->key, tmp_str)) {
				xbuf_cat(reply, "6:[ERROR]: user key is not equal to stored one");
				return HTTP_200_OK;
			}
			user_to_buf(reply, cur_user);
			return HTTP_200_OK;

		}
	   	else if (4 == method || 3 == method)
	   	{
			// server POST & PUT request
			char *req_data = (char *) get_env(argv, REQ_ENTITY);
			u64 content_len = (u64) get_env(argv, CONTENT_LENGTH);
			u64 res = 0;
			if (MAX_REQ_DATA < content_len) {
				xbuf_cat(reply, "7:[ERROR]: too long data, max len is 63");
				return HTTP_200_OK;
			}
			if ((res = get_user_from_req(req_data, content_len, &req_user))) {
				xbuf_xcat(reply, "8:[ERROR]: user data is no valid: [%d]", res);
				return HTTP_200_OK;
			}
			cur_user = (struct F_item *) kv_get(&users, req_user.name, strlen(req_user.name));
			if (cur_user) {
				// TODO implement update user
				// update user
				strncpy(cur_user->key, req_user.key, KEY_SIZE-1);
				cur_user->role = req_user.role;
				cur_user->id = req_user.id;
				user_to_buf(reply, cur_user);
				return HTTP_200_OK;
			} else {
				// TODO implement add user
				// add new user
				u64 fs_id = 0;
				if (!req_user.id) {
					req_user.id = getms();
				}
				fs_id = add_instance(&fs_store, &req_user);
				if (!fs_id) {
					xbuf_cat(reply, "9:[ERROR]: add_instance fail");
					return HTTP_200_OK;
				}
				cur_user = get_item(&fs_store, fs_id);
				if(0 != add_fsitem_to_kv(&users, cur_user)) {
					xbuf_cat(reply, "a:[ERROR]: add_fsitem_to kv fail");
					return HTTP_200_OK;
				}
				user_to_buf(reply, cur_user);
				return HTTP_200_OK;
			}
		}
	   	else
	   	{
			// no other requests are served
			xbuf_xcat(reply, "f:[ERROR]: such type(method=%d) of HTTP request is not served", method);
			return HTTP_200_OK;
		}
}
void user_to_buf(xbuf_t *buf, struct F_item *user)
{
	printf("D1\n");
	xbuf_xcat(buf, "|%lu:%s:%s:%d",
						user->id,
						user->name,
						user->key,
						user->role
	);
	printf("L1\n");
}
int get_user_from_req(char *data, u64 data_len, struct F_item *user)
{
	u64 counter = 0;
	u64 key_counter = 0;
	u64 role = 0;
	u64 offset = 0;
	char cur_char = *(data+counter);
	while ((',' != cur_char) && (counter < data_len)) {
		if ((NAME_SIZE-1) <= counter) {
			return 1;
		}
		*(user->name+counter) = cur_char;
		++counter;
		cur_char = *(data+counter);
	}
	++counter;
	offset = counter;
	cur_char = *(data+counter);
	while((',' != cur_char) && (counter < data_len)) {
		key_counter = counter-offset;
		if ((KEY_SIZE-1) <= key_counter) {
			return 2;
		}
		*(user->key+key_counter) = cur_char;
		++counter;
		cur_char = *(data+counter);
	}
	++counter;
	if (counter >= data_len) {
		return 3;
	}
	if (!(role = strtoll(data+counter, NULL, 10))) {
		// TODO extract more information from errno
		return 4;
	}
	user->role = role;
	return 0;

	/*
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
	*/
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
	kv_item *new_item = NULL;
	if(NULL == (new_item = kv_add(kv_store, &kvitem))) {
		fprintf(stderr, "[ERROR]: kv_add(): out_of_memory\n");
		return 1;
	}
	printf("item {%s, %d: %p} added to kv[%ld at %p]\n", fs_item->name, kvitem.klen, fs_item, kv_store->nbr_items, new_item);
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
	//struct F_item *user = (struct F_item *)item->val;
	printf("D2: %p;\n", item->val);
	printf("D22: %d;\n", item->klen);
	printf("D3: %p;\n", item);
	if (30 < item->klen) {
		return 1;
	}
	user_to_buf((xbuf_t *)reply, (struct F_item *)item->val);
	return 1;
}
