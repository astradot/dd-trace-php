--TEST--
Span properties are safely converted to strings without errors or exceptions
--SKIPIF--
<?php
if (version_compare(PHP_VERSION, '7.4.0', '>='))
    die('skip: test only works before 7.4');
?>
--FILE--
<?php
use DDTrace\SpanData;

date_default_timezone_set('UTC');

class MyDt extends DateTime {
    public function __toString() {
        return $this->format('Y-m-d');
    }
}

function prop_to_string($data) {}

DDTrace\trace_function('prop_to_string', function (SpanData $span, array $args) {
    $span->name = $args[0];
    $span->resource = $args[0];
    $span->service = $args[0];
    $span->type = $args[0];
});

$allTheTypes = [
    'already a string',
    42,
    4.2,
    true,
    false,
    null,
    function () {},
    new DateTime('2019-09-10'),
    new MyDt('2019-09-10'),
    ['foo' => 0],
    fopen('php://memory', 'rb'), // resource
];
foreach ($allTheTypes as $value) {
    prop_to_string($value);
}

// Spans are on a stack so we reverse the types
$allTheTypes = array_reverse($allTheTypes);

$i = 0;
foreach (dd_trace_serialize_closed_spans() as $span) {
    var_dump($allTheTypes[$i]);
    foreach (['name', 'resource', 'service', 'type'] as $prop) {
        if (isset($span[$prop]) && $span[$prop] !== "") {
            var_dump($span[$prop]);
        } else {
            printf("'%s' dropped\n", $prop);
        }
    }
    echo PHP_EOL;
    $i++;
}
?>
--EXPECTF--
resource(%d) of type (stream)
string(%d) "Resource id #%d"
string(%d) "Resource id #%d"
string(%d) "Resource id #%d"
string(%d) "Resource id #%d"

array(1) {
  ["foo"]=>
  int(0)
}
string(5) "Array"
string(5) "Array"
string(5) "Array"
string(5) "Array"

object(MyDt)#%d (3) {
  ["date"]=>
  string(26) "2019-09-10 00:00:00.000000"
  ["timezone_type"]=>
  int(3)
  ["timezone"]=>
  string(3) "UTC"
}
string(%d) "object(MyDt)#%d"
string(%d) "object(MyDt)#%d"
string(%d) "object(MyDt)#%d"
string(%d) "object(MyDt)#%d"

object(DateTime)#%d (3) {
  ["date"]=>
  string(26) "2019-09-10 00:00:00.000000"
  ["timezone_type"]=>
  int(3)
  ["timezone"]=>
  string(3) "UTC"
}
string(%d) "object(DateTime)#%d"
string(%d) "object(DateTime)#%d"
string(%d) "object(DateTime)#%d"
string(%d) "object(DateTime)#%d"

object(Closure)#%d (0) {
}
string(%d) "object(Closure)#%d"
string(%d) "object(Closure)#%d"
string(%d) "object(Closure)#%d"
string(%d) "object(Closure)#%d"

NULL
'name' dropped
'resource' dropped
'service' dropped
'type' dropped

bool(false)
string(5) "false"
'resource' dropped
string(5) "false"
string(5) "false"

bool(true)
string(4) "true"
string(4) "true"
string(4) "true"
string(4) "true"

float(4.2)
string(3) "4.2"
string(3) "4.2"
string(3) "4.2"
string(3) "4.2"

int(42)
string(2) "42"
string(2) "42"
string(2) "42"
string(2) "42"

string(16) "already a string"
string(16) "already a string"
string(16) "already a string"
string(16) "already a string"
string(16) "already a string"
