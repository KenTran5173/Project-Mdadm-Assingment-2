#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "mdadm.h"
#include "jbod.h"

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
  encode_operation(JBOD_SEEK_TO_DISK, disk_num, 0);
  encode_operation(JBOD_SEEK_TO_BLOCK, 0, block_num);
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
  if (len == 0 && buf == NULL)
    {
      return 0;
      }
  
  if (mounted ==0 || len > 1024 || (len > 0 && buf == NULL) || addr+ len > (JBOD_NUM_DISKS * JBOD_DISK_SIZE)) //test cases
  {
    return -1; 
  }
  int x = len;
  int data_read=0;
  int counter = 0;
  int curr_addy = addr;
  while (x!=0){
    translate_address(curr_addy, &disk_num, &block_num, &offset);
    seek(disk_num, block_num);
    uint32_t op = encode_operation(JBOD_READ_BLOCK, 0, 0);
    uint8_t buf1[256];
    jbod_operation(op, buf1);
    
   if (counter == 0) //first block bc current block is equal to first block num             
    {
      printf("first block: %d, %d\n", data_read, offset);
      memcpy(buf+data_read, buf1+offset, min((JBOD_BLOCK_SIZE - offset),len));
      counter += 1; //increments the counter so not in first block anymore                      
    }
   else if (x < JBOD_BLOCK_SIZE)//last block, read whats left of the data by doing len subtract all the data read so far}
    {
      //offset = 0;
      printf("second block: %d, %d\n", data_read, offset);
      memcpy(buf+data_read, buf1+offset, x);
    }
  else
    {
      //offset = 0;
      printf("third block: %d, %d\n", data_read, offset);
      memcpy(buf+data_read, buf1, JBOD_BLOCK_SIZE); //middle blocks, so read full block size. offset is zero, new block
    }
   //in new blpck, but offset still the same
     data_read = JBOD_BLOCK_SIZE - offset;//sets y to what was read so we can add it to buf
     x = x - min(x,data_read);//decrements length
     curr_addy = curr_addy + (JBOD_BLOCK_SIZE-offset);
  }
  
  return len;
}
