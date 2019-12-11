#include "param.h"
#include "types.h"
#include "defs.h"
#include "x86.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"

struct {
  struct spinlock lock;
  struct shm_page {
    uint id;
    char *frame;
    int refcnt;
  } shm_pages[64];
} shm_table;

void shminit() {
  int i;
  initlock(&(shm_table.lock), "SHM lock");
  acquire(&(shm_table.lock));
  for (i = 0; i< 64; i++) {
    shm_table.shm_pages[i].id =0;
    shm_table.shm_pages[i].frame =0;
    shm_table.shm_pages[i].refcnt =0;
  }
  release(&(shm_table.lock));
}

int shm_open(int id, char **pointer) {

//you write this
  int i = 0;
  uint va;
  int id_found = 0;
  acquire(&(shm_table.lock));
  for (i = 0; i< 64; i++) {  
	if(shm_table.shm_pages[i].id == id)  //if segment with that id exist
	{
	  va = PGROUNDUP(myproc()->sz);
	  mappages(myproc()->pgdir, (void*)va, PGSIZE, V2P(shm_table.shm_pages[i].frame), PTE_W|PTE_U);  //map virtual address to physicall address 
	  shm_table.shm_pages[i].refcnt ++;  //increment reference counter by 1
	  *pointer=(char *)va;  //return virtual address 
	  myproc()->sz = PGROUNDUP(myproc()->sz+PGSIZE);  //increase sz
	  id_found = 1;  //in the first loop, we found the segment, set id_flag to 1
	}
  }
  if(id_found == 0){  //if segment with that id does not exist 
	for(i = 0; i < 64; i++){ 
	  if(shm_table.shm_pages[i].id == 0)  //find a empty page
	    {
		  shm_table.shm_pages[i].id = id;  //set id 
		  shm_table.shm_pages[i].frame = kalloc();  //allocate one page of physical memory
		  shm_table.shm_pages[i].refcnt = 1;  //set reference counter to 1
		  memset(shm_table.shm_pages[i].frame,  0, PGSIZE);  //set that one page of memory to 0
		  va = PGROUNDUP(myproc()->sz);
	      mappages(myproc()->pgdir, (void*)va, PGSIZE, V2P(shm_table.shm_pages[i].frame), PTE_W|PTE_U);  //map virtual address to physical address
		  *pointer=(char*)va;  //return virtual address
		  myproc()->sz = PGROUNDUP(myproc()->sz+PGSIZE);  //increase sz
		  break;
		}		
	}
  }
  release(&(shm_table.lock));


return 0; //added to remove compiler warning -- you should decide what to return
}


int shm_close(int id) {
//you write this too!

  int i;
  acquire(&(shm_table.lock));
  for (i = 0; i< 64; i++) {
    if(shm_table.shm_pages[i].id == id){
	  if(shm_table.shm_pages[i].refcnt > 1){  
		shm_table.shm_pages[i].refcnt--;    
	  }else if(shm_table.shm_pages[i].refcnt == 1){
	    shm_table.shm_pages[i].id = 0;  
		shm_table.shm_pages[i].frame = 0;  
		shm_table.shm_pages[i].refcnt = 0; 
	  }
	}
  }
  release(&(shm_table.lock));


return 0; //added to remove compiler warning -- you should decide what to return
}






