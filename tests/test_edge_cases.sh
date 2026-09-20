#!/bin/bash

set -u

PROGRAM="./logpulse"

echo "================================"
echo " LogPulse Edge Case Tests"
echo "================================"
echo

pass=0
fail=0

check()
{
    name="$1"
    shift

    if "$@" > /dev/null 2>&1
    then
        echo "PASS: $name"
        pass=$((pass + 1))
    else
        echo "FAIL: $name"
        fail=$((fail + 1))
    fi
}

echo "1. Normal dataset"
check \
    "100K log processing" \
    "$PROGRAM" samples/application_100k.log 4

echo
echo "2. Anomaly detection"
check \
    "Anomaly test" \
    "$PROGRAM" samples/anomaly_test.log --anomalies 2

echo
echo "3. JSON output"
if "$PROGRAM" samples/anomaly_test.log --json 2 > tests/output.json 2>/dev/null &&
   python3 -m json.tool tests/output.json > /dev/null 2>&1
then
    echo "PASS: Valid JSON"
    pass=$((pass + 1))
else
    echo "FAIL: Valid JSON"
    fail=$((fail + 1))
fi

echo
echo "4. Empty file"

touch tests/empty.log

if "$PROGRAM" tests/empty.log 2>/dev/null
then
    echo "PASS: Empty file"
    pass=$((pass + 1))
else
    echo "FAIL: Empty file"
    fail=$((fail + 1))
fi

echo
echo "5. Missing file"

if ! "$PROGRAM" tests/does_not_exist.log 2>/dev/null
then
    echo "PASS: Missing file handled"
    pass=$((pass + 1))
else
    echo "FAIL: Missing file handling"
    fail=$((fail + 1))
fi

echo
echo "6. Invalid worker count"

if ! "$PROGRAM" samples/application.log 0 2>/dev/null
then
    echo "PASS: Worker count 0 rejected"
    pass=$((pass + 1))
else
    echo "FAIL: Worker count 0"
    fail=$((fail + 1))
fi

if ! "$PROGRAM" samples/application.log 33 2>/dev/null
then
    echo "PASS: Worker count 33 rejected"
    pass=$((pass + 1))
else
    echo "FAIL: Worker count 33"
    fail=$((fail + 1))
fi

echo
echo "================================"
echo "Passed: $pass"
echo "Failed: $fail"
echo "================================"

if [ "$fail" -eq 0 ]
then
    exit 0
else
    exit 1
fi
