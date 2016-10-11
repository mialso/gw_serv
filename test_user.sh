#!/bin/sh

#  user servlet test suite data
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

long_data="sssssssssssssssssssooooooooooooooooooommmmmmmmmmmmmmmmmmeeeeeeeeeeeeeeee long DATAAAAAAAAAAA"

invalid_user=",some_key,1"
test_user="test_user,some_key,1"
wrong_user="no_such_UUser,no_such-KEY,2"

wrong_user_name="some_wrong_name"
test_user_name="test_user"
test_user_updated="test_user,neew_key,2"

#  start servlet test
echo "user servlet test: [START]"

echo "REAL create-update-delete test"
test_success=0
test_fail=0
case="case 1: create test user"
res=$(curl --silent -X POST -d ${test_user} http://${host_addr}/?${servlet_name} | head -c 1)
[ ! $? -eq 0 ] && echo "curl error"
if [ "$res" = "|" ]; then
	test_success=$((test_success+1))
else
	test_fail=$((test_fail+1))
	echo "[FAIL]: ${case} result = ${res}"
fi
case="case 2: delete test user"
res=$(curl --silent -X DELETE http://${host_addr}/?${servlet_name}/name=${test_user_name})
[ ! $? -eq 0 ] && echo "curl error"
if [ "$res" = "0" ]; then
	test_success=$((test_success+1))
else
	test_fail=$((test_fail+1))
	echo "[FAIL]: ${case} result = ${res}"
fi
case="case 3: create test user"
res=$(curl --silent -X POST -d ${test_user} http://${host_addr}/?${servlet_name} | head -c 1)
[ ! $? -eq 0 ] && echo "curl error"
if [ "$res" = "|" ]; then
	test_success=$((test_success+1))
else
	test_fail=$((test_fail+1))
	echo "[FAIL]: ${case} result = ${res}"
fi
case="case 4: update test user"
res=$(curl --silent -X PUT -d ${test_user_updated} http://${host_addr}/?${servlet_name} | head -c 1)
[ ! $? -eq 0 ] && echo "curl error"
if [ "$res" = "|" ]; then
	test_success=$((test_success+1))
else
	test_fail=$((test_fail+1))
	echo "[FAIL]: ${case} result = ${res}"
fi
case="case 5: delete updated user"
res=$(curl --silent -X DELETE http://${host_addr}/?${servlet_name}/name=${test_user_name})
[ ! $? -eq 0 ] && echo "curl error"
if [ "$res" = "0" ]; then
	test_success=$((test_success+1))
else
	test_fail=$((test_fail+1))
	echo "[FAIL]: ${case} result = ${res}"
fi
echo "tests success: $test_success; tests fail: $test_fail"

echo "GET DELETE common suite:"
test_success=0
test_fail=0

case="case 1.1: no user name provided 1"
res=$(curl --silent http://${host_addr}/?${servlet_name}/name=/key=a | head -c 1)
[ ! $? -eq 0 ] && echo "curl error"
if [ $res -eq 1 ];then
	test_success=$((test_success+1))
else
	test_fail=$((test_fail+1))
	echo "[FAIL]: ${case}"
fi

case="case 1.2: no user name provided 2"
res=$(curl --silent http://${host_addr}/?${servlet_name}/key=a | head -c 1)
[ ! $? -eq 0 ] && echo "curl error"
if [ $res -eq 1 ];then
	test_success=$((test_success+1))
else
	test_fail=$((test_fail+1))
	echo "[FAIL]: ${case}"
fi

case="case 2: user name is too long"
res=$(curl --silent http://${host_addr}/?${servlet_name}/name=${long_name}/key=a | head -c 1)
[ ! $? -eq 0 ] && echo "curl error"
if [ $res -eq 2 ];then
	test_success=$((test_success+1))
else
	test_fail=$((test_fail+1))
	echo "[FAIL]: ${case}"
fi

case="case 3.1: no key provided 1"
res=$(curl --silent http://${host_addr}/?${servlet_name}/name=${valid_name}/key= | head -c 1)
[ ! $? -eq 0 ] && echo "curl error"
if [ $res -eq 3 ];then
	test_success=$((test_success+1))
else
	test_fail=$((test_fail+1))
	echo "[FAIL]: ${case}"
fi

case="case 3.2: no key provided 2"
res=$(curl --silent http://${host_addr}/?${servlet_name}/name=${valid_name} | head -c 1)
[ ! $? -eq 0 ] && echo "curl error"
if [ $res -eq 3 ];then
	test_success=$((test_success+1))
else
	test_fail=$((test_fail+1))
	echo "[FAIL]: ${case}"
fi

case="case 4: key is too long"
res=$(curl --silent http://${host_addr}/?${servlet_name}/name=${valid_name}/key=${long_key} | head -c 1)
[ ! $? -eq 0 ] && echo "curl error"
if [ $res -eq 4 ];then
	test_success=$((test_success+1))
else
	test_fail=$((test_fail+1))
	echo "[FAIL]: ${case}"
fi

case="case 5: wrong user name provided"
res=$(curl --silent http://${host_addr}/?${servlet_name}/name=${wrong_name}/key=${valid_key} | head -c 1)
[ ! $? -eq 0 ] && echo "curl error"
if [ $res -eq 5 ];then
	test_success=$((test_success+1))
else
	test_fail=$((test_fail+1))
	echo "[FAIL]: ${case}"
fi

case="case 6: wrong key provided"
res=$(curl --silent http://${host_addr}/?${servlet_name}/name=${right_name}/key=${valid_key} | head -c 1)
[ ! $? -eq 0 ] && echo "curl error"
if [ $res -eq 6 ];then
	test_success=$((test_success+1))
else
	test_fail=$((test_fail+1))
	echo "[FAIL]: ${case}"
fi
#TODO
#case="case 7: get default admin user"
echo "tests success: $test_success; tests fail: $test_fail"

echo "POST PUT common suite:"
test_success=0
test_fail=0

case="case 1.1:no data provided"
res=$(curl --silent -X POST -d 1 http://${host_addr}/?${servlet_name} | head -c 1)
[ ! $? -eq 0 ] && echo "curl error"
if [ $res -eq 7 ]; then
	test_success=$((test_success+1))
else
	test_fail=$((test_fail+1))
	echo "[FAIL]: ${case}"
fi
case="case 1.2: too long data provided"
res=$(curl --silent -X POST -d ${long_data} http://${host_addr}/?${servlet_name} | head -c 1)
[ ! $? -eq 0 ] && echo "curl error"
if [ $res -eq 7 ]; then
	test_success=$((test_success+1))
else
	test_fail=$((test_fail+1))
	echo "[FAIL]: ${case}"
fi

case="case 2: invalid user data provided"
res=$(curl --silent -X POST -d ${invalid_user} http://${host_addr}/?${servlet_name} | head -c 1)
[ ! $? -eq 0 ] && echo "curl error"
if [ "$res" = "8" ]; then
	test_success=$((test_success+1))
else
	test_fail=$((test_fail+1))
	echo "[FAIL]: ${case}"
fi
echo "tests success: $test_success; tests fail: $test_fail"

echo "POST suite:"
test_success=0
test_fail=0
case="case 1: create test user"
res=$(curl --silent -X POST -d ${test_user} http://${host_addr}/?${servlet_name} | head -c 1)
[ ! $? -eq 0 ] && echo "curl error"
if [ "$res" = "|" ]; then
	test_success=$((test_success+1))
else
	test_fail=$((test_fail+1))
	echo "[FAIL]: ${case} result = ${res}"
fi
case="case 2: not new(existent) user data provided"
res=$(curl --silent -X POST -d ${test_user} http://${host_addr}/?${servlet_name} | head -c 1)
[ ! $? -eq 0 ] && echo "curl error"
if [ "$res" = "a" ]; then
	test_success=$((test_success+1))
else
	test_fail=$((test_fail+1))
	echo "[FAIL]: ${case}"
fi
case="case 3: delete test user"
res=$(curl --silent -X DELETE http://${host_addr}/?${servlet_name}/name=${test_user_name})
[ ! $? -eq 0 ] && echo "curl error"
if [ "$res" = "0" ]; then
	test_success=$((test_success+1))
else
	test_fail=$((test_fail+1))
	echo "[FAIL]: ${case} result = ${res}"
fi
echo "tests success: $test_success; tests fail: $test_fail"

echo "PUT test suite:"
test_success=0
test_fail=0

case="case 1: no such user data provided"
res=$(curl --silent -X PUT -d ${wrong_user} http://${host_addr}/?${servlet_name} | head -c 1)
[ ! $? -eq 0 ] && echo "curl error"
if [ "$res" = "9" ]; then
	test_success=$((test_success+1))
else
	test_fail=$((test_fail+1))
	echo "[FAIL]: ${case} result = ${res}"
fi
echo "tests success: $test_success; tests fail: $test_fail"

echo "DELETE test suite:"
test_success=0
test_fail=0
case="case 1: no such user data provided"
res=$(curl --silent -X DELETE http://${host_addr}/?${servlet_name}/name=${wrong_user_name} | head -c 1)
[ ! $? -eq 0 ] && echo "curl error"
if [ "$res" = "d" ]; then
	test_success=$((test_success+1))
else
	test_fail=$((test_fail+1))
	echo "[FAIL]: ${case} result = ${res}"
fi
echo "tests success: $test_success; tests fail: $test_fail"


exit 0

