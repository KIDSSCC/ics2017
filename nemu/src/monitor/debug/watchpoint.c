#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = &wp_pool[i + 1];
    memset(wp_pool[i].expr,0,32);
    wp_pool[i].value=0;
  }
  wp_pool[NR_WP - 1].next = NULL;

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */
void insert(WP*from,WP*what)
{
    WP* itea=from;
    while(itea->next!=NULL)
    {
        itea=itea->next;
    }
    itea->next=what;
}
WP* new_wp()
{
    Assert(free_!=NULL,"No free watchpoint left\n");
    //取出新的观察点，freelist向后移
    WP* newWp=free_;
    free_=free_->next;
    newWp->next=NULL;
    //新的观察点插入headlist
    if(head==NULL)
    {
        head=newWp;
    }
    else
        insert(head,newWp);
    return newWp;
}

void free_up(WP*wp)
{
    flash(wp);
    if(wp==head)
    {
        head=head->next;
        wp->next=NULL;
        insert(free_,wp);
    }
    else
    {
        //先找到这个结构到底在哪里
        WP*itea=head;
        while(itea->next!=NULL)
        {
            if(itea->next==wp)
            {
                break;
            }
            itea=itea->next;
        }
        Assert(itea->next!=NULL,"this watchpoint is not used\n");
        itea->next=itea->next->next;
        wp->next=NULL;
        insert(free_,wp);
    }
}
void printlist()
{
    printf("freelist: ");
    WP*itea=free_;
    while(itea!=NULL)
    {
        printf("%d ",itea->NO);
        itea=itea->next;
    }
    printf("\n");
    printf("Uselist: ");
    itea=head;
    while(itea!=NULL)
    {
        printf("%d ",itea->NO);
        itea=itea->next;
    }
    printf("\n");
    itea=head;
    while(itea!=NULL)
    {
        printf("No:%d,expr is %s,the value is %d\n",itea->NO,itea->expr,itea->value);
        itea=itea->next;
    }

}
void flash(WP*wp)
{
    memset(wp->expr,0,32);
    wp->value=0;
}
WP* getHead()
{
    return head;
}
void free_watchpoint(int n)
{
    if((n<0)||(n>NR_WP))
    {
        printf("out of the range\n");
        return ;
    }
    else
    {
        free_up(&wp_pool[n]);
    }
}

