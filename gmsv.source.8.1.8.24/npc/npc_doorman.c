#include "version.h"
#include <string.h>
#include "object.h"
#include "char_base.h"
#include "char.h"
#include "item.h"
#include "util.h"
#include "handletime.h"
#include "npc_doorman.h"
#include "npc_door.h"
#include "npcutil.h"
#include "configfile.h"
/*
 *
 *    りにドアがあるときにそのドアを何らかの条件によって開くNPC.
 *  ドアのとなりにcreateするだけで�bそのドアをぁ作させることができる�e
 *  はなしかけられたときに全キャラを検索して�bまわり8マスにドアがいる
 *  場合はそのすべてに対して影響する�eふたつのドアが  時に開くことになる�e
 *
 *  インターフェイスはTalkで
 *
 * ドアを開くために
 *
 * 1 恭金を徴収する�e徴収できたらひらく     gold|100
 * 2 アイ  ムを1個徴収する �e徴収できたらひらく  item|45
 * 3 アイ  ムを  っているかどうか調べる�e   っていたら開く�eitemhave|44
 * 4 アイ  ムを  っていないかどうか調べる�e  っていなかったら開く�e
 *          itemnothave|333
 * 5 称号をもっているかどうか調べる�e  っていたら開く�e titlehave|string
 * 6 称号をもっていないかどうか調べる�e  っていなかったら開く�e
 *      titlenothave|string
 *
 * かならず質  に答えると開く�e金の場合は�b
 *「100ゴールドいただきますがいいですか！」で「はい」というと100ゴールド
 * とられる�eいきなり「はい」だけ言ってもとられる�eで�b「100ゴールド
 * いただきました�e」と言われる�e
 *
 * アイ  ム徴収の場合は�b「何々を一個いただきますがいいですか！」ときく�e
 *  3から6の場合は�b何かはなしかけて条件がそろってたら開く�e
 *
 *
 *
 *    ストの  法
 *
 *1  ドアをてきとうに  く
 *2  このNPCを適当にドアのとなりに  く�e引数を gold|100 にする
 *3  このNPCに対して�b100ゴールド以上もっている状態で「はい」と言う
 *4  ドアがひらいて金が減ったら��  �e
 *
 */

static void NPC_DoormanOpenDoor( char *nm  );

BOOL NPC_DoormanInit( int meindex )
{
	char	arg1[NPC_UTIL_GETARGSTR_BUFSIZE];
	char *arg;
    char dname[1024];

	/* イベントのタイプ設定 */
	CHAR_setWorkInt( meindex, CHAR_WORKEVENTTYPE,CHAR_EVENT_NPC);

    CHAR_setInt( meindex , CHAR_HP , 0 );
    CHAR_setInt( meindex , CHAR_MP , 0 );
    CHAR_setInt( meindex , CHAR_MAXMP , 0 );
    CHAR_setInt( meindex , CHAR_STR , 0 );
    CHAR_setInt( meindex , CHAR_TOUGH, 0 );
    CHAR_setInt( meindex , CHAR_LV , 0 );

    CHAR_setInt( meindex , CHAR_WHICHTYPE , CHAR_TYPETOWNPEOPLE );
    CHAR_setFlg( meindex , CHAR_ISOVERED , 0 );
    CHAR_setFlg( meindex , CHAR_ISATTACKED , 0 );  /*   撃されないよん */

	arg = NPC_Util_GetArgStr( meindex, arg1, sizeof( arg1));

    if(!getStringFromIndexWithDelim( arg, "|", 3, dname, sizeof(dname ))){
        print("RINGO: �]�w�櫑��脈瓢櫚n���墾W�r�髻I:%s:\n",
              arg );
        return FALSE;
    }
    print( "RINGO: Doorman create: arg: %s dname: %s\n",arg,dname);
    CHAR_setWorkChar( meindex , CHAR_WORKDOORMANDOORNAME , dname );

    return TRUE;
}

void NPC_DoormanTalked( int meindex , int talkerindex , char *msg ,
                     int color )
{
    char mode[128];
    char opt[256];
    char	arg1[NPC_UTIL_GETARGSTR_BUFSIZE];
    char *arg;

    /* プレイヤーがドアマンの1グリッド以  ならはんのう */
    if(NPC_Util_CharDistance( talkerindex, meindex ) > 1)return;

	arg = NPC_Util_GetArgStr( meindex, arg1, sizeof( arg1));

    if( !getStringFromIndexWithDelim( arg, "|", 1, mode, sizeof( mode )))
        return;

    if( !getStringFromIndexWithDelim( arg, "|", 2, opt, sizeof( opt ) ))
        return;

    if( strcmp( mode , "gold" ) == 0 ){
        int g = atoi( opt );
        int yn = NPC_Util_YN( msg );
        /*char *nm = CHAR_getChar( meindex , CHAR_NAME );*/
        char msg[256];

        if( g > 0 && yn < 0 ){
            snprintf( msg ,sizeof( msg ) ,
                      "ゴ�}��旨�n宜и%d�昏��l�o舎�i�H芹�H", g );
            CHAR_talkToCli( talkerindex, meindex , msg, CHAR_COLORWHITE );
        } else if( g > 0 && yn == 0 ){
            snprintf( msg , sizeof( msg ),
                      "ゴ�}�� %d�昏��l�Oゲ�n�此C", g );
        } else if( g > 0 && yn == 1 ){
            int now_g = CHAR_getInt( talkerindex, CHAR_GOLD );
            if( now_g < g ){
                snprintf( msg , sizeof( msg ) ,
                          "ゴ�}�� %d�昏��l�Oゲ�n�此C", g );
            	CHAR_talkToCli( talkerindex, meindex , msg, CHAR_COLORWHITE );
            } else {
                snprintf( msg , sizeof( msg ),
                          "%d Μ�讓��l�F�C�{�b�N�啅}���C", g );
            	CHAR_talkToCli( talkerindex, meindex , msg, CHAR_COLORWHITE );

                /* 恭金をゲット */
                now_g -= g;
                CHAR_setInt( talkerindex , CHAR_GOLD , now_g );
                /* あたらしいス  ータスを送信 */
                CHAR_send_P_StatusString(talkerindex, CHAR_P_STRING_GOLD);

                /* ドアひらく */
                NPC_DoormanOpenDoor(
                    CHAR_getWorkChar( meindex, CHAR_WORKDOORMANDOORNAME));
            }
        }
    } else if( strcmp( mode , "item" ) == 0 ){
        CHAR_talkToCli( talkerindex, meindex ,
                        "�|�bゼや刊社Α�C",
                        CHAR_COLORWHITE);
    } else if( strcmp( mode , "itemhave" ) == 0 ){
        CHAR_talkToCli( talkerindex, meindex ,
                        "�|�bゼや刊社Α�C",
                        CHAR_COLORWHITE);
    } else if( strcmp( mode , "itemnothave" ) == 0 ){
        CHAR_talkToCli( talkerindex, meindex ,
                        "�|�bゼや刊社Α�C",
                        CHAR_COLORWHITE);
    } else if( strcmp( mode , "titlehave" ) == 0 ){
        CHAR_talkToCli( talkerindex, meindex ,
                        "�|�bゼや刊社Α�C",
                        CHAR_COLORWHITE);

    } else if( strcmp( mode , "roomlimit" ) == 0 ){

		/*   屋の人数制限がある場合 */
		char szOk[256], szNg[256], szBuf[32];
		int checkfloor;
		int maxnum, i, iNum;

	    if( !getStringFromIndexWithDelim( arg, "|", 2, szBuf, sizeof( szBuf ) ))
    	    return;

		/* 調べるフひアと    人数 */
		if( sscanf( szBuf, "%d:%d", &checkfloor, &maxnum ) != 2 ){
			return;
		}

		for( iNum = 0,i = 0; i < getFdnum(); i ++ ){
			/* プレイヤー以外には興  が  い */
			if( CHAR_getCharUse( i ) == FALSE )continue;
			if( CHAR_getInt( i, CHAR_WHICHTYPE ) != CHAR_TYPEPLAYER )continue;
			/* 指定のフひア以外に興  が  い */
			if( CHAR_getInt( i, CHAR_FLOOR ) != checkfloor )continue;
			iNum++;
		}
	    if( !getStringFromIndexWithDelim( arg, "|", 5, szNg, sizeof( szNg ))){
   			strcpy( szNg, "�C�C�C�C" );	/* 資格なしのセリフ */
		}
    	if( !getStringFromIndexWithDelim( arg, "|", 4, szOk, sizeof( szOk ))){
   			strcpy( szOk, "�}���a�C�C�C" );	/* 資格ありのセリフ */
   		}

		if( iNum >= maxnum ){
			/*     を超えている場合 */
	        CHAR_talkToCli( talkerindex, meindex ,szNg, CHAR_COLORWHITE);
		}else{
			/*     に  たない場合 */
	        CHAR_talkToCli( talkerindex, meindex ,szOk, CHAR_COLORWHITE);
            NPC_DoormanOpenDoor(
                    CHAR_getWorkChar( meindex, CHAR_WORKDOORMANDOORNAME));
		}

    } else if( strcmp( mode , "titlenothave" ) == 0 ){
        CHAR_talkToCli( talkerindex, meindex ,
                        "�|�bゼや刊社Α�C",
                        CHAR_COLORWHITE);
    }
}

/*
 *    前で検索してヒットしたのをすべて開く�e
 *
 */
static void NPC_DoormanOpenDoor( char *nm)
{
    int doori = NPC_DoorSearchByName( nm );
    print( "RINGO: Doorman's Door: index: %d\n", doori );

    NPC_DoorOpen( doori , -1 );

}

