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
  int block_remainder;
  *disk_num = linear_address / JBOD_DISK_SIZE;
  block_remainder = linear_address % JBOD_DISK_SIZE;
  *block_num = block_remainder / JBOD_BLOCK_SIZE;
  *offset = block_remainder % JBOD_BLOCK_SIZE;
  
}

int seek(int disk_num, int block_num)
{
  encode_operation(JBOD_SEEK_TO_DISK, disk_num, 0);
  encode_operation(JBOD_SEEK_TO_BLOCK, 0, block_num);
}

int mdadm_mount(void) {
  uint32_t op = encode_operation(JBOD_MOUNT, 0, 0); // pass zero's because its default
  int value = jbod_operation(op, NULL);
  if (value == 0)
    {
      return 1;
    }
  return -1;
    
}

int mdadm_unmount(void) {
  uint32_t last_op = encode_operation(JBOD_UNMOUNT,16, 256); //last command called so its on last disk, last block
  int value = jbod_operation(last_op, NULL);
  if (value == 0)
    {
      return 1;
    }
  return -1;
}

int mdadm_read(uint32_t addr, uint32_t len, uint8_t *buf) {
  
  int disk_num, block_num, offset;
  
  translate_address(addr, &disk_num, &block_num, &offset);
  seek(disk_num, block_num);
  uint32_t op = encode_operation(JBOD_READ_BLOCK, 0, 0);
  uint32_t buf1[512];
  if (len >= 1025 || (len > 0 && *buf == NULL)
    {
      return //fail. what should I return
    }
  if (addr >= (JBOD_NUM_DISKS * JBOD_DISK_SIZE)+len)//== 1 MB - 1?
    {
      
    }
  jbod_operation(op, buf1); 
  //buf1 contains data read from the disk [0-255]
  //memcopy buf1 into buf
  // then read again from 256-511 until the whole 1024
  jbod_operation(op, buf);
  //append buf1 to buf
  //use loop
  return len;
  //buf is the data is returned from jbod_operation
  
}
