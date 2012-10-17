#ifndef __LINK_H__
#define __LINK_H__

#include "common.h"

/*
 * リスト構造を定義する  �e
 * このリストはvalは    的には何もしない�eつまりポインターのコピーのみ
 * 行う�e
 */
typedef struct tagNode
{
    struct tagNode* next;           /*鵜のノードへのポインター*/
    char* val;                      /*保  する  字  */
    int size;                       /*valのサイ�N*/
}Node;

BOOL Nodeappendhead( Node** top  , Node* add );
BOOL Nodeappendtail( Node** top  , Node* add );
BOOL Noderemovehead( Node** top , Node* ret);
BOOL Noderemovetail( Node** top , Node* ret);
#endif
