#include "version.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "object.h"
#include "char_base.h"
#include "char.h"
#include "util.h"
#include "npcutil.h"
#include "item.h"
#include "readmap.h"


#define RANGE 2
void NPC_HealerSpeak( int index, int talker);
void NPC_HealerAllHeal( int talker );
BOOL NPC_MoneyCheck(int meindex,int talker);
int NPC_CostCheck(int talker);
void NPC_CharCheckPoint(int meindex,int talker);
int NPC_WorkInput(int meindex,int talker);


/**********************************
初期化
************************************/
BOOL NPC_HealerInit( int meindex )
{
	//タイプをヒーラーに設定
    CHAR_setInt( meindex , CHAR_WHICHTYPE , CHAR_TYPEHEALER );
    //  撃できない    要ないかも  
    CHAR_setFlg( meindex , CHAR_ISATTACKED , 0 );
    //上に乗れない
//    CHAR_setFlg( meindex , CHAR_ISOVERED , 0 );

	return TRUE;

}




/*-------------------------------------------
 *
 *   され�bHP,MPを全開にする
 *
 --------------------------------------------*/
void NPC_HealerTalked( int meindex , int talker , char *msg ,int color )
{

    char*   npcarg;
    char    token[32];
	int msgNo;


	npcarg = CHAR_getChar(meindex,CHAR_NPCARGUMENT);

    getStringFromIndexWithDelim( npcarg, "|", 1, token,sizeof( token));
    msgNo  = atoi( token );

    /*---プレイヤーに対してだけ  応する---*/
    if( CHAR_getInt( talker , CHAR_WHICHTYPE ) != CHAR_TYPEPLAYER )	return;

	/*---  囲以  か！  囲は1または�bしんでいるか！---*/
	/* １グリッド以  のみ */
	if( NPC_Util_CharDistance( talker, meindex ) > 2) return;

	if( (CHAR_getWorkInt( talker, CHAR_WORKPARTYMODE) == 0)
	|| (CHAR_getWorkInt( talker, CHAR_WORKPARTYMODE) == 2) )
	{
		/*--回復させましょう--*/
		NPC_HealerAllHeal( talker);
	    if(msgNo == 1) {
		    CHAR_talkToCli( talker, meindex,
			    "�w�g��魁�^�_�C出�b�UΩ�困饒匹ぅ[�o�髻I",CHAR_COLORWHITE);

		}else if(msgNo == 2) {
		    CHAR_talkToCli( talker, meindex,
		    	"パ�鵲A�楔杞蝓A統и整�A�^�_�a�I",CHAR_COLORWHITE);
		}

	}else{
		int i = 0;
		int otherindex;

		for( i=0 ; i < CHAR_PARTYMAX ; i++)
		{
			otherindex = CHAR_getWorkInt( talker, CHAR_WORKPARTYINDEX1 + i);
			if(otherindex != -1){

				NPC_HealerAllHeal( otherindex);
				if(msgNo == 1) {
				    CHAR_talkToCli( otherindex, meindex,
				    	"�w�g��魁�^�_�C出�b�UΩ�困饒匹ぅ[�o�髻I",CHAR_COLORWHITE);

				}else if(msgNo == 2) {
					CHAR_talkToCli( otherindex, meindex, 
						"パ�鵲A�楔杞蝓A統и整�A�^�_�a�I",CHAR_COLORWHITE);
				}
			}
		}
	}
}


/*----------------------*/
/* 回復 */
/*-----------------------*/
void NPC_HealerAllHeal( int talker )
{
	int i;
	int petindex;
	char petsend[3];	
	char msgbuf[5];
	
    CHAR_setInt( talker , CHAR_HP ,CHAR_getWorkInt( talker, CHAR_WORKMAXHP ) );
    CHAR_setInt( talker , CHAR_MP ,CHAR_getWorkInt( talker, CHAR_WORKMAXMP ) );



	for(i = 0 ; i < CHAR_MAXPETHAVE; i ++) {
		petindex = CHAR_getCharPet( talker, i);

		if( petindex == -1  )  continue;

		/*  キャラの    チェック    */
		if( !CHAR_CHECKINDEX( talker ) )  continue;

		/* ペットのindexチェックをする */
		if( CHAR_CHECKINDEX( petindex) == FALSE ) continue;

		/*--回復--*/
		CHAR_setFlg( petindex, CHAR_ISDIE, 0);
		CHAR_setInt( petindex , CHAR_HP ,CHAR_getWorkInt( petindex, CHAR_WORKMAXHP ) );
		CHAR_setInt( petindex , CHAR_MP ,CHAR_getWorkInt( petindex, CHAR_WORKMAXMP ) );

		/*--パラメータ調整--*/
		CHAR_complianceParameter( petindex );
		sprintf( petsend, "K%d", i );
		CHAR_sendStatusString( talker , petsend );
	}
	
	/*---仲間がいれば仲間にも送信--*/
	if(CHAR_getWorkInt( talker, CHAR_WORKPARTYMODE) != CHAR_PARTY_NONE )
	{
		int	topartyarray = -1;
		int	oyaindex = CHAR_getWorkInt( talker	, CHAR_WORKPARTYINDEX1);
	
		if( CHAR_CHECKINDEX( oyaindex )) {
			int	i;
	
			/* 自分と治したやつの仲間の    の場所を取   */
			for( i = 0; i < CHAR_PARTYMAX; i ++ ) {
				int workindex = CHAR_getWorkInt( oyaindex, CHAR_WORKPARTYINDEX1 + i);
				if( CHAR_CHECKINDEX( workindex) ) {
					if( workindex == talker ) {
						topartyarray = i;
						break;
					}
				}
			}
			
			for( i = 0; i < CHAR_PARTYMAX; i ++ ) {
				int otherindex = CHAR_getPartyIndex( talker, i);
				/* 仲間パラメータを送る */
				if( CHAR_CHECKINDEX( otherindex) ) {
					snprintf( msgbuf, sizeof( msgbuf), "N%d", topartyarray);
					if( otherindex != talker) {
						CHAR_sendStatusString( otherindex, msgbuf);
					}
				}
			}
		}
	}

	/*--パラメータ送り--*/
	CHAR_send_P_StatusString( talker, CHAR_P_STRING_HP);
	CHAR_send_P_StatusString( talker, CHAR_P_STRING_MP);

}

