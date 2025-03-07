--TEST--
Sample rate is changed to 0 after first call during a minute when STANDALONE ASM is enabled and no asm events
--SKIPIF--
<?php include __DIR__ . '/../includes/skipif_no_dev_env.inc'; ?>
--ENV--
DD_TRACE_LOG_LEVEL=info,startup=off
DD_AGENT_HOST=request-replayer
DD_TRACE_AGENT_PORT=80
DD_TRACE_AGENT_FLUSH_INTERVAL=333
DD_TRACE_GENERATE_ROOT_SPAN=0
DD_INSTRUMENTATION_TELEMETRY_ENABLED=0
DD_TRACE_SIDECAR_TRACE_SENDER=0
DD_EXPERIMENTAL_APPSEC_STANDALONE_ENABLED=1
--INI--
datadog.trace.agent_test_session_token=background-sender/agent_samplingc
--FILE--
<?php
include __DIR__ . '/../includes/request_replayer.inc';

$rr = new RequestReplayer();

$get_sampling = function() use ($rr) {
    $root = json_decode($rr->waitForDataAndReplay()["body"], true);
    $spans = $root["chunks"][0]["spans"] ?? $root[0];
    return $spans[0]["metrics"]["_sampling_priority_v1"];
};

\DDTrace\start_span();
\DDTrace\close_span();
echo "First call it is used as heartbeat: {$get_sampling()}\n";

dd_trace_internal_fn("synchronous_flush");

\DDTrace\start_span();
\DDTrace\close_span();
echo "Any other call in a minute is set to auto reject: {$get_sampling()}\n";

// reset it for other tests
dd_trace_internal_fn("synchronous_flush");

\DDTrace\start_span();
\DDTrace\close_span();
echo "This one is also set to auto reject: {$get_sampling()}\n";

?>
--EXPECTF--
[ddtrace] [info] Flushing trace of size 1 to send-queue for http://request-replayer:80
First call it is used as heartbeat: 1
[ddtrace] [info] Flushing trace of size 1 to send-queue for http://request-replayer:80
Any other call in a minute is set to auto reject: 0
[ddtrace] [info] Flushing trace of size 1 to send-queue for http://request-replayer:80
This one is also set to auto reject: 0
[ddtrace] [info] No finished traces to be sent to the agent
