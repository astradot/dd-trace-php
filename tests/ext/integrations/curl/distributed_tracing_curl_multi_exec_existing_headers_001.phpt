--TEST--
Distributed tracing headers propagate with curl_multi_exec() and headers set with curl_setopt()
--SKIPIF--
<?php if (!extension_loaded('curl')) die('skip: curl extension required'); ?>
<?php if (!getenv('HTTPBIN_HOSTNAME')) die('skip: HTTPBIN_HOSTNAME env var required'); ?>
--ENV--
DD_TRACE_AUTO_FLUSH_ENABLED=0
DD_TRACE_LOG_LEVEL=info,startup=off
HTTP_X_DATADOG_ORIGIN=phpt-test
--FILE--
<?php
include 'curl_helper.inc';
include 'distributed_tracing.inc';

DDTrace\trace_function('doMulti', function (\DDTrace\SpanData $span) {
    $span->name = 'doMulti';
});

function dumpHeaders($ch)
{
    $response = curl_multi_getcontent($ch);
    $headers = dt_decode_headers_from_httpbin($response);
    dt_dump_headers_from_httpbin($headers, [
        'x-ch-1-foo',
        'x-ch-1-bar',
        'x-ch-2-foo',
        'x-ch-2-bar',
        'x-datadog-parent-id',
        'x-datadog-origin',
    ]);
}

function doMulti($url)
{
    $ch1 = curl_init_no_dns_cache();
    curl_setopt($ch1, CURLOPT_URL, $url);
    curl_setopt($ch1, CURLOPT_RETURNTRANSFER, true);
    curl_setopt($ch1, CURLOPT_HTTPHEADER, [
        'x-ch-1-foo: foo',
        'x-ch-1-bar: bar',
    ]);

    $ch2 = curl_init_no_dns_cache();
    curl_setopt($ch2, CURLOPT_URL, $url);
    curl_setopt($ch2, CURLOPT_RETURNTRANSFER, true);
    curl_setopt($ch2, CURLOPT_HTTPHEADER, [
        'x-ch-2-foo: foo',
        'x-ch-2-bar: bar',
    ]);

    $mh = curl_multi_init();
    curl_multi_add_handle($mh, $ch1);
    curl_multi_add_handle($mh, $ch2);

    do {
        $status = curl_multi_exec($mh, $active);
        curl_multi_select($mh);
    } while ($active > 0 && $status === CURLM_OK);

    show_curl_multi_error_on_fail($status);
    show_curl_error_on_fail($ch1);
    show_curl_error_on_fail($ch2);

    dumpHeaders($ch1);
    dumpHeaders($ch2);

    curl_multi_remove_handle($mh, $ch1);
    curl_multi_remove_handle($mh, $ch2);

    curl_multi_close($mh);
}

$port = getenv('HTTPBIN_PORT') ?: '80';
$url = 'http://' . getenv('HTTPBIN_HOSTNAME') . ':' . $port .'/headers';

doMulti($url);

echo 'Done.' . PHP_EOL;

?>
--EXPECTF--
x-ch-1-bar: bar
x-ch-1-foo: foo
x-datadog-origin: phpt-test
x-datadog-parent-id: %d
x-ch-2-bar: bar
x-ch-2-foo: foo
x-datadog-origin: phpt-test
x-datadog-parent-id: %d
Done.
[ddtrace] [info] Flushing trace of size 2 to send-queue for %s
