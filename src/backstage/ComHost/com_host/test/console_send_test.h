#include "com_host.h"
#include <windows.h>
#include <stdio.h>
#include <stdint.h>

static uint32_t start_time = 0;

static uint8_t host_buf[ 12 * 1024 ];
static com_host_handle_t host1;
static com_host_port_handle_t console_port;

static int16_t console_write(void* opt_ctx, const uint8_t* data, uint16_t size)
{
    for(  int16_t i = 0 ; i < size ; i++ )
    {
        printf("0x%02X ", data[i]);
    }
    printf("\r\n");
    return size;
}

static uint32_t get_tick_ms(void)
{
    return GetTickCount() - start_time;
}

int main(void)
{
    start_time = GetTickCount();

    //初始化com_host
    com_host_ll_depend_t ll_depend;
    ll_depend.stack_buff = host_buf;
    ll_depend.stack_buff_size = (uint16_t)sizeof(host_buf);
    ll_depend.stack_align = (uint8_t)4;
    ll_depend.get_time_ms = get_tick_ms;

    com_host_init( &ll_depend );

    //创建主机
    com_host_desc_t host_desc;
    host_desc.host_id = (com_host_id_t)COM_HOST_ID_UNKNOWN;
    host1 = com_host_create( &host_desc );

    //创建、添加端口
    com_host_port_desc_t port_desc;
    port_desc.name = "console";
    port_desc.attr = COM_HOST_PORT_ATTR_DEFAULT & ~COM_HOST_PORT_ATTR_READABLE;
    port_desc.pack_buf_size = 1024;
    port_desc.unpack_buf_size = 1024;
    port_desc.opt_ctx = NULL;
    port_desc.write = console_write;
    port_desc.read = NULL;
    console_port = com_host_add_port( host1, &port_desc );

    //添加路由
    com_host_add_route( host1 , (com_host_id_t)COM_HOST_ID_ADMIN , console_port );
    
    //发送请求系统信息消息
    com_host_msg_t msg;
    msg.msg_id = COM_HOST_MSG_ID_REQ_SYS_INFO;
    msg.data = NULL;
    msg.data_len = 0;
    msg.need_ack = 1;
    msg.is_ack = 0;
    com_host_send_msg( host1 , COM_HOST_ID_ADMIN , &msg , NULL );

    //运行主机
    while(1)
    {
        com_host_send_msg( host1 , COM_HOST_ID_ADMIN , &msg , NULL );
        com_host_run( host1 );
    }

    return 0;
}

