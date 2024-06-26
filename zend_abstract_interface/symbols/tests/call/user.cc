extern "C" {
#include "symbols/symbols.h"
}

#include "tea/testing/catch2.hpp"
#include <cstdlib>
#include <cstring>

TEA_TEST_CASE_WITH_STUB("symbol/call/user", "global function", "./stubs/call/user/Stub.php", {
    zval param;
    ZVAL_STRINGL(&param, "string", sizeof("string")-1);

    zval result;

    zai_str fn = ZAI_STRL("\\stub");

    REQUIRE(zai_symbol_call(ZAI_SYMBOL_SCOPE_GLOBAL, NULL, ZAI_SYMBOL_FUNCTION_NAMED, &fn, &result, 1, &param));

    REQUIRE(Z_TYPE(result) == IS_LONG);
    REQUIRE(Z_LVAL(result) == 6);

    zval_ptr_dtor(&param);
    zval_ptr_dtor(&result);
})

TEA_TEST_CASE_WITH_STUB("symbol/call/user", "ns function", "./stubs/call/user/Stub.php", {
    zval param;
    ZVAL_STRINGL(&param, "string", sizeof("string")-1);

    zval result;

    zai_str ns = ZAI_STRL("\\DDTraceTesting");
    zai_str fn = ZAI_STRL("stub");

    REQUIRE(zai_symbol_call(ZAI_SYMBOL_SCOPE_NAMESPACE, &ns, ZAI_SYMBOL_FUNCTION_NAMED, &fn, &result, 1, &param));

    REQUIRE(Z_TYPE(result) == IS_LONG);
    REQUIRE(Z_LVAL(result) == 6);

    zval_ptr_dtor(&param);
    zval_ptr_dtor(&result);
})

TEA_TEST_CASE_WITH_STUB("symbol/call/user", "static public", "./stubs/call/user/Stub.php", {
    zval result;

    zai_str ns = ZAI_STRL("\\DDTraceTesting");
    zai_str cn = ZAI_STRL("Stub");
    zai_str fn = ZAI_STRL("staticPublicFunction");

    zend_class_entry *ce = (zend_class_entry*) zai_symbol_lookup(ZAI_SYMBOL_TYPE_CLASS, ZAI_SYMBOL_SCOPE_NAMESPACE, &ns, &cn);

    REQUIRE(zai_symbol_call(ZAI_SYMBOL_SCOPE_CLASS, ce, ZAI_SYMBOL_FUNCTION_NAMED, &fn, &result, 0));

    REQUIRE(Z_TYPE(result) == IS_LONG);
    REQUIRE(Z_LVAL(result) == 42);

    zval_ptr_dtor(&result);
})

TEA_TEST_CASE_WITH_STUB("symbol/call/user", "static protected", "./stubs/call/user/Stub.php", {
    zval result;

    zai_str ns = ZAI_STRL("\\DDTraceTesting");
    zai_str cn = ZAI_STRL("Stub");
    zai_str fn = ZAI_STRL("staticProtectedFunction");

    zend_class_entry *ce = (zend_class_entry*) zai_symbol_lookup(ZAI_SYMBOL_TYPE_CLASS, ZAI_SYMBOL_SCOPE_NAMESPACE, &ns, &cn);

    REQUIRE(zai_symbol_call(ZAI_SYMBOL_SCOPE_CLASS, ce, ZAI_SYMBOL_FUNCTION_NAMED, &fn, &result, 0));

    REQUIRE(Z_TYPE(result) == IS_LONG);
    REQUIRE(Z_LVAL(result) == 42);

    zval_ptr_dtor(&result);
})

TEA_TEST_CASE_WITH_STUB("symbol/call/user", "static private", "./stubs/call/user/Stub.php", {
    zval result;

    zai_str ns = ZAI_STRL("\\DDTraceTesting");
    zai_str cn = ZAI_STRL("Stub");
    zai_str fn = ZAI_STRL("staticPrivateFunction");

    zend_class_entry *ce = (zend_class_entry*) zai_symbol_lookup(ZAI_SYMBOL_TYPE_CLASS, ZAI_SYMBOL_SCOPE_NAMESPACE, &ns, &cn);

    REQUIRE(zai_symbol_call(ZAI_SYMBOL_SCOPE_CLASS, ce, ZAI_SYMBOL_FUNCTION_NAMED, &fn, &result, 0));

    REQUIRE(Z_TYPE(result) == IS_LONG);
    REQUIRE(Z_LVAL(result) == 42);

    zval_ptr_dtor(&result);
})

TEA_TEST_CASE_WITH_STUB("symbol/call/user", "instance public", "./stubs/call/user/Stub.php", {
    zval result, object;

    zai_str ns = ZAI_STRL("\\DDTraceTesting");
    zai_str cn = ZAI_STRL("Stub");

    zend_class_entry *ce = (zend_class_entry*) zai_symbol_lookup(ZAI_SYMBOL_TYPE_CLASS, ZAI_SYMBOL_SCOPE_NAMESPACE, &ns, &cn);

    zai_symbol_new(&object, ce, 0);

    zai_str fn = ZAI_STRL("publicFunction");

    REQUIRE(zai_symbol_call(ZAI_SYMBOL_SCOPE_OBJECT, &object, ZAI_SYMBOL_FUNCTION_NAMED, &fn, &result, 0));

    REQUIRE(Z_TYPE(result) == IS_LONG);
    REQUIRE(Z_LVAL(result) == 42);

    zval_ptr_dtor(&result);
    zval_ptr_dtor(&object);
})

TEA_TEST_CASE_WITH_STUB("symbol/call/user", "instance protected", "./stubs/call/user/Stub.php", {
    zval result, object;

    zai_str ns = ZAI_STRL("\\DDTraceTesting");
    zai_str cn = ZAI_STRL("Stub");

    zend_class_entry *ce = (zend_class_entry*) zai_symbol_lookup(ZAI_SYMBOL_TYPE_CLASS, ZAI_SYMBOL_SCOPE_NAMESPACE, &ns, &cn);

    zai_symbol_new(&object, ce, 0);

    zai_str fn = ZAI_STRL("protectedFunction");

    REQUIRE(zai_symbol_call(ZAI_SYMBOL_SCOPE_OBJECT, &object, ZAI_SYMBOL_FUNCTION_NAMED, &fn, &result, 0));

    REQUIRE(Z_TYPE(result) == IS_LONG);
    REQUIRE(Z_LVAL(result) == 42);

    zval_ptr_dtor(&result);
    zval_ptr_dtor(&object);
})

TEA_TEST_CASE_WITH_STUB("symbol/call/user", "instance private", "./stubs/call/user/Stub.php", {
    zval result, object;

    zai_str ns = ZAI_STRL("\\DDTraceTesting");
    zai_str cn = ZAI_STRL("Stub");

    zend_class_entry *ce = (zend_class_entry*) zai_symbol_lookup(ZAI_SYMBOL_TYPE_CLASS, ZAI_SYMBOL_SCOPE_NAMESPACE, &ns, &cn);

    zai_symbol_new(&object, ce, 0);

    zai_str fn = ZAI_STRL("privateFunction");

    REQUIRE(zai_symbol_call(ZAI_SYMBOL_SCOPE_OBJECT, &object, ZAI_SYMBOL_FUNCTION_NAMED, &fn, &result, 0));

    REQUIRE(Z_TYPE(result) == IS_LONG);
    REQUIRE(Z_LVAL(result) == 42);

    zval_ptr_dtor(&result);
    zval_ptr_dtor(&object);
})

TEA_TEST_CASE_WITH_STUB("symbol/call/user", "no magic", "./stubs/call/user/Stub.php", {
    zval result, object;

    zai_str ns = ZAI_STRL("\\DDTraceTesting");
    zai_str cn = ZAI_STRL("NoMagicCall");

    zend_class_entry *ce = (zend_class_entry*) zai_symbol_lookup(ZAI_SYMBOL_TYPE_CLASS, ZAI_SYMBOL_SCOPE_NAMESPACE, &ns, &cn);

    zai_symbol_new(&object, ce, 0);

    zai_str fn = ZAI_STRL("nonExistent");

    REQUIRE(!zai_symbol_call(ZAI_SYMBOL_SCOPE_OBJECT, &object, ZAI_SYMBOL_FUNCTION_NAMED, &fn, &result, 0));

    zval_ptr_dtor(&object);
})

TEA_TEST_CASE_WITH_STUB("symbol/call/user", "no abstract", "./stubs/call/user/Stub.php", {
    zval result;

    zai_str ns = ZAI_STRL("\\DDTraceTesting");
    zai_str cn = ZAI_STRL("NoAbstractCall");

    zend_class_entry *ce = (zend_class_entry*) zai_symbol_lookup(ZAI_SYMBOL_TYPE_CLASS, ZAI_SYMBOL_SCOPE_NAMESPACE, &ns, &cn);

    zai_str fn = ZAI_STRL("abstractFunction");

    REQUIRE(!zai_symbol_call(ZAI_SYMBOL_SCOPE_CLASS, ce, ZAI_SYMBOL_FUNCTION_NAMED, &fn, &result, 0));
})

TEA_TEST_CASE_WITH_STUB("symbol/call/user", "static mismatch", "./stubs/call/user/Stub.php", {
    zval result;

    zai_str ns = ZAI_STRL("\\DDTraceTesting");
    zai_str cn = ZAI_STRL("NoStaticMismatch");

    zend_class_entry *ce = (zend_class_entry*) zai_symbol_lookup(ZAI_SYMBOL_TYPE_CLASS, ZAI_SYMBOL_SCOPE_NAMESPACE, &ns, &cn);

    zai_str fn = ZAI_STRL("nonStaticFunction");

    REQUIRE(!zai_symbol_call(ZAI_SYMBOL_SCOPE_CLASS, ce, ZAI_SYMBOL_FUNCTION_NAMED, &fn, &result, 0));
})

TEA_TEST_CASE_WITH_STUB("symbol/call/user", "exception", "./stubs/call/user/Stub.php", {
    zval result;

    zai_str ns = ZAI_STRL("\\DDTraceTesting");
    zai_str cn = ZAI_STRL("NoExceptionLeakage");

    zend_class_entry *ce = (zend_class_entry*) zai_symbol_lookup(ZAI_SYMBOL_TYPE_CLASS, ZAI_SYMBOL_SCOPE_NAMESPACE, &ns, &cn);

    zai_str fn = ZAI_STRL("throwsException");

    REQUIRE(!zai_symbol_call(ZAI_SYMBOL_SCOPE_CLASS, ce, ZAI_SYMBOL_FUNCTION_NAMED, &fn, &result, 0));
})
