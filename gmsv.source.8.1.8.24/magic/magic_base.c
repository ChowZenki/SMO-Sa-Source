#include "version.h"
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "buf.h"
#include "configfile.h"
#include "magic_base.h"
#include "magic.h"

static Magic	*MAGIC_magic;
static int		MAGIC_magicnum;



#ifdef __ATTACK_MAGIC

AttMagic	*ATTMAGIC_magic;
int		 ATTMAGIC_magicnum;

#endif



typedef struct tagMagic_MagicFunctionTable
{
	char			*functionname;		/*	呪術設定ファイルに書く関数の  前 */
	MAGIC_CALLFUNC	func;				/*    際に呼び出される関数 */
	int				hash;				/*  hash */
}MAGIC_MagicFunctionTable;

/* 呪術を  やしたらここに登  する事 */
static MAGIC_MagicFunctionTable MAGIC_functbl[] = {
	{ "MAGIC_Recovery", 		MAGIC_Recovery,			0},
	{ "MAGIC_OtherRecovery",	MAGIC_OtherRecovery,	0},
	{ "MAGIC_FieldAttChange",	MAGIC_FieldAttChange,	0},
	{ "MAGIC_StatusChange",		MAGIC_StatusChange,		0},
	{ "MAGIC_MagicDef",			MAGIC_MagicDef,			0},
	{ "MAGIC_StatusRecovery",	MAGIC_StatusRecovery,	0},
	{ "MAGIC_Ressurect",		MAGIC_Ressurect,	0},
	{ "MAGIC_AttReverse",		MAGIC_AttReverse,	0},
	{ "MAGIC_ResAndDef",		MAGIC_ResAndDef,	0},
	
#ifdef __ATTACK_MAGIC
	{ "MAGIC_AttMagic" , 		MAGIC_AttMagic , 0 },
#endif
#ifdef _OTHER_MAGICSTAUTS
	{ "MAGIC_MagicStatusChange",	MAGIC_MagicStatusChange,	0},
#endif
#ifdef _ITEM_METAMO
	{ "MAGIC_Metamo", 		MAGIC_Metamo,	0},
#endif
#ifdef _ITEM_ATTSKILLMAGIC
	//{ "MAGIC_AttSkill", 		MAGIC_AttSkill,	0},
#endif
#ifdef _MAGIC_WEAKEN       // vincent  埼�F:糾�z
	{ "MAGIC_Weaken", 		  MAGIC_Weaken,	      0},
#endif
#ifdef _MAGIC_DEEPPOISON   // vincent  埼�F:�@�r
	{ "MAGIC_StatusChange2",  MAGIC_StatusChange2,0},
#endif
#ifdef _MAGIC_BARRIER      // vincent  埼�F:�]支
	{ "MAGIC_Barrier", 		  MAGIC_Barrier,	  0},
#endif
#ifdef _MAGIC_NOCAST       // vincent  埼�F:�I�q
	{ "MAGIC_Nocast", 		  MAGIC_Nocast,	      0},
#endif
#ifdef _MAGIC_TOCALL	// �b�s�}
	{ "MAGIC_ToCallDragon",	MAGIC_ToCallDragon,		0},
#endif
};

/*----------------------------------------------------------------------*/


/* 基  チェック�fアクセス関係 */
/*----------------------------------------------------------------------*/
INLINE BOOL MAGIC_CHECKINDEX( int index )
{
    if( MAGIC_magicnum<=index || index<0 )return FALSE;
    return TRUE;
}
/*----------------------------------------------------------------------*/
static INLINE BOOL MAGIC_CHECKINTDATAINDEX( int index)
{
	if( MAGIC_DATAINTNUM <= index || index < 0 ) return FALSE;
	return TRUE;
}
/*----------------------------------------------------------------------*/
static INLINE BOOL MAGIC_CHECKCHARDATAINDEX( int index)
{
	if( MAGIC_DATACHARNUM <= index || index < 0 ) return FALSE;
	return TRUE;
}
/*----------------------------------------------------------------------*/
INLINE int MAGIC_getInt( int index, MAGIC_DATAINT element)
{
	return MAGIC_magic[index].data[element];
}
/*----------------------------------------------------------------------*/
INLINE int MAGIC_setInt( int index, MAGIC_DATAINT element, int data)
{
	int buf;
	buf = MAGIC_magic[index].data[element];
	MAGIC_magic[index].data[element]=data;
	return buf;
}
/*----------------------------------------------------------------------*/
INLINE char* MAGIC_getChar( int index, MAGIC_DATACHAR element)
{
	if( !MAGIC_CHECKINDEX( index)) return NULL;
	if( !MAGIC_CHECKCHARDATAINDEX( element)) return NULL;
	return MAGIC_magic[index].string[element].string;
}

/*----------------------------------------------------------------------*/
INLINE BOOL MAGIC_setChar( int index ,MAGIC_DATACHAR element, char* new )
{
    if(!MAGIC_CHECKINDEX(index))return FALSE;
    if(!MAGIC_CHECKCHARDATAINDEX(element))return FALSE;
    strcpysafe( MAGIC_magic[index].string[element].string,
                sizeof(MAGIC_magic[index].string[element].string),
                new );
    return TRUE;
}
/*----------------------------------------------------------------------
 *   法の数を知る�e
 *---------------------------------------------------------------------*/
int MAGIC_getMagicNum( void)
{
	return MAGIC_magicnum;
}

/*----------------------------------------------------------------------
 *   法の設定ファイルを  む
 *---------------------------------------------------------------------*/
BOOL MAGIC_initMagic( char *filename)
{
    FILE*   f;
    char    line[256];
    int     linenum=0;
    int     magic_readlen=0;
	int		i,j;

	int		max_magicid =0;
	char	token[256];

    f = fopen(filename,"r");
    if( f == NULL ){
        print( "file open error\n");
        return FALSE;
    }

    MAGIC_magicnum=0;

    /*  まず  効な行が何行あるかどうか調べる    */
    while( fgets( line, sizeof( line ), f ) ){
        linenum ++;
        if( line[0] == '#' )continue;        /* comment */
        if( line[0] == '\n' )continue;       /* none    */
        chomp( line );

#ifdef _MAGIC_OPTIMUM // Robin ���X殻�jMAGIC ID
		if( getStringFromIndexWithDelim( line, ",", MAGIC_DATACHARNUM+MAGIC_ID+1,
				token, sizeof(token)) == FALSE )
			continue;
		max_magicid = max( atoi( token), max_magicid);
#endif

        MAGIC_magicnum++;
    }

#ifdef _MAGIC_OPTIMUM
	print("\n Τ��MT:%d 殻�jMT:%d \n", MAGIC_magicnum, max_magicid);
	MAGIC_magicnum = max_magicid +1;
#endif

    if( fseek( f, 0, SEEK_SET ) == -1 ){
        fprint( "Seek Error\n" );
        fclose(f);
        return FALSE;
    }

    MAGIC_magic = allocateMemory( sizeof(struct tagMagic)
                                   * MAGIC_magicnum );
    if( MAGIC_magic == NULL ){
        fprint( "Can't allocate Memory %d\n" ,
                sizeof(struct tagMagic)*MAGIC_magicnum);
        fclose( f );
        return FALSE;
    }

	/* 初期化 */
    for( i = 0; i < MAGIC_magicnum; i ++ ) {
    	for( j = 0; j < MAGIC_DATAINTNUM; j ++ ) {
    		MAGIC_setInt( i,j,-1);
    	}
    	for( j = 0; j < MAGIC_DATACHARNUM; j ++ ) {
    		MAGIC_setChar( i,j,"");
    	}
    }

    /*  また  み  す    */
    linenum = 0;
    while( fgets( line, sizeof( line ), f ) ){
        linenum ++;
        if( line[0] == '#' )continue;        /* comment */
        if( line[0] == '\n' )continue;       /* none    */
        chomp( line );

        /*  行を整形する    */
        /*  まず tab を " " に  き換える    */
        replaceString( line, '\t' , ' ' );
        /* 先  のスペースを取る�e*/
{
        char    buf[256];
        for( i = 0; i < strlen( line); i ++) {
            if( line[i] != ' ' ) {
                break;
            }
            strcpy( buf, &line[i]);
        }
        if( i != 0 ) {
            strcpy( line, buf);
        }
}
{
        char    token[256];
        int     ret;

#ifdef _MAGIC_OPTIMUM
		if( getStringFromIndexWithDelim( line, ",", MAGIC_DATACHARNUM+MAGIC_ID+1,
				token, sizeof(token)) == FALSE )
			continue;
		magic_readlen = atoi( token);
#endif

		for( i = 0; i < MAGIC_DATACHARNUM; i ++ ) {

	        /*    字  用トークンを見る    */
	        ret = getStringFromIndexWithDelim( line,",",
	        									i + 1,
	        									token,sizeof(token));
	        if( ret==FALSE ){
	            fprint("Syntax Error file:%s line:%d\n",filename,linenum);
	            break;
	        }
	        MAGIC_setChar( magic_readlen, i, token);
		}
        /* 4つ  以降は数値データ */
#define	MAGIC_STARTINTNUM		5
        for( i = MAGIC_STARTINTNUM; i < MAGIC_DATAINTNUM+MAGIC_STARTINTNUM; i ++ ) {
            ret = getStringFromIndexWithDelim( line,",",i,token,
                                               sizeof(token));

#ifdef __ATTACK_MAGIC
            
            if( FALSE == ret )

            	break;
            	
            if( 0 != strlen( token ) )
            {	
            	MAGIC_setInt( magic_readlen , i - MAGIC_STARTINTNUM , atoi( token ) );
            }
            	                                               
#else
                                               
            if( ret==FALSE ){
                fprint("Syntax Error file:%s line:%d\n",filename,linenum);
                break;
            }
            if( strlen( token) != 0 ) {
	            MAGIC_setInt( magic_readlen, i - MAGIC_STARTINTNUM, atoi( token));
			}
			
	    #endif
        }

#ifdef __ATTACK_MAGIC
        
        if( i != MAGIC_STARTINTNUM + MAGIC_IDX && i != MAGIC_DATAINTNUM + MAGIC_STARTINTNUM )
        	continue;
        	
#else
        
        if( i < MAGIC_DATAINTNUM+MAGIC_STARTINTNUM )
        	 continue;
        	 
#endif
		/* ちょっと不細工だけどこうする�e */
		if( MAGIC_getInt( magic_readlen, MAGIC_TARGET_DEADFLG) == 1 ) {
			MAGIC_setInt( magic_readlen, MAGIC_TARGET,
						MAGIC_getInt( magic_readlen, MAGIC_TARGET)+100);
		}

        magic_readlen ++;
}
    }
    fclose(f);

    MAGIC_magicnum = magic_readlen;


    print( "Valid magic Num is %d...", MAGIC_magicnum );

	/* hash の登   */
	for( i = 0; i < arraysizeof( MAGIC_functbl); i ++ ) {
		MAGIC_functbl[i].hash = hashpjw( MAGIC_functbl[i].functionname);
	}
#if 0
    for( i=0; i <MAGIC_magicnum ; i++ ){
        for( j = 0; j < MAGIC_DATACHARNUM; j ++ ) {
	        print( "%s ", MAGIC_getChar( i, j));
		}
		print( "\n");
        for( j = 0; j < MAGIC_DATAINTNUM; j ++ ) {
            print( "%d ", MAGIC_getInt( i, j));
        }
        print( "\n-------------------------------------------------\n");
    }
#endif
    return TRUE;
}
/*------------------------------------------------------------------------
 * Magicの設定ファイル  み  し
 *-----------------------------------------------------------------------*/
BOOL MAGIC_reinitMagic( void )
{
	freeMemory( MAGIC_magic);
	return( MAGIC_initMagic( getMagicfile()));
}


#ifdef __ATTACK_MAGIC

/*------------------------------------------------------------------------
 * AttMagic�昏讒lて
 *-----------------------------------------------------------------------*/
BOOL ATTMAGIC_initMagic( char *filename )
{
    FILE *file;

	// Open file
    if( NULL == ( file = fopen( filename , "r" ) ) )
	{ 
	    ATTMAGIC_magicnum	= 0;
		ATTMAGIC_magic		= NULL;

        return TRUE;
    }

	fseek( file , 0 , SEEK_END );

	// Calculate the number of attack magics
	ATTMAGIC_magicnum = ftell( file ) / sizeof( struct tagAttMagic );
	if( ATTMAGIC_magicnum % 2 )
	{
		fprint( "File format is illegal\n" );
		fclose( file );

		return FALSE;
	}

        fseek( file , 0 , SEEK_SET );
        
	// Allocate memory to attack magics
    ATTMAGIC_magic = allocateMemory( sizeof( struct tagAttMagic ) * ATTMAGIC_magicnum );
	if( NULL == ATTMAGIC_magic )
	{
		fprint( "Can't allocate Memory %d\n" , sizeof( struct tagAttMagic ) * ATTMAGIC_magicnum );
		fclose( file );

		return FALSE;
    }

	// Read attack magics information
	memset( ATTMAGIC_magic , 0 , sizeof( struct tagAttMagic ) * ATTMAGIC_magicnum );
	fread( ATTMAGIC_magic , 1 , sizeof( struct tagAttMagic ) * ATTMAGIC_magicnum , file );
	
	fclose( file );

	ATTMAGIC_magicnum = ATTMAGIC_magicnum / 2;

    print( "Valid attmagic Num is %d\n" , ATTMAGIC_magicnum );

	return TRUE;
}



/*------------------------------------------------------------------------
 * AttMagic�墾A�廬讒lて
 *-----------------------------------------------------------------------*/
BOOL ATTMAGIC_reinitMagic( void )
{
   freeMemory( ATTMAGIC_magic );
   ATTMAGIC_magicnum = 0;
   
   return ATTMAGIC_initMagic( getAttMagicfileName() );
//	    return ATTMAGIC_initMagic( getMagicfile() );
}

#endif

/*------------------------------------------------------------------------
 * MAGIC_IDから添字を知る関数
 * 返り値
 * ��  : 添字
 * 失  : -1
 *-----------------------------------------------------------------------*/
int MAGIC_getMagicArray( int magicid)
{
#ifdef _MAGIC_OPTIMUM
	if( magicid >= 0 && magicid < MAGIC_magicnum)
		return	magicid;
#else
	int		i;
	for( i = 0; i < MAGIC_magicnum; i ++ ) {
		if( MAGIC_magic[i].data[MAGIC_ID] == magicid ) {
			return i;
		}
	}
#endif
	return -1;
}
/*------------------------------------------------------------
 * 呪術の関数  からポインターを返す
 * 引数
 *  name        char*       呪術の  前
 * 返り値
 *  関数へのポインタ�eない場合にはNULL
 ------------------------------------------------------------*/
MAGIC_CALLFUNC MAGIC_getMagicFuncPointer(char* name)
{
    int i;
    int hash; //ttom
    //ttom 12/18/2000
    if(name==NULL) return NULL;
    //ttom
    //int hash = hashpjw( name );
    hash=hashpjw(name);
    for( i = 0 ; i< arraysizeof( MAGIC_functbl) ; i++ ) {
        if( MAGIC_functbl[i].hash == hash ) {
        	if( strcmp( MAGIC_functbl[i].functionname, name ) == 0 )  {
	            return MAGIC_functbl[i].func;
			}
		}
	}
    return NULL;
}


// Nuke start (08/23)
/*
  台   Nuke さんのチェック�e
    法の効果  囲をチェックする�e

  Check the validity of the target of a magic.
  Return value:
	0: Valid
	-1: Invalid
*/
int MAGIC_isTargetValid( int magicid, int toindex)
{
	int marray;
	marray= MAGIC_getMagicArray( magicid);

	#ifdef __ATTACK_MAGIC

	if( toindex >= 0 && toindex <= 19 )
		return 0;

	// One side of players
	if( 20 == toindex || 21 == toindex )
	{
		if( MAGIC_TARGET_WHOLEOTHERSIDE == MAGIC_magic[marray].data[MAGIC_TARGET] || MAGIC_TARGET_ALL_ROWS == MAGIC_magic[marray].data[MAGIC_TARGET] ) 
			return 0;
		else
			return -1;
	}

	// All players
	if( 22 == toindex )
	{
		if( MAGIC_TARGET_ALL == MAGIC_magic[marray].data[MAGIC_TARGET] ) 
			return 0;
		else
			return -1;
	}

	// One row
	if( 23 == toindex || 24 == toindex || 25 == toindex || 26 == toindex )
	{
		if( MAGIC_TARGET_ONE_ROW == MAGIC_magic[marray].data[MAGIC_TARGET] )
			return 0;
		else
			return -1;
	}

	#else

	// Single player
	if ((toindex >= 0x0) && (toindex <= 0x13)) return 0;
	// All players
	if (toindex == 0x16) {
		if (MAGIC_magic[marray].data[MAGIC_TARGET] == MAGIC_TARGET_ALL) 
			return 0; else return -1;
	}
	// One side of players
	if ((toindex == 0x14) || (toindex == 0x15)) {
		if (MAGIC_magic[marray].data[MAGIC_TARGET] == MAGIC_TARGET_WHOLEOTHERSIDE) 
			return 0; else return -1;
	}
	
	#endif
	
	// Others: Error
	return -1;
}
// Nuke end
