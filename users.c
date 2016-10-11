#include "gwan.h"
#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>	//strtoll

#include "f_store.h"

#pragma link "/home/mialso/projects/vs_pm/gwan_linux64-bit/libraries/libfstore.so"

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

struct User_store {
	kv_t *user_kv;
	struct F_store *user_store;
};

static int add_fsitem_to_kv(kv_t *kv_store, struct F_item *fs_item);
static void user_to_buf(xbuf_t *buf, struct F_item *user);
static int get_user_from_req(char *data, u64 data_len, struct F_item *user);
static void write_all_users(struct F_store *store, xbuf_t *buffer);

int main(int argc, char *argv[])
{
	char *tmp_str = NULL;
	xbuf_t *reply = get_reply(argv);

	u64 method = (u64) get_env(argv, REQUEST_METHOD);

	struct F_item req_user = {0};
	struct F_item *cur_user = NULL;

	struct User_store **glob_store = (struct User_store **) get_env(argv, US_SERVER_DATA);
	struct F_store *fs_store = (*glob_store)->user_store;
	kv_t *users = (*glob_store)->user_kv;

	if (1 == method || 5 == method) 
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
			write_all_users(fs_store, reply);
			return HTTP_200_OK;
		}
		strncpy(req_user.name, tmp_str, NAME_SIZE);
		cur_user = (struct F_item *) kv_get(users, req_user.name, strlen(req_user.name));
		if (5 == method) {
			// serve delete request
			u64 fs_id = 0;
			u64 res = 0;
			if (!cur_user) {
				xbuf_cat(reply, "d:[ERROR]: remove user: no such user");
				return HTTP_200_OK;
			}
			if (!(res = kv_del(users, cur_user->name, strlen(cur_user->name)))) {
				xbuf_cat(reply, "e:[ERROR]: remove from kv failed");
				return HTTP_200_OK;
			}
			fs_id = (((u64)cur_user - (u64)fs_store->data)/(sizeof(struct F_item)))+1;
			if ((res = delete_item(fs_store, fs_id))) {
				xbuf_cat(reply, "g:[ERROR]: delete from f_store failed");
				return HTTP_200_OK;
			}
			xbuf_cat(reply, "0");
			return HTTP_200_OK;
		}
		tmp_str = NULL;
		get_arg("key=", &tmp_str, argc, argv);
		if (NULL == tmp_str || ('\0' == *(tmp_str))) {
			xbuf_cat(reply, "3:[ERROR]: no key provided");
			return HTTP_200_OK;
		} else if (31 < strlen(tmp_str)) {
			xbuf_cat(reply, "4:[ERROR]: key is too long");
			return HTTP_200_OK;
		}
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
   	else
   	{
		if (2 == method || 4 < method) {
			xbuf_xcat(reply, "f:[ERROR]: such type(method=%d) of HTTP request is not served", method);
			return HTTP_200_OK;
		}
		char *req_data = (char *) get_env(argv, REQ_ENTITY);
		u64 content_len = (u64) get_env(argv, CONTENT_LENGTH);
		u64 res = 0;
		u64 fs_id = 0;
		if ((2 > content_len) || (MAX_REQ_DATA < content_len)) {
			xbuf_xcat(reply, "7:[ERROR]: data size ={%d} is wrong, the range is [1..%d]", content_len, MAX_REQ_DATA);
			return HTTP_200_OK;
		}
		if ((res = get_user_from_req(req_data, content_len, &req_user))) {
			xbuf_xcat(reply, "8:[ERROR]: user data is not valid: parser error=[%d]", res);
			return HTTP_200_OK;
		}
		cur_user = (struct F_item *) kv_get(users, req_user.name, strlen(req_user.name));
		switch (method) {
			case 4: 	// PUT
				if (!cur_user) {
					xbuf_xcat(reply, "9:[ERROR]: no user to update");
					return HTTP_200_OK;
				}
				strncpy(cur_user->key, req_user.key, KEY_SIZE-1);
				cur_user->role = req_user.role;
				//cur_user->id = req_user.id;
				user_to_buf(reply, cur_user);
				return HTTP_200_OK;
			case 3:		// POST
				if (cur_user) {
					xbuf_xcat(reply, "a:[ERROR]: valid user provided in POST, use PUT to update");
					return HTTP_200_OK;
				}
				// add new user
				if (!req_user.id) {
					// TODO create random and check for uniqueness
					req_user.id = getms();
				}
				fs_id = add_instance(fs_store, &req_user);
				if (!fs_id) {
					xbuf_cat(reply, "b:[ERROR]: <f_store>: add_instance fail");
					return HTTP_200_OK;
				}
				cur_user = get_item(fs_store, fs_id);
				if(0 != add_fsitem_to_kv(users, cur_user)) {
					// TODO remove user from file storage
					xbuf_cat(reply, "c:[ERROR]: add_fsitem_to kv fail");
					return HTTP_200_OK;
				}
				user_to_buf(reply, cur_user);
				return HTTP_200_OK;
			default:
				xbuf_xcat(reply, "f:[ERROR]: such type(method=%d) of HTTP request is not served", method);
				return HTTP_200_OK;
		}
	}
}
void write_all_users(struct F_store *store, xbuf_t *buffer)
{
	u64 counter = 1;
	while (counter <= store->size) {
		struct F_item *user = get_item(store, counter);
		if (user) {
			user_to_buf(buffer, user);
		}
		++counter;
	}
}
void user_to_buf(xbuf_t *buf, struct F_item *user)
{
	xbuf_xcat(buf, "|%lu:%s:%s:%d",
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
	u64 role = 0;
	u64 offset = 0;
	if (!data) {
		return 255;
	}
	char cur_char = *(data+counter);
	if (',' == cur_char) return 1;
	while ((',' != cur_char) && (counter < data_len)) {
		if ((NAME_SIZE-1) <= counter) {
			return 2;
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
			return 3;
		}
		*(user->key+key_counter) = cur_char;
		++counter;
		cur_char = *(data+counter);
	}
	++counter;
	if (counter >= data_len) {
		return 4;
	}
	if (!(role = strtoll(data+counter, NULL, 10))) {
		// TODO extract more information from errno
		return 5;
	}
	user->role = role;
	return 0;
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
