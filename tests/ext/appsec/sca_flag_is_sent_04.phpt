--TEST--
DD_APPSEC_SCA_ENABLED is set by INI
--DESCRIPTION--
This configuration is used by the backend to display/charge customers
--SKIPIF--
<?php
if (getenv('PHP_PEAR_RUNTESTS') === '1') die("skip: pecl run-tests does not support {PWD}");
if (PHP_OS === "WINNT" && PHP_VERSION_ID < 70400) die("skip: Windows on PHP 7.2 and 7.3 have permission issues with synchronous access to telemetry");
if (getenv('USE_ZEND_ALLOC') === '0' && !getenv("SKIP_ASAN")) die('skip timing sensitive test - valgrind is too slow');
require __DIR__ . '/../includes/clear_skipif_telemetry.inc'
?>
--ENV--
DD_TRACE_GENERATE_ROOT_SPAN=0
DD_INSTRUMENTATION_TELEMETRY_ENABLED=1
--INI--
datadog.trace.agent_url="file://{PWD}/sca_flag_is_sent_04-telemetry.out"
datadog.appsec.sca_enabled=1
--FILE_EXTERNAL--
sca_test.inc
--EXPECT--
array(4) {
  ["name"]=>
  string(18) "appsec.sca_enabled"
  ["value"]=>
  string(1) "1"
  ["origin"]=>
  string(7) "env_var"
  ["config_id"]=>
  NULL
}
string(4) "Sent"
--CLEAN--
<?php

@unlink(__DIR__ . '/sca_flag_is_sent_04-telemetry.out');
