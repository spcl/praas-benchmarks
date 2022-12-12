
REPETITIONS=100
URL="http://knative-noop.default.34.233.97.200.sslip.io"

SIZE_STEPS="10 1024 102400 1048576 5242880"

for size in ${SIZE_STEPS}
do
  echo "Perform 100 repetitions of size ${size}"
  ./tester/tester $URL ${size} $REPETITIONS 5 > result_$size.csv
  head result_$size.csv
  tail result_$size.csv
done

