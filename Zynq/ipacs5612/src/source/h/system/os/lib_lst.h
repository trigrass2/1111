/* lib_lst.h - doubly linked list library header */

/* solar*/

#ifndef _LIB_LST_H
#define _LIB_LST_H

#ifdef __cplusplus
extern "C" {
#endif

/* type definitions */

typedef struct node		/* Node of a linked list. */
    {
    struct node *next;		/* Points at the next node in the list */
    struct node *previous;	/* Points at the previous node in the list */
    } NODE;


/* HIDDEN */

typedef struct			/* Header for a linked list. */
    {
    NODE node;			/* Header list node */
    int count;			/* Number of nodes in list */
    } LIST;

/* END_HIDDEN */

#define HEAD	node.next		/* first node in list */
#define TAIL	node.previous		/* last node in list */

#define lstFirst(pList) (((LIST *)pList)->HEAD)
#define lstLast(pList)  (((LIST *)pList)->TAIL)
#define lstNext(pNode)  (((NODE *)pNode)->next)

/* function declarations */
//NODE *	lstFirst (LIST *pList);
NODE *	lstGet (LIST *pList);
//NODE *	lstLast (LIST *pList);
NODE *	lstNStep (NODE *pNode, int nStep);
//NODE *	lstNext (NODE *pNode);
NODE *	lstNth (LIST *pList, int nodenum);
NODE *	lstPrevious (NODE *pNode);
int 	lstCount (LIST *pList);
int 	lstFind (LIST *pList, NODE *pNode);
void 	lstAdd (LIST *pList, NODE *pNode);
void 	lstConcat (LIST *pDstList, LIST *pAddList);
void 	lstDelete (LIST *pList, NODE *pNode);
void 	lstExtract (LIST *pSrcList, NODE *pStartNode, NODE *pEndNode,
	  		    LIST *pDstList);
void 	lstFree (LIST *pList);
void 	lstInit (LIST *pList);
void 	lstInsert (LIST *pList, NODE *pPrev, NODE *pNode);

#ifdef __cplusplus
}
#endif

#endif 
