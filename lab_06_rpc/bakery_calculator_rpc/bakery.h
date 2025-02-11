/*
 * Please do not edit this file.
 * It was generated using rpcgen.
 */

#ifndef _BAKERY_H_RPCGEN
#define _BAKERY_H_RPCGEN

#include <rpc/rpc.h>


#ifdef __cplusplus
extern "C" {
#endif


enum OPERATION {
	PLUS = 0,
	MINUS = 1,
	MULTIPLICATION = 2,
	DIVISION = 3,
};
typedef enum OPERATION OPERATION;

struct REQUEST {
	int index;
	int pid;
	float arg1;
	enum OPERATION operation;
	float arg2;
	float res;
	int number;
};
typedef struct REQUEST REQUEST;

#define BAKERY_PROG 0x20000001
#define BAKERY_VER 1

#if defined(__STDC__) || defined(__cplusplus)
#define GET_NUMBER 1
extern  int * get_number_1(struct REQUEST *, CLIENT *);
extern  int * get_number_1_svc(struct REQUEST *, struct svc_req *);
#define BAKERY_SERVICE 2
extern  struct REQUEST * bakery_service_1(struct REQUEST *, CLIENT *);
extern  struct REQUEST * bakery_service_1_svc(struct REQUEST *, struct svc_req *);
extern int bakery_prog_1_freeresult (SVCXPRT *, xdrproc_t, caddr_t);

#else /* K&R C */
#define GET_NUMBER 1
extern  int * get_number_1();
extern  int * get_number_1_svc();
#define BAKERY_SERVICE 2
extern  struct REQUEST * bakery_service_1();
extern  struct REQUEST * bakery_service_1_svc();
extern int bakery_prog_1_freeresult ();
#endif /* K&R C */

/* the xdr functions */

#if defined(__STDC__) || defined(__cplusplus)
extern  bool_t xdr_OPERATION (XDR *, OPERATION*);
extern  bool_t xdr_REQUEST (XDR *, REQUEST*);

#else /* K&R C */
extern bool_t xdr_OPERATION ();
extern bool_t xdr_REQUEST ();

#endif /* K&R C */

#ifdef __cplusplus
}
#endif

#endif /* !_BAKERY_H_RPCGEN */
