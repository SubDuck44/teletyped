#define ArrayN(t, n)                                                           \
	typedef struct {                                                           \
		t*     ptr;                                                            \
		size_t len;                                                            \
		size_t cap;                                                            \
	} n

#define Array(t) ArrayN(t, t##s)

#define ArrayExtend(arr)                                                       \
	((arr).len >= (arr).cap                                                    \
	     ? (arr).ptr = reallocarray(                                           \
			   (arr).ptr, ((arr).cap = max((arr).cap << 1, 16)),               \
			   sizeof(*(arr).ptr)                                              \
		   )                                                                   \
	     : 0)

#define ArrayAdd(arr, x) (ArrayExtend(arr), (arr).ptr[(arr).len++] = (x))

#define ArrayNew(arr)                                                          \
	(ArrayExtend(arr), memset(&(arr).ptr[(arr).len++], 0, sizeof(*(arr).ptr)))

#define ArrayLoop(arr, body) ArrayLoopN(arr, it, body)

#define ArrayLoopN(arr, it, body)                                              \
	for(size_t i = 0; i < (arr).len; i++) {                                    \
		typeof((arr).ptr[0])* it = &(arr).ptr[i];                              \
		body                                                                   \
	}

#define ArrayFindI(arr, result, pred)                                          \
	ArrayLoopN(arr, it, {                                                      \
		if(pred) {                                                             \
			result = it;                                                       \
			break;                                                             \
		}                                                                      \
	})

#define ArrayFind(arr, result, pred)                                           \
	typeof((arr).ptr[0])* result = NULL;                                       \
	ArrayFindI(arr, result, pred)

#define ArrayLast(arr) (arr).ptr[(arr).len - 1]

#define ArrayFree(arr)                                                         \
	do {                                                                       \
		free((arr).ptr);                                                       \
		(arr).ptr = NULL;                                                      \
		(arr).len = (arr).cap = 0;                                             \
	} while(0)
