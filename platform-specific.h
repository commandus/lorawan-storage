#if BYTE_ORDER == BIG_ENDIAN
#endif

#ifdef _MSC_VER
#define EXPORT_SHARED_C_FUNC extern "C" __declspec(dllexport)
#else
#define EXPORT_SHARED_C_FUNC extern "C"
#endif
