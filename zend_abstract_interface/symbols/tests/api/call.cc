extern "C" {
#include "symbols/symbols.h"
}

#include "tea/testing/catch2.hpp"

TEA_TEST_CASE("symbol/api/call", "literal", {
    zval result;

    CHECK(zai_symbol_call_global(ZAI_STRL("phpversion"), &result, 0));

    zval_ptr_dtor(&result);
})

TEA_TEST_CASE_WITH_STUB("symbol/api/call", "literal ns", "./stubs/call/user/Stub.php", {
    zval result;

    CHECK(zai_symbol_call_ns(ZAI_STRL("DDTraceTesting"), ZAI_STRL("noargs"), &result, 0));

    zval_ptr_dtor(&result);
})

TEA_TEST_CASE_WITH_STUB("symbol/api/call", "static", "./stubs/call/user/Stub.php", {
    zval result;
    zend_class_entry *ce = zai_symbol_lookup_class_ns(
        ZAI_STRL("DDTraceTesting"), ZAI_STRL("Stub"));

    CHECK(zai_symbol_call_static(ce, ZAI_STRL("staticPublicFunction"), &result, 0));

    zval_ptr_dtor(&result);
})

TEA_TEST_CASE_WITH_STUB("symbol/api/call", "static literal", "./stubs/call/user/Stub.php", {
    zval result;
    zend_class_entry *ce = zai_symbol_lookup_class_ns(
        ZAI_STRL("DDTraceTesting"), ZAI_STRL("Stub"));

    CHECK(zai_symbol_call_static(ce, ZAI_STRL("staticPublicFunction"), &result, 0));

    zval_ptr_dtor(&result);
})
