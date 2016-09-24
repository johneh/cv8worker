/* ../bin/jsi -p ../bin -f math.js */

var math = $load('./libmath.so');
$print('PI =', math.PI);
$print('1 + 1 =', math.add(1, 1));
$print('1 - 1 =', math.sub(1, 1));
