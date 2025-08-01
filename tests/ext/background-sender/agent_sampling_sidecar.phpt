--TEST--
The sidecar trace flusher sender informs about changes to the agent sample rate
--SKIPIF--
<?php include __DIR__ . '/../includes/skipif_no_dev_env.inc'; ?>
<?php if (getenv('USE_ZEND_ALLOC') === '0' && !getenv("SKIP_ASAN")) die('skip: valgrind reports sendmsg(msg.msg_control) points to uninitialised byte(s), but it is unproblematic and outside our control in rust code'); ?>
--ENV--
DD_TRACE_LOG_LEVEL=info,startup=off
DD_AGENT_HOST=request-replayer
DD_TRACE_AGENT_PORT=80
DD_TRACE_AGENT_FLUSH_INTERVAL=333
DD_TRACE_GENERATE_ROOT_SPAN=0
DD_INSTRUMENTATION_TELEMETRY_ENABLED=0
DD_TRACE_SIDECAR_TRACE_SENDER=1
DD_TRACE_IGNORE_AGENT_SAMPLING_RATES=0
--INI--
datadog.trace.agent_test_session_token=background-sender/agent_sampling_sidecar
--FILE--
<?php
include __DIR__ . '/../includes/request_replayer.inc';

$contents = [];
$filename = null;

// Race conditions are annoying, especially with parallel test runs
function checkUpdated($marker) {
    if (PHP_OS === "Linux") {
        $retries = 100;
        do {
            foreach (glob("/dev/shm/*") as $f) {
                if (@filesize($f) < 5000) {
                    $file = @file_get_contents($f);
                    if (@strpos($file, $marker) !== false) {
                        global $contents, $filename;
                        $filename = $f;
                        $contents[] = bin2hex($file);
                        return;
                    }
                }
            }
            usleep(100000);
        } while (--$retries);
        foreach (glob("/dev/shm/*") as $f) {
            var_dump($f, bin2hex(file_get_contents($f)));
        }
    }
}

function recordContents() {
    if (PHP_OS === "Linux") {
        global $contents, $filename;
        $contents[] = bin2hex(file_get_contents($filename));
    }
}

$rr = new RequestReplayer();
$rr->replayRequest(); // cleanup possible leftover

$expected = [1,0,1];
$error = false;
$get_sampling = function() use ($rr, &$expected, &$error) {
    $root = json_decode($rr->waitForDataAndReplay()["body"], true);
    $spans = $root["chunks"][0]["spans"] ?? $root[0];
    $priority = $spans[0]["metrics"]["_sampling_priority_v1"];
    if ($priority != array_shift($expected)) {
        $error = true;
    }
    return $priority;
};

$rr->setResponse(["rate_by_service" => ["service:,env:" => 0, "service:agent_sampling_sidecar_test,env:first" => 1]]);

\DDTrace\start_span();
\DDTrace\close_span();

echo "Initial sampling: {$get_sampling()}\n";

checkUpdated("service:agent_sampling_sidecar_test,env:first");

$rr->setResponse(["rate_by_service" => ["service:,env:" => 0, "service:foo,env:none" => 1, "service:agent_sampling_sidecar_test,env:second" => 0]]);

recordContents();
\DDTrace\start_span();
\DDTrace\close_span();
recordContents();

checkUpdated("service:agent_sampling_sidecar_test,env:second");

echo "Generic sampling: {$get_sampling()}\n";

// reset it for other tests
$rr->setResponse(["rate_by_service" => []]);

recordContents();
$s = \DDTrace\start_span();
$s->service = "foo";
$s->env = "none";
\DDTrace\close_span();
recordContents();

echo "Specific sampling: {$get_sampling()}\n";

if ($error && PHP_OS === "Linux") {
    var_dump($contents);
}

?>
--EXPECTF--
[ddtrace] [info] Flushing trace of size 1 to send-queue for http://request-replayer:80
Initial sampling: 1
[ddtrace] [info] Flushing trace of size 1 to send-queue for http://request-replayer:80
Generic sampling: 0
[ddtrace] [info] Flushing trace of size 1 to send-queue for http://request-replayer:80
Specific sampling: 1
[ddtrace] [info] No finished traces to be sent to the agent
