--TEST--
Report user config telemetry
--SKIPIF--
<?php
if (getenv('PHP_PEAR_RUNTESTS') === '1') {
    die("skip: pecl run-tests does not support {PWD}");
}
if (PHP_OS === "WINNT" && PHP_VERSION_ID < 70400) {
    die("skip: Windows on PHP 7.2 and 7.3 have permission issues with synchronous access to telemetry");
}
if (getenv('USE_ZEND_ALLOC') === '0' && !getenv("SKIP_ASAN")) {
    die('skip timing sensitive test - valgrind is too slow');
}
require __DIR__ . '/../includes/clear_skipif_telemetry.inc'
?>
--ENV--
DD_TRACE_GENERATE_ROOT_SPAN=0
DD_TRACE_AUTOFINISH_SPANS=1
DD_INSTRUMENTATION_TELEMETRY_ENABLED=1
DD_AGENT_HOST=
DD_AUTOLOAD_NO_COMPILE=
DD_TRACE_GIT_METADATA_ENABLED=0
DD_TRACE_IGNORE_AGENT_SAMPLING_RATES=1
--INI--
datadog.trace.agent_url="file://{PWD}/config-telemetry.out"
--FILE--
<?php

DDTrace\start_span();

include __DIR__ . '/vendor/autoload.php';

DDTrace\close_span();

dd_trace_serialize_closed_spans();

dd_trace_internal_fn("finalize_telemetry");

for ($i = 0; $i < 300; ++$i) {
    usleep(100000);
    if (file_exists(__DIR__ . '/config-telemetry.out')) {
        foreach (file(__DIR__ . '/config-telemetry.out') as $l) {
            if ($l) {
                $json = json_decode($l, true);
                $batch = $json["request_type"] == "message-batch" ? $json["payload"] : [$json];
                foreach ($batch as $json) {
                    if ($json["request_type"] == "app-client-configuration-change") {
                        $cfg = $json["payload"]["configuration"];
                        print_r(array_values(array_filter($cfg, function ($c) {
                            return $c["origin"] == "env_var" && $c["name"] != "trace.sources_path" && $c["name"] != "trace.sidecar_trace_sender";
                        })));
                        var_dump(count(array_filter($cfg, function ($c) {
                            return $c["origin"] == "default";
                        })) > 100); // all the configs, no point in asserting them all here
                        var_dump(count(array_filter($cfg, function ($c) {
                            return $c["origin"] != "default" && $c["origin"] != "env_var";
                        }))); // all other configs
                        break 3;
                    }
                }
            }
        }
    }
}

?>
--EXPECTF--
Included
Array
(
    [0] => Array
        (
            [name] => trace.agent_url
            [value] => file://%s/config-telemetry.out
            [origin] => env_var
            [config_id] => 
        )

    [1] => Array
        (
            [name] => instrumentation_telemetry_enabled
            [value] => 1
            [origin] => env_var
            [config_id] => 
        )

    [2] => Array
        (
            [name] => trace.ignore_agent_sampling_rates
            [value] => 1
            [origin] => env_var
            [config_id] => 
        )

    [3] => Array
        (
            [name] => trace.generate_root_span
            [value] => 0
            [origin] => env_var
            [config_id] => 
        )

    [4] => Array
        (
            [name] => trace.git_metadata_enabled
            [value] => 0
            [origin] => env_var
            [config_id] => 
        )

    [5] => Array
        (
            [name] => ssi_forced_injection_enabled
            [value] => False
            [origin] => env_var
            [config_id] => 
        )

)
bool(true)
int(0)
--CLEAN--
<?php

@unlink(__DIR__ . '/config-telemetry.out');
