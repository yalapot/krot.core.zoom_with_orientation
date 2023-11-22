/*
Файл определения компиляторо-зависимых макросов и обьявлений
*/
#ifdef _MSC_VER

 #pragma warning(disable : 4996) // for VC 8.00


 #define EXPORT __declspec(dllexport)
 #define MAXDRIVE _MAX_DRIVE
 #define MAXDIR _MAX_DIR
 #define MAXEXT _MAX_EXT
 #define MAXFILE _MAX_FNAME
 #define MAXPATH _MAX_PATH
 #define INLINE __inline
 #define fnmerge _makepath
 #define fnsplit _splitpath
#endif /* ifdef _MSC_VER */

#ifdef __BORLANDC__
 #define EXPORT _export
 #define FARPROC _export _stdcall
 #define DllMain DllEntryPoint
 #define INLINE
#endif /* __BORLANDC__ */
