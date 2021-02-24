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
{int block_remainder;
 
  *disk_num = linear_address / JBOD_DISK_SIZE;
  block_remainder = linear_address % JBOD_DISK_SIZE;
  *block_num = block_remainder / JBOD_BLOCK_SIZE;
  *offset = block_remainder % JBOD_BLOCK_SIZE;
  
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
  return len;
}
