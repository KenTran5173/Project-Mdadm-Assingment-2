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
  uint32_t buf1[]; //store data into buf1 dont know how to store it
  jbod_operation(op, buf); //[0-255]
  //buf1 will contain data read from the disk
  //copy buf1 to buf
  jbod_operation(op, buf); //[256-511] until read the whole 1024
  //append buf1 to buf
  //use loop
  return len;
}
