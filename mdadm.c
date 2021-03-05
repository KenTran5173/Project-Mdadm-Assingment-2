#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "mdadm.h"
#include "jbod.h"
#include <stdlib.h>
char *help(uint8_t buf[], int length) {
  char *p = (char *)malloc(length * 6);
  for (int i = 0, n = 0; i < length; ++i) {
    if (i && i % 16 == 0)
      n += sprintf(p + n, "\n");
    n += sprintf(p + n, "0x%02x ", buf[i]);
  }
  return p;
}
uint32_t encode_operation(jbod_cmd_t cmd, int disk_num, int block_num)
{

  uint32_t op = cmd << 26 | disk_num << 22 | block_num;
  
  return op;
}

void translate_address(uint32_t linear_address,
		       int *disk_num,
		       int *block_num,
		       int *offset)
{
  int block_remainder=0;
  *disk_num = linear_address / JBOD_DISK_SIZE;
  block_remainder = linear_address % JBOD_DISK_SIZE;
  *block_num = block_remainder / JBOD_BLOCK_SIZE;
  *offset = block_remainder % JBOD_BLOCK_SIZE;  
}


int seek(int disk_num, int block_num)
{
  uint32_t op;
  uint32_t ops;  
  op = encode_operation(JBOD_SEEK_TO_DISK, disk_num, 0);
  ops = encode_operation(JBOD_SEEK_TO_BLOCK, 0, block_num);
  jbod_operation(op, NULL);
  jbod_operation(ops, NULL);
  return 1;
}
int min(int n, int n2)
{
  if (n>n2)
    {
      return n2;
    }
  else if (n==n2)
    {
      return n;
    }
  else
    {
      return n;
    }
}

int mounted = 0; 
int mdadm_mount(void) {
  uint32_t op = encode_operation(JBOD_MOUNT, 0, 0); // pass zero's because its default
  int value = jbod_operation(op, NULL);
  if (value == 0)
    {
       mounted = 1;
      return 1;
    }
  return -1;
    
}

int mdadm_unmount(void) {
  uint32_t last_op = encode_operation(JBOD_UNMOUNT,0, 0); //last command called so its on last disk, last block
  int value = jbod_operation(last_op, NULL);
  if (value == 0)
    {
      mounted =0;
      return 1;
    }
  return -1;
}

int mdadm_read(uint32_t addr, uint32_t len, uint8_t *buf) {
  
  int disk_num, block_num, offset;
  if (mounted == 0 || len > 1024 || (len && buf == NULL) || addr+ len >= (JBOD_NUM_DISKS * JBOD_DISK_SIZE)) //test cases
  {
    return -1; 
  }
  int total = len;
  int data_read = 0;
  int curr_addy = addr;
  int sumDR = 0;
  int counter = 0;
  uint8_t buf1[256];
  while (curr_addy<addr+len){
    
    translate_address(curr_addy, &disk_num, &block_num, &offset);
    seek(disk_num, block_num);
    uint32_t op = encode_operation(JBOD_READ_BLOCK, 0, 0);
    
    jbod_operation(op, buf1);
    char *out = help(buf1,256);
    printf("buf that i read:\n got:\n%s\n", out);
    printf("disk num: %d\nblock num: %d\noffset: %d\n", disk_num, block_num, offset); 
   if (counter == 0) //first block bc current block is equal to first block num      
    {
      //printf("\nfirst block:\ndata read: %d\noffset: %d\nlen: %d\n", data_read, offset, min((JBOD_BLOCK_SIZE - offset),len));
      //char *out = help(buf1,256);
      //printf("buf that i read:\n got:\n%s\n", out);
      //printf("disk num: %d\nblock num: %d\n", disk_num, block_num);
      memcpy(buf+sumDR, buf1+offset, min((JBOD_BLOCK_SIZE - offset),len));
      data_read = min((JBOD_BLOCK_SIZE - offset),len);
      //char *out_s = help(buf,len);
      //printf("failed:\n got:\n%s\n", out_s);
      counter += 1; //increments the counter so not in first block anymore
      //if last block, then disk + 1 to go to next disk  	
    }
   else if (total < JBOD_BLOCK_SIZE)//last block, read whats left of the data by using x
    {
      //printf("\nlast block:\ndata read: %d\noffset: %d\nlen: %d\n", sumDR, offset, total);
      //char *out = help(buf1,256);
      //printf("buf that i read:\n got:\n%s\n", out);

      //printf("disk num: %d\nblock num: %d\n", disk_num, block_num);
      
      memcpy(buf+sumDR, buf1, total);
      data_read = total;
  
    }
  else
    {
      //printf("\nmiddle block:\ndata read: %d\noffset: %d\nlen: %d\n", data_read, offset, JBOD_BLOCK_SIZE);
      memcpy(buf+sumDR, buf1, JBOD_BLOCK_SIZE); //middle blocks, so read full block size. offset is zero, new block
      data_read = JBOD_BLOCK_SIZE;
      
    }
  //data_read = JBOD_BLOCK_SIZE - offset;//sets data_read to what was read already so we can add it to buf
     printf("data read before adding: %d\n", data_read);
     sumDR = sumDR + data_read;
     total = total - data_read;//keeps track of the bytes left to read
     curr_addy = curr_addy + data_read;
  }
  return len;
}



