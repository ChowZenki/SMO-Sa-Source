#include "version.h"
#include "common.h"
#include "readmap.h"
#include "char.h"
#include "map_util.h"
#include "lssproto_serv.h"
#include "net.h"
#include "anim_tbl.h"

/*#define MAPEDITORTROUBLE*/
#define SPR_kmydamY CG_HIT_MARK_10 // anim_tbl.h に登  されるまで  のエフェクト

/*------------------------------------------------------------
 * �い韻襪�どうか  断する�eキャラが  んでいるかどうか引数がある�e
 * 引数
 *  ff          int     floor
 *  fx          int     x座  
 *  fy          int     y座  
 *  isfly       int       んでいるかどうか
 * 返り値
 *  �い韻�      TRUE(1)
 *  �い韻覆�    FALSE(0)
 ------------------------------------------------------------*/
BOOL MAP_walkAbleFromPoint( int ff, int fx, int fy, BOOL isfly )
{
    int map[2];

    if( !MAP_getTileAndObjData( ff,fx,fy, &map[0], &map[1] ) ){
        return FALSE;
	}

    if( isfly ){
        int i;
        for( i = 0 ; i < 2 ; i ++ )
            if( MAP_getImageInt( map[i], MAP_HAVEHEIGHT ) == TRUE  ){
                return FALSE;
			}
        return TRUE;
    }else{
        switch( MAP_getImageInt( map[1], MAP_WALKABLE ) ){
        case 0:
            return FALSE;
            break;
        case 1:
            if( MAP_getImageInt( map[0], MAP_WALKABLE ) == 1 ){
                return TRUE;
			}else{
                return FALSE;
			}
            break;
        case 2:
            return TRUE;
            break;
        default:
            return FALSE;
            break;
        }
    }

    return FALSE;
}



/*------------------------------------------------------------
 * �い韻襪�どうか  断する�e
 * 引数
 *  index       int     Char    での Index
 *  ff          int     floor id
 *  fx          int     x座  
 *  fy          int     y座  
 * 返り値
 *  �いい討茲�      TRUE(1)
 *  �いい討魯瀬�    FALSE(0)
 ------------------------------------------------------------*/
BOOL MAP_walkAble( int index,int ff, int fx, int fy)
{
    /*      キャラならOK    */
    if( CHAR_getFlg( index, CHAR_ISTRANSPARENT ) )      return TRUE;

    // Arminius 7.9 Airplane ok
    if( CHAR_getInt( index, CHAR_WHICHTYPE ) == CHAR_TYPEBUS) return TRUE;
    if( CHAR_getWorkInt( index, CHAR_WORKPARTYMODE) == CHAR_PARTY_CLIENT ) {
      int oyaindex = CHAR_getWorkInt( index, CHAR_WORKPARTYINDEX1);
      if( CHAR_CHECKINDEX( oyaindex)) {
        if (CHAR_getInt( oyaindex, CHAR_WHICHTYPE ) == CHAR_TYPEBUS) return TRUE;
      }
    }

    return MAP_walkAbleFromPoint( ff,fx,fy,CHAR_getFlg(index,CHAR_ISFLYING)|CHAR_getWorkInt(index,CHAR_WORKSKYWALKER) );
}

/*------------------------------------------------------------
 * 座  を指定して�bそこのオブジェクトに高さがあるかどうか
 * 引数
 *  fl              int     フひア
 *  x               int     x 座  
 *  y               int     y 座  
 * 返り値
 *  高さがある  TRUE
 *  高さがない  FALSE
 ------------------------------------------------------------*/
BOOL MAP_haveHeight( int fl, int x, int y )
{
    int     map[2];

    /*  マップデータを  てくる  */
    if( !MAP_getTileAndObjData( fl,x,y, &map[0], &map[1] ) )
        return FALSE;

    return MAP_getImageInt( map[1], MAP_HAVEHEIGHT );
}



/*----------------------------------------
 * ス  ータス  化系の  更をする
 * 引数
 *  index   int     キャラのインデックス
 *  map     int     マップデータ
 *  outof   BOOL    進む時�b退く時か TRUE ならば 進む時である�e
 * 返り値
 *  パラメータ  化をした    TRUE
 *  パラメータ  化をしなかった  FALSE
  ----------------------------------------*/
static BOOL MAP_changeCharStatusFromMapDataAndTime( int index,
                                                    int map, BOOL outof)
{
#if 0
// ストーンエイジでは使わない
    int i;
    int offset;
    BOOL    change=FALSE;
    static struct tagStatusInteractionOfBitAndDefine{
        int     mapdataindex;
        int     charadataindex;
    }statusInteraction[]={
        {MAP_INTOPOISON,        CHAR_POISON},
        {MAP_INTOPARALYSIS,     CHAR_PARALYSIS},
        {MAP_INTOSILENCE,       CHAR_SLEEP},
        {MAP_INTOSTONE,         CHAR_STONE},
        {MAP_INTODARKNESS,      CHAR_DRUNK},
        {MAP_INTOCONFUSION,     CHAR_CONFUSION},

        {MAP_OUTOFPOISON,       CHAR_POISON},
        {MAP_OUTOFPARALYSIS,    CHAR_PARALYSIS},
        {MAP_OUTOFSILENCE,      CHAR_SLEEP},
        {MAP_OUTOFSTONE,        CHAR_STONE},
        {MAP_OUTOFDARKNESS,     CHAR_DRUNK},
        {MAP_OUTOFCONFUSION,    CHAR_CONFUSION},
    };

    if( outof == FALSE )offset = 6;
    else                offset= 0;
    int     newdata;
    for( i = 0 ; i < 6 ; i ++ ){
        newdata = MAP_getImageInt(map,statusInteraction[i+offset].
                                  mapdataindex);
        if( newdata > 0 ){
            change=TRUE;
            /*  ス  ータスの設定    */
            CHAR_setInt( index,
                         statusInteraction[i+offset].charadataindex,
                         CHAR_getInt(index,
                                     statusInteraction[i+offset].
                                     charadataindex ) + newdata );
        }
    }
    return change;
#else
	return FALSE;
#endif
}


/*------------------------------------------------------------
 * Map イベントの pre postを処  する
 * 引数
 *  index       int     乗ろうとしているキャラのインデックス
 *  mode        BOOL    TRUEの時は�bin  FALSEの時は out に対応
 * 返り値なし
 ------------------------------------------------------------*/
static void MAP_dealprepostevent( int index, BOOL mode )
{
    int     map[2];
    int     i;
    int     damaged=FALSE,statuschange=FALSE;

    if( CHAR_getFlg(index,CHAR_ISFLYING) )
        /*    んでるやつには何もしない  */
        return;

    if( !MAP_getMapDataFromCharIndex( index  ,  map ) ) return;
    /*  damage */
    int damage;
    for( i = 0 ; i < 2 ; i ++ ){
        damage = MAP_getImageInt(map[i],
                                 mode ? MAP_INTODAMAGE : MAP_OUTOFDAMAGE);
        if( damage != 0 ){
            /*  ダメージ�い鮴気�  現するので  号  れ替え    */
            int opt[2]={SPR_kmydamY,-damage};
            damaged=TRUE;
            CHAR_setInt(index, CHAR_HP,
                        CHAR_getInt(index,CHAR_HP) + damage );
            CHAR_complianceParameter(index);
            /*  ダメージ受けたエフェクト出す    */
            CHAR_sendWatchEvent(CHAR_getWorkInt(index,CHAR_WORKOBJINDEX),
                                CHAR_ACTDAMAGE,opt,2,TRUE);
        }
        if( MAP_changeCharStatusFromMapDataAndTime(
            index,map[i], mode ? TRUE : FALSE ) )
            statuschange=TRUE;
    }
    if( damaged )       CHAR_sendStatusString(index,"M");
    if( statuschange ){
        CHAR_sendCToArroundCharacter(index);
        CHAR_sendStatusString(index,"P");
    }

}


/*----------------------------------------
 * そのタイルに乗る前に呼ばれる�eここでそこから退くと�bxxx系の
 * 事を  現する�e
 * 引数
 *  index       int     乗ろうとしているキャラのインデックス
 *  flooor      int     フひアID
 *  fx          int     x座  
 *  fy          int     y座  
 * 返り値
 *  なし
 ----------------------------------------*/
void MAP_preovered( int index )
{
    MAP_dealprepostevent( index, FALSE );
}

/*----------------------------------------
 * そのタイルに乗った  に呼ばれる�e
 * 引数
 *  index       int     乗ろうとしているキャラのインデックス
 * 返り値
 *  なし
 ----------------------------------------*/
void MAP_postovered( int index )
{
    MAP_dealprepostevent( index, TRUE );
}


/*----------------------------------------
 * キャラの周りのマップをすべて送る
 * 引数
 *  fd          int
 *  charaindex  int     キャラのインデックス
 * 返り値
 *  ��      TRUE(1)
 *  失      FALSE(0)
 ----------------------------------------*/
BOOL MAP_sendArroundCharNeedFD( int fd,int charaindex )
{
    char*   stringdata;
    int     x=CHAR_getInt(charaindex,CHAR_X);
    int     y=CHAR_getInt(charaindex,CHAR_Y);
    int     fl=CHAR_getInt(charaindex,CHAR_FLOOR);
    int     size=MAP_CHAR_DEFAULTSEESIZ;
    RECT    seekr,retr;
    seekr.x = x - (int)(size/2);
    seekr.y = y - (int)(size/2);
    seekr.width  = size;
    seekr.height = size;
#if 1
{
	int		tilesum, objsum, eventsum;
    stringdata = MAP_getChecksumFromRECT(fl,&seekr,&retr, &tilesum,&objsum,&eventsum);
    if( stringdata == NULL )
        return FALSE;

    lssproto_MC_send(fd,fl,
                    retr.x,              retr.y,
                    retr.x + retr.width, retr.y + retr.height,
                    tilesum,
                    objsum,
                    eventsum,
                    stringdata );
}
#else
    stringdata = MAP_getdataFromRECT(fl,&seekr,&retr);
    if( stringdata == NULL )
        return FALSE;

    lssproto_M_send(fd,fl,
                    retr.x,              retr.y,
                    retr.x + retr.width, retr.y + retr.height,
                    stringdata );
#endif
    return TRUE;
}


/*----------------------------------------
 * マップデータを  る�e
 * 引数
 *  charaindex      int     キャラのインデックス
 * 返り値
 *  ��      TRUE(1)
 *  失      FALSE(0)
 ----------------------------------------*/
BOOL MAP_sendArroundChar(int charaindex)
{
    int fd;
    fd = getfdFromCharaIndex( charaindex );
    if( fd == -1 )return FALSE;

    return MAP_sendArroundCharNeedFD(fd, charaindex);
}
