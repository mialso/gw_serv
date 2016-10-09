#!/bin/sh

#  user servlet test suite
servlet_name="users"
host_addr="localhost:3000"

valid_name="vvosl"
right_name="vasil"
wrong_name="xxx"
long_name="sklajflkasdfjklavaksdhvskdhaklsdhfklsjfksjflksjdljfa"

valid_key="some"
right_key="123"
wrong_key="any"
long_key="sakdfhkjashfkjahsdfkjsdhfkshfkhkjdfhskjdfhksjdhfjkdhfk"


test_success=0
test_fail=0
echo "user servlet test: [START]"

echo "GET test suite:"

case="case 1: no user name provided 1"

res=$(curl --silent http://${host_addr}/?${servlet_name}/name=/key=a | head -c 1)
[ ! $? -eq 0 ] && echo "curl error"
if [ $res -eq 1 ];then
	test_success=$((test_success+1))
else
	test_fail=$((test_fail+1))
	echo "[FAIL]: ${case}"
fi

case="case 2: no user name provided 2"
res=$(curl --silent http://${host_addr}/?${servlet_name}/key=a | head -c 1)
[ ! $? -eq 0 ] && echo "curl error"
if [ $res -eq 1 ];then
	test_success=$((test_success+1))
else
	test_fail=$((test_fail+1))
	echo "[FAIL]: ${case}"
fi

case="case 3: user name is too long"
res=$(curl --silent http://${host_addr}/?${servlet_name}/name=${long_name}/key=a | head -c 1)
[ ! $? -eq 0 ] && echo "curl error"
if [ $res -eq 2 ];then
	test_success=$((test_success+1))
else
	test_fail=$((test_fail+1))
	echo "[FAIL]: ${case}"
fi

case="case 4: no key provided 1"
res=$(curl --silent http://${host_addr}/?${servlet_name}/name=${valid_name}/key= | head -c 1)
[ ! $? -eq 0 ] && echo "curl error"
if [ $res -eq 3 ];then
	test_success=$((test_success+1))
else
	test_fail=$((test_fail+1))
	echo "[FAIL]: ${case}"
fi

case="case 5: no key provided 2"
res=$(curl --silent http://${host_addr}/?${servlet_name}/name=${valid_name} | head -c 1)
[ ! $? -eq 0 ] && echo "curl error"
if [ $res -eq 3 ];then
	test_success=$((test_success+1))
else
	test_fail=$((test_fail+1))
	echo "[FAIL]: ${case}"
fi

case="case 6: key is too long"
res=$(curl --silent http://${host_addr}/?${servlet_name}/name=${valid_name}/key=${long_key} | head -c 1)
[ ! $? -eq 0 ] && echo "curl error"
if [ $res -eq 4 ];then
	test_success=$((test_success+1))
else
	test_fail=$((test_fail+1))
	echo "[FAIL]: ${case}"
fi

case="case 7: wrong user name provided"
res=$(curl --silent http://${host_addr}/?${servlet_name}/name=${wrong_name}/key=${valid_key} | head -c 1)
[ ! $? -eq 0 ] && echo "curl error"
if [ $res -eq 5 ];then
	test_success=$((test_success+1))
else
	test_fail=$((test_fail+1))
	echo "[FAIL]: ${case}"
fi

case="case 8: wrong key provided"
res=$(curl --silent http://${host_addr}/?${servlet_name}/name=${right_name}/key=${valid_key} | head -c 1)
[ ! $? -eq 0 ] && echo "curl error"
if [ $res -eq 6 ];then
	test_success=$((test_success+1))
else
	test_fail=$((test_fail+1))
	echo "[FAIL]: ${case}"
fi
echo "tests success: $test_success; tests fail: $test_fail"

echo "POST test suite:"
test_success=0
test_fail=0
echo "tests success: $test_success; tests fail: $test_fail"

exit 0

