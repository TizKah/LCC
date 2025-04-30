#ifndef __SEM_CV_H__
#define __SEM_CV_H__

typedef struct __cond_var_t *cond_var_t;
typedef struct __thread_t *thread_t;

cond_var_t cond_var_init();
void cond_var_destroy(cond_var_t cv);
void cv_wait(cond_var_t cond_var);
void cv_signal(cond_var_t cond_var);
void cv_broadcast(cond_var_t cond_var);

#endif