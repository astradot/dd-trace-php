<?php

namespace DDTrace\Tests\Integrations\PCNTL;

use DDTrace\Tests\Common\SpanAssertion;
use DDTrace\Tests\Common\IntegrationTestCase;

final class PCNTLTest extends IntegrationTestCase
{
    public function testAAA()
    {
        $traces = $this->getTracesFromCommand();

        $this->assertFlameGraph($traces, [
            SpanAssertion::build(
                'my_app',
                'foo_service',
                'custom',
                'foo_resource'
            )->withExactTags([
                'foo' => 'bar',
            ])->withChildren([
                SpanAssertion::exists(
                    'mysqli_connect',
                    'mysqli_connect'
                ),
                SpanAssertion::exists(
                    'curl_exec',
                    'http://httpbin_integration/status/?'
                ),
            ])
        ]);
    }
}
