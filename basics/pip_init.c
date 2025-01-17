/*
 * $PIP_license: <Simplified BSD License>
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *
 *     Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 * $
 * $RIKEN_copyright: Riken Center for Computational Sceience (R-CCS),
 * System Software Development Team, 2016-2021
 * $
 * $PIP_TESTSUITE: Version 1.1.0$
 *
 * $Author: Atsushi Hori (R-CCS) mailto: ahori@riken.jp or ahori@me.com
 * $
 * originally written by Noriyuki Soda (SRA)
 */

#include <test.h>

static int test_pip_init( char **argv ) {
#if PIP_VERSION_MAJOR > 1
  int pipid, ntasks;
  void *exp;

  CHECK( pip_is_initialized(), RV, return(EXIT_FAIL) );
  ntasks = NTASKS;
  exp = NULL;
  CHECK( pip_init( &pipid, &ntasks, &exp, 0 ), RV, return(EXIT_FAIL) );
  CHECK( pip_is_initialized(), !RV, return(EXIT_FAIL) );
#endif
  return EXIT_PASS;
}

static int test_pip_init_null( char **argv ) {
  char *env = getenv( PIP_ENV_MODE );
  int pip_mode;

  CHECK( pip_init( NULL, NULL, NULL, 0 ), RV, return(EXIT_FAIL) );

  if( env == NULL ) return EXIT_PASS;

  CHECK( pip_get_mode( &pip_mode ), RV, return(EXIT_FAIL) );
  switch( pip_mode ) {
  case PIP_MODE_PTHREAD:
    CHECK( ( strcasecmp( env, PIP_ENV_MODE_THREAD  ) == 0 ||
	     strcasecmp( env, PIP_ENV_MODE_PTHREAD ) == 0 ), !RV, return(EXIT_FAIL) );
    break;
  case PIP_MODE_PROCESS_PRELOAD:
    CHECK( strcasecmp( env, PIP_ENV_MODE_PROCESS_PRELOAD ),   RV, return(EXIT_FAIL) );
    break;
  case PIP_MODE_PROCESS_PIPCLONE:
    CHECK( strcasecmp( env, PIP_ENV_MODE_PROCESS_PIPCLONE ),  RV, return(EXIT_FAIL) );
    break;
#ifdef PIP_MODE_PROCESS_GOT
  case PIP_MODE_PROCESS_GOT:
    CHECK( strcasecmp( env, PIP_ENV_MODE_PROCESS_GOT ),       RV, return(EXIT_FAIL) );
    break;
#endif
  default:
    return(EXIT_FAIL);
  }
  return EXIT_PASS;
}

static int test_pip_init_preload( char **argv ) {
  char *env = getenv( PIP_ENV_MODE );
  int pipid, ntasks;
  void *exp;

  ntasks = NTASKS;
  exp = NULL;
  if( env == NULL || strcasecmp( env, PIP_ENV_MODE_PROCESS_PRELOAD ) == 0 ) {
    CHECK( pip_init( &pipid, &ntasks, &exp, PIP_MODE_PROCESS_PRELOAD ),
	   RV,
	   return(EXIT_FAIL) );
  } else {
    CHECK( pip_init( &pipid, &ntasks, &exp, PIP_MODE_PROCESS_PRELOAD ),
	   RV!=EPERM,
	   return(EXIT_FAIL) );
  }
  return EXIT_PASS;
}

#ifdef PIP_MODE_PROCESS_GOT
static int test_pip_init_got( char **argv ) {
  char *env = getenv( PIP_ENV_MODE );
  int pipid, ntasks;
  void *exp;

  ntasks = NTASKS;
  exp = NULL;
  if( env != NULL && strcasecmp( env, PIP_ENV_MODE_PROCESS_PRELOAD ) == 0 ) {
    CHECK( pip_init( &pipid, &ntasks, &exp, PIP_MODE_PROCESS_GOT ),
	   RV!=EPERM,
	   return(EXIT_FAIL) );
  } else {
    CHECK( pip_init( &pipid, &ntasks, &exp, PIP_MODE_PROCESS_GOT ),
	   RV,
	   return(EXIT_FAIL) );
  }
  return EXIT_PASS;
}
#endif

static int test_twice( char **argv ) {
  int pipid, ntasks;
  void *exp;

  ntasks = NTASKS;
  exp = NULL;
  CHECK( pip_init( &pipid, &ntasks, &exp, 0 ), RV, return(EXIT_FAIL) );

  ntasks = NTASKS;
  exp = NULL;
  CHECK( pip_init( &pipid, &ntasks, &exp, 0 ), RV!=EBUSY, return(EXIT_FAIL) );
  return EXIT_PASS;
}

static int test_ntask_is_zero( char **argv ) {
  int pipid, ntasks;
  void *exp;

  ntasks = 0;
  exp = NULL;
  CHECK( pip_init( &pipid, &ntasks, &exp, 0 ), RV!=EINVAL, return(EXIT_FAIL) );
  return EXIT_PASS;
}

static int test_ntask_too_big( char **argv ) {
  int pipid, ntasks;
  void *exp;

  ntasks = PIP_NTASKS_MAX + 1;
  exp = NULL;
  CHECK( pip_init( &pipid, &ntasks, &exp, 0 ),
	 RV!=EOVERFLOW,
	 return(EXIT_FAIL) );
  return EXIT_PASS;
}

static int test_invalid_opts( char **argv ) {
  int pipid, ntasks;
  void *exp;

  ntasks = 0;
  exp = NULL;
  CHECK( pip_init( &pipid, &ntasks, &exp, ~PIP_VALID_OPTS ),
	 RV!=EINVAL,
	 return(EXIT_FAIL) );
  return EXIT_PASS;
}

static int test_both_pthread_process( char **argv ) {
  int pipid, ntasks;
  void *exp;

  ntasks = 0;
  exp = NULL;
  CHECK( pip_init( &pipid, &ntasks, &exp,
		   PIP_MODE_PTHREAD | PIP_MODE_PROCESS ),
	 RV!=EINVAL,
	 return(EXIT_FAIL) );
  return EXIT_PASS;
}

static int test_both_preload_clone( char **argv ) {
  int pipid, ntasks;
  void *exp;

  ntasks = 0;
  exp = NULL;
  CHECK( pip_init( &pipid, &ntasks, &exp,
		   PIP_MODE_PROCESS_PRELOAD | PIP_MODE_PROCESS_PIPCLONE ),
	 RV!=EINVAL,
	 return(EXIT_FAIL) );
  return EXIT_PASS;
}

static int test_pip_task_unset( char **argv ) {
  int pipid, ntasks;
  void *exp;

  ntasks = 1;
  exp = NULL;
  CHECK( pip_init( &pipid, &ntasks, &exp, 0 ), RV, return(EXIT_FAIL) );

  if( pipid == PIP_PIPID_ROOT ) {
    pipid = 0;
    CHECK( pip_spawn( argv[0], argv, NULL, PIP_CPUCORE_ASIS, &pipid,
		      NULL, NULL, NULL ),
	   RV,
	   return(EXIT_FAIL) );
    CHECK( pip_wait( 0, NULL ), RV, return(EXIT_FAIL) );
  }
  CHECK( pip_fin(), 	 	RV, return(EXIT_FAIL) );
  return EXIT_PASS;
}

static int test_pip_child_task( char **argv ) {
  int pipid, ntasks = 10;

  CHECK( pip_init( &pipid, &ntasks, NULL, 0 ), RV, return(EXIT_FAIL) );
  CHECK( (pipid==PIP_PIPID_ROOT || (pipid>=0 && pipid<ntasks)), !RV, return(EXIT_FAIL) );
  CHECK( pip_fin(), RV, return(EXIT_FAIL) );
  return EXIT_PASS;
}

int main( int argc, char **argv ) {
  static struct {
    char *name;
    int (*func)( char **argv );
  } tab[] = {
    { "null", 			test_pip_init_null },
    { "preload", 		test_pip_init_preload },
#ifdef PIP_MODE_PROCESS_GOT
    { "got", 			test_pip_init_got },
#endif
    { "twice", 			test_twice },
    { "ntask_is_zero", 		test_ntask_is_zero },
    { "ntask_too_big", 		test_ntask_too_big },
    { "invalid_opts", 		test_invalid_opts },
    { "both_pthread_process", 	test_both_pthread_process },
    { "both_preload_clone", 	test_both_preload_clone },
    { "pip_task_unset", 	test_pip_task_unset },
    { "pip_child_task", 	test_pip_child_task }
  };
  char *test = argv[1];
  int i, ex = EXIT_FAIL;

  switch( argc ) {
  case 1:
    pip_exit( test_pip_init( argv ) );
    break;
  default:
    for( i = 0; i < sizeof( tab ) / sizeof( tab[0] ); i++ ) {
      if( strcmp( tab[i].name, test ) == 0 ) {
	ex = tab[i].func( argv );
	pip_exit( ex );
	fprintf( stderr, "%s: pip_exit(%d) returns\n", test, ex );
	return EXIT_FAIL;
      }
    }
    fprintf( stderr, "%s: unknown test type\n", test );
    break;
  }
  return EXIT_FAIL;
}
