#include "gwan.h"


int main(int args, char *argv[])
{
		xbuf_t *reply = get_reply(argv);
		xbuf_cat(reply,
						"0001:vasil:123:admin"
						"|0002:petro:456:manager"
						"|0003:stranger:17:manager");
		return HTTP_200_OK;
}
