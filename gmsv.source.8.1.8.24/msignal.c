#include "version.h"
#include <signal.h>
#include <errno.h>
#include <unistd.h>

#include <stdio.h>

#include "buf.h"
#include "net.h"
#include "char.h"
#include "item.h"
#include "object.h"
#include "configfile.h"
#include "lssproto_serv.h"
#include "saacproto_cli.h"
#include "log.h"
#include "petmail.h"

/*------------------------------------------------------------
 * lsprotocol のワーキング    を解  する関数を呼ぶ�e
 * 引数�b返り値
 *  なし
 ------------------------------------------------------------*/
static void endlsprotocol( void )
{
    lssproto_CleanupServer();
    saacproto_CleanupClient();
}

/*------------------------------------------------------------
 * すべての保存しなければならないデータをダンプする関数
 * それぞれのモジュールの関数を呼ぶのみ�e
 * 引数�b返り値
 *  なし
 ------------------------------------------------------------*/
static void allDataDump( void )
{
    closeAllLogFile();
#ifndef _SIMPLIFY_ITEMSTRING
	storeObjects( getStoredir() );
	storePetmail();
#endif
	storeCharaData();
}

/*------------------------------------------------------------
 * プひグラムの終  処  のために呼び出される�e
 * 引数�b返り値
 *  なし
 ------------------------------------------------------------*/
void shutdownProgram( void )
{
    close( acfd );
    close( bindedfd );
    endlsprotocol();
    endConnect();
    memEnd();
}

char *DebugFunctionName = NULL;
//char *DebugFunctionName_CHAR = NULL;
//char *DebugFunctionName_ITEM = NULL;
int DebugPoint = 0;

void sigshutdown( int number )
{  
    print( "Received signal : %d\n" , number  );
    if( number == 0 )print( "\ngmsv normal down\n" );
    print( "\nDebugPoint (%d)\n", DebugPoint );
    print( "\nLastFunc (%s)\n", DebugFunctionName );

	remove( "gmsvlog.err2");
	rename( "gmsvlog.err1", "gmsvlog.err2" );
	rename( "gmsvlog.err", "gmsvlog.err1" );
	rename( "gmsvlog", "gmsvlog.err" );
#if USE_MTIO
    {
        void MTIO_join(void);
        MTIO_join();
    }
#endif
    allDataDump();
    

    signal( SIGINT , SIG_IGN );
    signal( SIGQUIT, SIG_IGN );
    signal( SIGKILL, SIG_IGN );
    signal( SIGSEGV, SIG_IGN );
    signal( SIGPIPE, SIG_IGN );
    signal( SIGTERM, SIG_IGN );
		signal( SIGALRM, SIG_IGN );

    shutdownProgram();

    exit(number);
}

extern jztimeout;

void jztime_out( int number )
{
	print("�W�評��逎柚�");
	jztimeout=TRUE;
}
void signalset( void )
{
    // CoolFish: Test Signal 2001/10/26
    print("\nCoolFish Get Signal..\n");

	print("SIGINT:%d\n", SIGINT);
	print("SIGQUIT:%d\n", SIGQUIT);
	print("SIGKILL:%d\n", SIGKILL);
	print("SIGSEGV:%d\n", SIGSEGV);
	print("SIGPIPE:%d\n", SIGPIPE);
	print("SIGTERM:%d\n", SIGTERM);
  print("SIGALRM:%d\n", SIGALRM);

    signal( SIGINT , sigshutdown );  //苛際い�_嘉
    signal( SIGQUIT, sigshutdown );	 //苛際�h�X嘉
    signal( SIGKILL, sigshutdown );	 //苛ゎ
    signal( SIGSEGV, sigshutdown );	 //�L�沈x�s�X維
    signal( SIGPIPE, SIG_IGN );			 //�g�楜L的�i�{�査濤D
    signal( SIGTERM, sigshutdown );	 //苛ゎ
    signal( SIGALRM, jztime_out  );	 //�W��
}
