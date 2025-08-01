struct _zend_string;


#ifndef DDTRACE_PHP_H
#define DDTRACE_PHP_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "common.h"
#include "telemetry.h"
#include "sidecar.h"

typedef struct ddog_Vec_CChar *(*ddog_DynamicConfigUpdate)(ddog_CharSlice config,
                                                           ddog_CharSlice value,
                                                           bool return_old);

/**
 * `QueueId` is a struct that represents a unique identifier for a queue.
 * It contains a single field, `inner`, which is a 64-bit unsigned integer.
 */
typedef uint64_t ddog_QueueId;

/**
 * A 128-bit (16 byte) buffer containing the UUID.
 *
 * # ABI
 *
 * The `Bytes` type is always guaranteed to be have the same ABI as [`Uuid`].
 */
typedef uint8_t ddog_Bytes[16];

/**
 * A Universally Unique Identifier (UUID).
 *
 * # Examples
 *
 * Parse a UUID given in the simple format and print it as a urn:
 *
 * ```
 * # use uuid::Uuid;
 * # fn main() -> Result<(), uuid::Error> {
 * let my_uuid = Uuid::parse_str("a1a2a3a4b1b2c1c2d1d2d3d4d5d6d7d8")?;
 *
 * println!("{}", my_uuid.urn());
 * # Ok(())
 * # }
 * ```
 *
 * Create a new random (V4) UUID and print it out in hexadecimal form:
 *
 * ```
 * // Note that this requires the `v4` feature enabled in the uuid crate.
 * # use uuid::Uuid;
 * # fn main() {
 * # #[cfg(feature = "v4")] {
 * let my_uuid = Uuid::new_v4();
 *
 * println!("{}", my_uuid);
 * # }
 * # }
 * ```
 *
 * # Formatting
 *
 * A UUID can be formatted in one of a few ways:
 *
 * * [`simple`](#method.simple): `a1a2a3a4b1b2c1c2d1d2d3d4d5d6d7d8`.
 * * [`hyphenated`](#method.hyphenated):
 *   `a1a2a3a4-b1b2-c1c2-d1d2-d3d4d5d6d7d8`.
 * * [`urn`](#method.urn): `urn:uuid:A1A2A3A4-B1B2-C1C2-D1D2-D3D4D5D6D7D8`.
 * * [`braced`](#method.braced): `{a1a2a3a4-b1b2-c1c2-d1d2-d3d4d5d6d7d8}`.
 *
 * The default representation when formatting a UUID with `Display` is
 * hyphenated:
 *
 * ```
 * # use uuid::Uuid;
 * # fn main() -> Result<(), uuid::Error> {
 * let my_uuid = Uuid::parse_str("a1a2a3a4b1b2c1c2d1d2d3d4d5d6d7d8")?;
 *
 * assert_eq!(
 *     "a1a2a3a4-b1b2-c1c2-d1d2-d3d4d5d6d7d8",
 *     my_uuid.to_string(),
 * );
 * # Ok(())
 * # }
 * ```
 *
 * Other formats can be specified using adapter methods on the UUID:
 *
 * ```
 * # use uuid::Uuid;
 * # fn main() -> Result<(), uuid::Error> {
 * let my_uuid = Uuid::parse_str("a1a2a3a4b1b2c1c2d1d2d3d4d5d6d7d8")?;
 *
 * assert_eq!(
 *     "urn:uuid:a1a2a3a4-b1b2-c1c2-d1d2-d3d4d5d6d7d8",
 *     my_uuid.urn().to_string(),
 * );
 * # Ok(())
 * # }
 * ```
 *
 * # Endianness
 *
 * The specification for UUIDs encodes the integer fields that make up the
 * value in big-endian order. This crate assumes integer inputs are already in
 * the correct order by default, regardless of the endianness of the
 * environment. Most methods that accept integers have a `_le` variant (such as
 * `from_fields_le`) that assumes any integer values will need to have their
 * bytes flipped, regardless of the endianness of the environment.
 *
 * Most users won't need to worry about endianness unless they need to operate
 * on individual fields (such as when converting between Microsoft GUIDs). The
 * important things to remember are:
 *
 * - The endianness is in terms of the fields of the UUID, not the environment.
 * - The endianness is assumed to be big-endian when there's no `_le` suffix
 *   somewhere.
 * - Byte-flipping in `_le` methods applies to each integer.
 * - Endianness roundtrips, so if you create a UUID with `from_fields_le`
 *   you'll get the same values back out with `to_fields_le`.
 *
 * # ABI
 *
 * The `Uuid` type is always guaranteed to be have the same ABI as [`Bytes`].
 */
typedef ddog_Bytes ddog_Uuid;

extern ddog_Uuid ddtrace_runtime_id;

extern void (*ddog_log_callback)(ddog_CharSlice);

extern ddog_VecRemoteConfigProduct DDTRACE_REMOTE_CONFIG_PRODUCTS;

extern ddog_VecRemoteConfigCapabilities DDTRACE_REMOTE_CONFIG_CAPABILITIES;

extern const uint8_t *DDOG_PHP_FUNCTION;

extern struct ddog_SidecarTransport *ddtrace_sidecar;

/**
 * # Safety
 * Must be called from a single-threaded context, such as MINIT.
 */
void ddtrace_generate_runtime_id(void);

void ddtrace_format_runtime_id(uint8_t (*buf)[36]);

ddog_CharSlice ddtrace_get_container_id(void);

void ddtrace_set_container_cgroup_path(ddog_CharSlice path);

char *ddtrace_strip_invalid_utf8(const char *input, uintptr_t *len);

void ddtrace_drop_rust_string(char *input, uintptr_t len);

struct ddog_Endpoint *ddtrace_parse_agent_url(ddog_CharSlice url);

ddog_Configurator *ddog_library_configurator_new_dummy(bool debug_logs, ddog_CharSlice language);

int posix_spawn_file_actions_addchdir_np(void *file_actions, const char *path);

bool ddog_shall_log(enum ddog_Log category);

void ddog_set_error_log_level(bool once);

void ddog_set_log_level(ddog_CharSlice level, bool once);

void ddog_log(enum ddog_Log category, bool once, ddog_CharSlice msg);

void ddog_reset_logger(void);

uint32_t ddog_get_logs_count(ddog_CharSlice level);

void ddog_init_remote_config(bool live_debugging_enabled,
                             bool appsec_activation,
                             bool appsec_config);

struct ddog_RemoteConfigState *ddog_init_remote_config_state(const struct ddog_Endpoint *endpoint);

const char *ddog_remote_config_get_path(const struct ddog_RemoteConfigState *remote_config);

bool ddog_process_remote_configs(struct ddog_RemoteConfigState *remote_config);

bool ddog_type_can_be_instrumented(const struct ddog_RemoteConfigState *remote_config,
                                   ddog_CharSlice typename_);

bool ddog_global_log_probe_limiter_inc(const struct ddog_RemoteConfigState *remote_config);

struct ddog_Vec_CChar *ddog_CharSlice_to_owned(ddog_CharSlice str);

bool ddog_remote_configs_service_env_change(struct ddog_RemoteConfigState *remote_config,
                                            ddog_CharSlice service,
                                            ddog_CharSlice env,
                                            ddog_CharSlice version,
                                            const struct ddog_Vec_Tag *tags);

bool ddog_remote_config_alter_dynamic_config(struct ddog_RemoteConfigState *remote_config,
                                             ddog_CharSlice config,
                                             ddog_CharSlice new_value);

void ddog_setup_remote_config(ddog_DynamicConfigUpdate update_config,
                              const struct ddog_LiveDebuggerSetup *setup);

void ddog_rshutdown_remote_config(struct ddog_RemoteConfigState *remote_config);

void ddog_shutdown_remote_config(struct ddog_RemoteConfigState*);

void ddog_log_debugger_data(const struct ddog_Vec_DebuggerPayload *payloads);

void ddog_log_debugger_datum(const struct ddog_DebuggerPayload *payload);

ddog_MaybeError ddog_send_debugger_diagnostics(const struct ddog_RemoteConfigState *remote_config_state,
                                               struct ddog_SidecarTransport **transport,
                                               const struct ddog_InstanceId *instance_id,
                                               ddog_QueueId queue_id,
                                               const struct ddog_Probe *probe,
                                               uint64_t timestamp);

void ddog_sidecar_enable_appsec(ddog_CharSlice shared_lib_path,
                                ddog_CharSlice socket_file_path,
                                ddog_CharSlice lock_file_path,
                                ddog_CharSlice log_file_path,
                                ddog_CharSlice log_level);

ddog_MaybeError ddog_sidecar_connect_php(struct ddog_SidecarTransport **connection,
                                         const char *error_path,
                                         ddog_CharSlice log_level,
                                         bool enable_telemetry);

void ddtrace_sidecar_reconnect(struct ddog_SidecarTransport **transport,
                               struct ddog_SidecarTransport *(*factory)(void));

bool ddog_shm_limiter_inc(const struct ddog_MaybeShmLimiter *limiter, uint32_t limit);

bool ddog_exception_hash_limiter_inc(struct ddog_SidecarTransport *connection,
                                     uint64_t hash,
                                     uint32_t granularity_seconds);

bool ddtrace_detect_composer_installed_json(struct ddog_SidecarTransport **transport,
                                            const struct ddog_InstanceId *instance_id,
                                            const ddog_QueueId *queue_id,
                                            ddog_CharSlice path);

struct ddog_SidecarActionsBuffer *ddog_sidecar_telemetry_buffer_alloc(void);

void ddog_sidecar_telemetry_buffer_drop(struct ddog_SidecarActionsBuffer*);

void ddog_sidecar_telemetry_addIntegration_buffer(struct ddog_SidecarActionsBuffer *buffer,
                                                  ddog_CharSlice integration_name,
                                                  ddog_CharSlice integration_version,
                                                  bool integration_enabled);

void ddog_sidecar_telemetry_addDependency_buffer(struct ddog_SidecarActionsBuffer *buffer,
                                                 ddog_CharSlice dependency_name,
                                                 ddog_CharSlice dependency_version);

void ddog_sidecar_telemetry_enqueueConfig_buffer(struct ddog_SidecarActionsBuffer *buffer,
                                                 ddog_CharSlice config_key,
                                                 ddog_CharSlice config_value,
                                                 enum ddog_ConfigurationOrigin origin,
                                                 ddog_CharSlice config_id);

ddog_MaybeError ddog_sidecar_telemetry_buffer_flush(struct ddog_SidecarTransport **transport,
                                                    const struct ddog_InstanceId *instance_id,
                                                    const ddog_QueueId *queue_id,
                                                    struct ddog_SidecarActionsBuffer *buffer);

void ddog_sidecar_telemetry_register_metric_buffer(struct ddog_SidecarActionsBuffer *buffer,
                                                   ddog_CharSlice metric_name,
                                                   enum ddog_MetricType metric_type,
                                                   enum ddog_MetricNamespace namespace_);

void ddog_sidecar_telemetry_add_span_metric_point_buffer(struct ddog_SidecarActionsBuffer *buffer,
                                                         ddog_CharSlice metric_name,
                                                         double metric_value,
                                                         ddog_CharSlice tags);

void ddog_sidecar_telemetry_add_integration_log_buffer(enum ddog_Log category,
                                                       struct ddog_SidecarActionsBuffer *buffer,
                                                       ddog_CharSlice log);

ddog_ShmCacheMap *ddog_sidecar_telemetry_cache_new(void);

void ddog_sidecar_telemetry_cache_drop(ddog_ShmCacheMap*);

bool ddog_sidecar_telemetry_config_sent(const ddog_ShmCacheMap *cache,
                                        ddog_CharSlice service,
                                        ddog_CharSlice env);

ddog_MaybeError ddog_sidecar_telemetry_filter_flush(struct ddog_SidecarTransport **transport,
                                                    const struct ddog_InstanceId *instance_id,
                                                    const ddog_QueueId *queue_id,
                                                    struct ddog_SidecarActionsBuffer *buffer,
                                                    ddog_ShmCacheMap *cache,
                                                    ddog_CharSlice service,
                                                    ddog_CharSlice env);

void ddog_init_span_func(void (*free_func)(struct _zend_string*),
                         void (*addref_func)(struct _zend_string*));

void ddog_set_span_service_zstr(ddog_SpanBytes *ptr, struct _zend_string *str);

void ddog_set_span_name_zstr(ddog_SpanBytes *ptr, struct _zend_string *str);

void ddog_set_span_resource_zstr(ddog_SpanBytes *ptr, struct _zend_string *str);

void ddog_set_span_type_zstr(ddog_SpanBytes *ptr, struct _zend_string *str);

void ddog_add_span_meta_zstr(ddog_SpanBytes *ptr,
                             struct _zend_string *key,
                             struct _zend_string *val);

void ddog_add_CharSlice_span_meta_zstr(ddog_SpanBytes *ptr,
                                       ddog_CharSlice key,
                                       struct _zend_string *val);

void ddog_add_zstr_span_meta_str(ddog_SpanBytes *ptr, struct _zend_string *key, const char *val);

void ddog_add_str_span_meta_str(ddog_SpanBytes *ptr, const char *key, const char *val);

void ddog_add_str_span_meta_zstr(ddog_SpanBytes *ptr, const char *key, struct _zend_string *val);

void ddog_add_str_span_meta_CharSlice(ddog_SpanBytes *ptr, const char *key, ddog_CharSlice val);

void ddog_del_span_meta_zstr(ddog_SpanBytes *ptr, struct _zend_string *key);

void ddog_del_span_meta_str(ddog_SpanBytes *ptr, const char *key);

bool ddog_has_span_meta_zstr(ddog_SpanBytes *ptr, struct _zend_string *key);

bool ddog_has_span_meta_str(ddog_SpanBytes *ptr, const char *key);

ddog_CharSlice ddog_get_span_meta_str(ddog_SpanBytes *span, const char *key);

void ddog_add_span_metrics_zstr(ddog_SpanBytes *ptr, struct _zend_string *key, double val);

bool ddog_has_span_metrics_zstr(ddog_SpanBytes *ptr, struct _zend_string *key);

void ddog_del_span_metrics_zstr(ddog_SpanBytes *ptr, struct _zend_string *key);

void ddog_add_span_metrics_str(ddog_SpanBytes *ptr, const char *key, double val);

bool ddog_get_span_metrics_str(ddog_SpanBytes *ptr, const char *key, double *result);

void ddog_del_span_metrics_str(ddog_SpanBytes *ptr, const char *key);

void ddog_add_span_meta_struct_zstr(ddog_SpanBytes *ptr,
                                    struct _zend_string *key,
                                    struct _zend_string *val);

void ddog_add_zstr_span_meta_struct_CharSlice(ddog_SpanBytes *ptr,
                                              struct _zend_string *key,
                                              ddog_CharSlice val);

#endif  /* DDTRACE_PHP_H */
