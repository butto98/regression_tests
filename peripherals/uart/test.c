/*
 * Copyright (C) 2018 ETH Zurich and University of Bologna
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* 
 * Mantainer: Luca Valente (luca.valente2@unibo.it)
 */

#include <stdio.h>
#include "pulp.h"

#define BUFFER_SIZE 16
#define UART_BAUDRATE 115200

int tx_buffer[BUFFER_SIZE];
int rx_buffer[BUFFER_SIZE];

int uart_read_nb(int uart_id, void *buffer, uint32_t size)
{
  int periph_id = ARCHI_UDMA_UART_ID(uart_id);
  int channel = UDMA_CHANNEL_ID(periph_id);

  unsigned int base = hal_udma_channel_base(channel);

  plp_udma_enqueue(base, (int)buffer, size, UDMA_CHANNEL_CFG_EN | UDMA_CHANNEL_CFG_SIZE_8);

  //uart_wait_rx_done(periph_id);

  return 0;
}

int uart_write_nb(int uart_id, void *buffer, uint32_t size)
{
  int periph_id = ARCHI_UDMA_UART_ID(uart_id);
  int channel = UDMA_CHANNEL_ID(periph_id) + 1;

  unsigned int base = hal_udma_channel_base(channel);

  plp_udma_enqueue(base, (int)buffer, size, UDMA_CHANNEL_CFG_EN | UDMA_CHANNEL_CFG_SIZE_8);

  //uart_wait_tx_done(periph_id);

  return 0;
}

int main()
{

int error = 0;
int setup;
int k;
int a;

tx_buffer[0] = 'S';
tx_buffer[1] =  't';
tx_buffer[2] =  'a';
tx_buffer[3] =  'y';
tx_buffer[4] =  ' ';
tx_buffer[5] =  'a';
tx_buffer[6] =  't';
tx_buffer[7] =  ' ';
tx_buffer[8] = 'h';
tx_buffer[9] = 'o';
tx_buffer[10] = 'm';
tx_buffer[11] = 'e';
tx_buffer[12] = '!';
tx_buffer[13] = '!';
tx_buffer[14] = '!';
tx_buffer[15] = '!';

for (int u = 0; u < ARCHI_UDMA_NB_UART; ++u)
{
  for (int j = 0; j < BUFFER_SIZE; ++j)
  {
    rx_buffer[j] = 0;
  }

  printf("[%d, %d] Start test uart %d\n",  get_cluster_id(), get_core_id(), u);
  uart_open(u,UART_BAUDRATE);

  k=0;
  a=0;
  //configure pin function for RTS e CTS
  setup = pulp_read32(0x1A104018);
  setup = setup | 0x00000028;
  pulp_write32(0x1A104018, setup);
  //enable control flow inside UART peripheral
  setup = pulp_read32(ARCHI_UDMA_ADDR + UDMA_UART_OFFSET(channel) + UDMA_CHANNEL_CUSTOM_OFFSET + UART_SETUP_OFFSET);
  setup = setup | 0x00C0;
  pulp_write32(ARCHI_UDMA_ADDR + UDMA_UART_OFFSET(channel) + UDMA_CHANNEL_CUSTOM_OFFSET + UART_SETUP_OFFSET, setup);
  //fase 1 e 2
  while(a < BUFFER_SIZE)
  {
    if(pulp_read32(ARCHI_UDMA_ADDR + UDMA_UART_OFFSET(channel) + UDMA_CHANNEL_CUSTOM_OFFSET + UART_STATUS_OFFSET) & 0x0004)
    {
      uart_read(u,rx_buffer + k,1);     //--- blocking read
      k=k+1;
      //uart_write(u,tx_buffer + a,1);  //--- blocking write
      //a=a+1;
    }
    else
    {
      uart_write(u,tx_buffer + a,1);  //--- blocking write
      a=a+1;
    }
  }
  //fase 3
  while(k < BUFFER_SIZE)
  {
    uart_read(u,rx_buffer + k,1);     //--- blocking read
    k=k+1;
  }

  for(int b = 0; b < BUFFER_SIZE; ++b)
  {    
    //uart_write_nb(u,tx_buffer + b,1); //--- non blocking write
    //uart_read(u,rx_buffer + b,1);     //--- blocking read
    if (tx_buffer[b] == rx_buffer[b])
    {
      printf("PASS: tx %c, rx %c\n", tx_buffer[b],rx_buffer[b]);
    }else{
      printf("FAIL: tx %c, rx %c\n", tx_buffer[b],rx_buffer[b]);
      error++;
    }
  }
}
  return error;
}

/*for (int i = 0; i < BUFFER_SIZE; ++i)
  {
    uart_write(u,tx_buffer + i,1); //--- non blocking write
    uart_read(u,rx_buffer + i,1);     //--- blocking read
    if(i > 10)
    {
      if(a < BUFFER_SIZE)
      {
        uart_read(u,rx_buffer + a,1);     //--- blocking read
        a++;
      }
    }
  }*/