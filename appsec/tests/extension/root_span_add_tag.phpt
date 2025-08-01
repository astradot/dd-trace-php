--TEST--
Test ddtrace_root_span_add_tag
--ENV--
DD_TRACE_GENERATE_ROOT_SPAN=0
DD_CODE_ORIGIN_FOR_SPANS_ENABLED=0
DD_TRACE_AUTO_FLUSH_ENABLED=0
DD_SERVICE=appsec_tests
--INI--
extension=ddtrace.so
datadog.appsec.log_file=/tmp/php_appsec_test.log
datadog.appsec.log_level=debug
--FILE--
<?php
include __DIR__ . '/inc/ddtrace_version.php';

ddtrace_version_at_least('0.79.0');
// Fail if root span not available
var_dump(\datadog\appsec\testing\root_span_add_tag("before", "root_span"));
DDTrace\start_span();
var_dump(\datadog\appsec\testing\root_span_add_tag("after", "root_span"));
// Fail if we attempt to add an existing tag
var_dump(\datadog\appsec\testing\root_span_add_tag("after", "duplicate"));
DDTrace\close_span(0);
var_dump(dd_trace_serialize_closed_spans());
?>
--EXPECTF--
bool(false)
bool(true)
bool(false)
array(1) {
  [0]=>
  array(10) {
    ["trace_id"]=>
    string(%d) "%d"
    ["span_id"]=>
    string(%d) "%d"
    ["start"]=>
    int(%d)
    ["duration"]=>
    int(%d)
    ["name"]=>
    string(21) "root_span_add_tag.php"
    ["resource"]=>
    string(21) "root_span_add_tag.php"
    ["service"]=>
    string(12) "appsec_tests"
    ["type"]=>
    string(3) "cli"
    ["meta"]=>
    array(4) {
      ["_dd.p.dm"]=>
      string(2) "-0"
      ["_dd.p.tid"]=>
      string(16) "%s"
      ["after"]=>
      string(9) "root_span"
      ["runtime-id"]=>
      string(%d) %s
    }
    ["metrics"]=>
    array(6) {
      [%s"]=>
      float(%d)
      ["_sampling_priority_v1"]=>
      float(1)
      ["php.compilation.total_time_ms"]=>
      float(%s)
      ["php.memory.peak_real_usage_bytes"]=>
      float(%f)
      ["php.memory.peak_usage_bytes"]=>
      float(%f)
      ["process_id"]=>
      float(%f)
    }
  }
}
