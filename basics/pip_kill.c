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
 */

#include <test.h>

static struct my_exp	*expp;
static int 		ntasks, pipid;

static struct my_exp {
  pthread_barrier_t	barr;
} exp;

static int block_sigusrs( void ) {
  sigset_t 	ss;
  errno = 0;
  if( sigemptyset( &ss )        != 0 ) exit(EXIT_FAIL);
  if( sigaddset( &ss, SIGUSR1 ) != 0 ) exit(EXIT_FAIL);
  if( sigaddset( &ss, SIGUSR2 ) != 0 ) exit(EXIT_FAIL);
  return pip_sigmask( SIG_BLOCK, &ss, NULL );
}

static int wait_signal_root( int signo ) {
  sigset_t 	ss;
  int		sig;

  if( sigemptyset( &ss )        != 0 ) exit(EXIT_FAIL);
  if( sigaddset( &ss, SIGUSR1 ) != 0 ) exit(EXIT_FAIL);
  if( sigaddset( &ss, SIGUSR2 ) != 0 ) exit(EXIT_FAIL);
  do {
    (void) sigwait( &ss, &sig );
  } while( sig != signo );
  return 0;
}

static int wait_signal_task( int pipid ) {
  sigset_t 	ss;
  int 		sig, next;

  next = pipid + 1;
  next = ( next >= ntasks ) ? next = PIP_PIPID_ROOT : next;

  if( sigemptyset( &ss )        != 0 ) exit(EXIT_FAIL);
  if( sigaddset( &ss, SIGUSR1 ) != 0 ) exit(EXIT_FAIL);
  if( sigaddset( &ss, SIGUSR2 ) != 0 ) exit(EXIT_FAIL);
  (void) sigwait( &ss, &sig );
  if( next >= ntasks ) next = PIP_PIPID_ROOT;
  CHECK( pip_kill( next, sig ), RV, exit(EXIT_FAIL) );
  return sig;
}

int main( int argc, char **argv ) {
  char	*env;
  int	ntenv;
  int	i, extval = 0;

  ntasks = 0;
  if( argc > 1 ) {
    ntasks = strtol( argv[1], NULL, 10 );
  }
  ntasks = ( ntasks <= 0 ) ? NTASKS : ntasks;
  if( ( env = getenv( "NTASKS" ) ) != NULL ) {
    ntenv = strtol( env, NULL, 10 );
    if( ntasks > ntenv )  return(EXIT_UNTESTED);
  } else {
    if( ntasks > NTASKS ) return(EXIT_UNTESTED);
  }

  expp = &exp;
  CHECK( pip_init(&pipid,&ntasks,(void**)&expp,0), RV, return(EXIT_FAIL) );
  if( pipid == PIP_PIPID_ROOT ) {
    memset( &exp, 0, sizeof(exp) );
    CHECK( pthread_barrier_init(&exp.barr,NULL,ntasks+1),
	   RV, return(EXIT_FAIL) );

    for( i=0; i<ntasks; i++ ) {
      pipid = i;
      CHECK( pip_spawn(argv[0],argv,NULL,PIP_CPUCORE_ASIS,&pipid,
		       NULL,NULL,NULL),
	     RV, return(EXIT_FAIL) );
    }
    CHECK( block_sigusrs(), RV,    return(EXIT_FAIL) );
    CHECK( pthread_barrier_wait( &expp->barr ),
	   (RV!=0&&RV!=PTHREAD_BARRIER_SERIAL_THREAD), return(EXIT_FAIL) );

    CHECK( pip_kill( 0, SIGUSR1 ),      RV,            return(EXIT_FAIL) );
    CHECK( wait_signal_root( SIGUSR1),  RV,            return(EXIT_FAIL) );

    CHECK( pip_kill( 0, SIGUSR2 ),      RV,            return(EXIT_FAIL) );
    CHECK( wait_signal_root( SIGUSR2 ), RV,            return(EXIT_FAIL) );

    CHECK( pthread_barrier_wait( &expp->barr ),
	   (RV!=0&&RV!=PTHREAD_BARRIER_SERIAL_THREAD), return(EXIT_FAIL) );

    for( i=0; i<ntasks; i++ ) {
      int status;
      CHECK( pip_wait_any( NULL, &status ),    RV, return(EXIT_FAIL) );
      if( WIFEXITED( status ) ) {
	CHECK( ( extval = WEXITSTATUS( status ) ),
	       RV,
	       return(EXIT_FAIL) );
      } else {
	CHECK( "Task is signaled",             RV,    return(EXIT_UNRESOLVED) );
      }
    }
    CHECK( pip_kill( ntasks, SIGUSR1 ),        RV!=ERANGE, return(EXIT_FAIL) );

  } else {
    CHECK( block_sigusrs(),                    RV,     return(EXIT_FAIL) );

    CHECK( pthread_barrier_wait( &expp->barr ),
	   (RV!=0&&RV!=PTHREAD_BARRIER_SERIAL_THREAD), return(EXIT_FAIL) );

    CHECK( wait_signal_task(pipid),  RV!=SIGUSR1, return(EXIT_FAIL) );
    CHECK( wait_signal_task(pipid),  RV!=SIGUSR2, return(EXIT_FAIL) );

    CHECK( pthread_barrier_wait( &expp->barr ),
	   (RV!=0&&RV!=PTHREAD_BARRIER_SERIAL_THREAD), return(EXIT_FAIL) );
  }
  CHECK( pip_fin(), RV, return(EXIT_FAIL) );
  return extval;
}
