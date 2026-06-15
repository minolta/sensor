#ifndef UNITY_CONFIG_H
#define UNITY_CONFIG_H

// Unity library is compiled without Arduino headers; keep output as no-ops here.
// Test code may still use Serial directly for diagnostics.
#define UNITY_OUTPUT_CHAR(a) ((void)(a))
#define UNITY_OUTPUT_FLUSH() ((void)0)
#define UNITY_OUTPUT_START() ((void)0)
#define UNITY_OUTPUT_COMPLETE() ((void)0)

#endif
