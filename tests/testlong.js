var i1 = $toLong("123456789");
$print($formatLong(i1));
var i2 = $toLong("123456789", true);
$print($formatLong(i1, "s"));

i1 = $toLong("-123456789");
$print($formatLong(i1, "s"));
i2 = $toLong("-123456789", true);
$print($formatLong(i1));

i1 = $toLong("1234567890123");
var low = $formatLong(i1, 'l');
var high = $formatLong(i1, 'h');
i2 = $toLong([low, high]);
$print($formatLong(i1), $formatLong(i2));

i1 = $toLong("143141355151351515616565165161613");
$print($formatLong(i1));	// == "0"

$print($formatLong($toLong([0, 0])),
		$formatLong($toLong([1, 0])),
		$formatLong($toLong({}))	// == 0
);
$print($formatLong($toLong([-1, -1], true)));	// UINT64_MAX
$print($formatLong($toLong([-1, ~(1 << 31)])));  // INT64_MAX
$print($formatLong($toLong([0, (1 << 31)])));   // INT64_MIN

$print($formatLong($toLong(1.0/0/0)),
		$formatLong($toLong(0.0/0.0)));
