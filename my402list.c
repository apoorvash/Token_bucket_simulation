
#include<stdio.h>
#include<stdlib.h>
#include "my402list.h"
#include "cs402.h"

int My402ListLength(My402List *list)
{
	
	return (list->num_members);
	

}


int My402ListEmpty(My402List *list)
{
	if(list->num_members == 0)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


extern int My402ListAppend(My402List* list, void* obj)
{
	My402ListElem *newnode = (My402ListElem *)malloc(sizeof(My402ListElem));
	if(newnode == NULL)
	{
		return FALSE;
	}
	
	My402ListElem *curr;
	if (list->anchor.next == NULL)
	{
		list->anchor.next = newnode;
		list->anchor.prev = newnode;
		newnode->next =&list->anchor;
		newnode->prev =&list->anchor;
		newnode->obj = obj;
		
		//display(list);
		// call to display printf("%p",newnode->obj);
	}
	else
	{
		curr = My402ListLast(list);
		curr->next = newnode;
		newnode->prev = curr;
		newnode->next = &list->anchor;
		list->anchor.prev = newnode;
		newnode->obj = obj;
		
		//display(list);
		//call to display
	}
	

	(list->num_members)++;

	return TRUE;
	
}

int My402ListPrepend(My402List* list, void* obj)
{
	My402ListElem *newnode = (My402ListElem *)malloc(sizeof(My402ListElem));
	if(newnode == NULL)
	{
		return FALSE;
	}
	
	My402ListElem *curr;
	if(list->anchor.next ==  NULL)
	{
		list->anchor.next = newnode;
		list->anchor.prev = newnode;
		newnode->next = &list->anchor;
		newnode->prev = &list->anchor;
		newnode->obj = obj;
		
		//display(list);
	}
	else
	{
		curr = My402ListFirst(list);
		curr->prev = newnode;
		newnode->next = curr;
		list->anchor.next = newnode;
		newnode->prev = &list->anchor;
		newnode->obj = obj;
		
		//display(list);
	//call to display
	}
	(list->num_members)++;

	return TRUE;
	//My402ListUnlink(list,newnode);

}

void My402ListUnlink(My402List* list, My402ListElem* elem)
{
	My402ListElem *next_node = elem->next;
	My402ListElem *prev_node = elem->prev;
	
	prev_node->next = next_node;
	next_node->prev = prev_node;
	
	(list->num_members)--; 
}

void My402ListUnlinkAll(My402List* list)
{
	list->anchor.next = &list->anchor;
	list->anchor.prev = &list->anchor;
	
	list->num_members = 0;
}

int  My402ListInsertAfter(My402List* list, void* obj, My402ListElem* elem)
{
		My402ListElem *newnode = (My402ListElem*)malloc(sizeof(My402ListElem));
		
		My402ListElem *next_node = elem->next;
		elem->next = newnode;
		newnode->prev = elem;
		newnode->next = next_node;
		next_node->prev = newnode;
		newnode->obj = obj;
		
		(list->num_members)++;

		return TRUE;
		
		//display(list);
}


int  My402ListInsertBefore(My402List* list, void* obj, My402ListElem* elem)
{
		My402ListElem *newnode = (My402ListElem*)malloc(sizeof(My402ListElem));
		
		My402ListElem *prev_node = elem->prev;
		prev_node->next = newnode;
		newnode->prev = prev_node;
		newnode->next = elem;
		elem->prev = newnode;
		newnode->obj = obj;
		
		(list->num_members)++;

		return TRUE;
		//display(list);
}


My402ListElem *My402ListFirst(My402List* list)
{
		return list->anchor.next;
}


My402ListElem *My402ListLast(My402List* list)
{
		return list->anchor.prev;
}


My402ListElem *My402ListNext(My402List* list, My402ListElem* elem)
{
	if(elem->next == &list->anchor)
	{
		return NULL;
	}
	else
		return elem->next;
}


My402ListElem *My402ListPrev(My402List* list, My402ListElem* elem)
{
	if(elem->prev == &list->anchor)
	{
		return NULL;
	}
	else
	return elem->prev;
}



My402ListElem *My402ListFind(My402List* list, void *obj)
{
	My402ListElem *elem = NULL;
	

	for(elem = My402ListFirst(list); elem != NULL ; elem = My402ListNext(list,elem))
	{
		if(elem->obj == obj)
		{
			return elem;
		}
		
	}
	return NULL;	
}


int My402ListInit(My402List* list)
{
	My402List *mylist; 
	mylist = (My402List *)malloc(sizeof(My402List));
	mylist->num_members= 0;
	mylist->anchor.next = NULL;
	mylist->anchor.prev = NULL;
	mylist->anchor.obj = NULL;
	
	return TRUE;
}


/*void display(My402List* list)
{
	int num_items = My402ListLength(list);
	if(num_items == 0)
	{
		printf("Empty");
	}
	My402ListElem *curr;
	curr = My402ListFirst(list);
	while(curr->obj != 0)
	{
		printf("%d\n",curr->obj);
		curr = curr->next;
	}
}

int main()
{
	My402List *mylist; 
	mylist = (My402List *)malloc(sizeof(My402List));
	
	My402ListInit(mylist);	
	
	My402ListElem *myelem = NULL;
	myelem = mylist->anchor.next;
	myelem = myelem->next;

My402ListAppend(mylist,16);
	My402ListAppend(mylist,13);
	My402ListPrepend(mylist,74);
	My402ListAppend(mylist,24);
	My402ListPrepend(mylist,34);
	myelem = My402ListFind(mylist,74);
	if(myelem==NULL)
    {	printf("\nnot found");	}
    else
	{	printf("found %d",myelem->obj);	}
	//My402ListInsertAfter(mylist,152,myelem);
	printf("%d\n",mylist->num_members);
	display(mylist);
	
}

*/


