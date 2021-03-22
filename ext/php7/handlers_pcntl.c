#include <php.h>
#include <stdbool.h>

#include "coms.h"
#include "configuration.h"
#include "engine_api.h"
#include "engine_hooks.h"  // for ddtrace_backup_error_handling
#include "handlers_internal.h"
#include "logging.h"
#include "span.h"

// True global - only modify during MINIT/MSHUTDOWN
bool dd_ext_pcntl_loaded = false;

static void (*dd_pcntl_fork_handler)(INTERNAL_FUNCTION_PARAMETERS) = NULL;

ZEND_FUNCTION(ddtrace_pcntl_fork) {
    // Since PID has changed, we need to reset PID based communication on the background sender
    pid_t caller_pid_before_fork = getpid();
    dd_pcntl_fork_handler(INTERNAL_FUNCTION_PARAM_PASSTHRU);
    pid_t caller_pid_after_fork = getpid();
    if (caller_pid_after_fork != caller_pid_before_fork) {
        // we are in forked process
        ddtrace_bgs_logf("pcntl_fork after wrapper", "");
        ddtrace_coms_on_pid_change();
        ddtrace_init_internal_distributed_trace(TSRMLS_C);
    }
}

struct dd_pcntl_handler {
    const char *name;
    size_t name_len;
    void (**old_handler)(INTERNAL_FUNCTION_PARAMETERS);
    void (*new_handler)(INTERNAL_FUNCTION_PARAMETERS);
};
typedef struct dd_pcntl_handler dd_pcntl_handler;

static void dd_install_handler(dd_pcntl_handler handler) {
    zend_function *old_handler;
    old_handler = zend_hash_str_find_ptr(CG(function_table), handler.name, handler.name_len);
    if (old_handler != NULL) {
        *handler.old_handler = old_handler->internal_function.handler;
        old_handler->internal_function.handler = handler.new_handler;
    }
}

/* This function is called during process startup so all of the memory allocations should be
 * persistent to avoid using the Zend Memory Manager. This will avoid an accidental use after free.
 *
 * "If you use ZendMM out of the scope of a request (like in MINIT()), the allocation will be
 * silently cleared by ZendMM before treating the first request, and you'll probably use-after-free:
 * simply don't."
 *
 * @see http://www.phpinternalsbook.com/php7/memory_management/zend_memory_manager.html#common-errors-and-mistakes
 */
void ddtrace_pcntl_handlers_startup(void) {
    // if we cannot find ext/pcntl then do not instrument it
    zend_string *pcntl = zend_string_init(ZEND_STRL("pcntl"), 1);
    dd_ext_pcntl_loaded = zend_hash_exists(&module_registry, pcntl);
    zend_string_release(pcntl);
    if (!dd_ext_pcntl_loaded) {
        return;
    }

    /* We hook into pcntl_exec twice:
     *   - One that handles general dispatch so it will call the associated closure with pcntl_exec
     *   - One that handles the distributed tracing headers
     * The latter expects the former is already done because it needs a span id for the distributed tracing headers;
     * register them inside-out.
     */
    dd_pcntl_handler handlers[] = {
        {ZEND_STRL("pcntl_fork"), &dd_pcntl_fork_handler, ZEND_FN(ddtrace_pcntl_fork)},
    };
    size_t handlers_len = sizeof handlers / sizeof handlers[0];
    for (size_t i = 0; i < handlers_len; ++i) {
        dd_install_handler(handlers[i]);
    }

    if (ddtrace_resource != -1) {
        ddtrace_string pcntl_fork = DDTRACE_STRING_LITERAL("pcntl_fork");
        ddtrace_replace_internal_function(CG(function_table), pcntl_fork);
    }
}
