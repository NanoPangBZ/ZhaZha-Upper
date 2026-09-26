#include "com_host.h"
#include <windows.h>
#include <stdio.h>
#include <stdint.h>

static uint32_t start_time = 0;

static uint8_t host_buf[ 12 * 1024 ];
static com_host_handle_t host1;
static com_host_port_handle_t virtual_buf_port;

static int16_t virtual_buf_read(void* opt_ctx, uint8_t* buffer, uint16_t size)
{
    const uint8_t test_buf[] = {
        0xAA, 0x55, 0xFE, 0xFE, 0x00, 0x02, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x05, 0x9D
    };
    
    uint16_t read_len = sizeof(test_buf) > size ? size : sizeof(test_buf);
    memcpy( buffer, test_buf, read_len );

    return read_len;
}

static uint32_t get_tick_ms(void)
{
    return GetTickCount() - start_time;
}

void sys_info_rsp_handler( com_host_handle_t host,
                           com_host_port_handle_t port,
                           com_host_id_t sender_host_id,
                           com_host_msg_t* msg ,
                           uint8_t seq )
{
    if( !msg->is_ack )
    {
        printf("respond sys_info_rsp msg.\r\n");
        printf("msg data(len:%d): " , msg->data_len);
        for( int i = 0 ; i < msg->data_len ; i++ )
        {
            printf("0x%02X ", msg->data[i]);
        }
        printf("\r\n");

        if( msg->need_ack )
        {
            //回复ack
            com_host_send_ack( host , port , sender_host_id , msg , NULL , 0 , seq );
            printf("send sys_info_rsp ack\r\n");
        }
    }
    else
    {
        printf("receive sys_info_rsp ack\r\n");
    }
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
    host_desc.host_id = (com_host_id_t)COM_HOST_ID_ADMIN;
    host1 = com_host_create( &host_desc );

    //创建、添加端口
    com_host_port_desc_t port_desc;
    port_desc.name = "virtual_buf";
    port_desc.attr = COM_HOST_PORT_ATTR_DEFAULT & ~COM_HOST_PORT_ATTR_WRITEABLE;
    port_desc.pack_buf_size = 512;
    port_desc.unpack_buf_size = 512;
    port_desc.opt_ctx = NULL;
    port_desc.write = NULL;
    port_desc.read = virtual_buf_read;
    virtual_buf_port = com_host_add_port( host1, &port_desc );

    //添加消息响应
    com_host_add_msg_handler( host1 , COM_HOST_MSG_ID_REQ_SYS_INFO , sys_info_rsp_handler );

    uint32_t used_mem = 0;
    com_host_get_mem_info( host1 , NULL , &used_mem );

    printf("host create success, used mem=%d bytes\r\n", used_mem );

    //运行主机
    while(1)
    {
        com_host_run( host1 );
        Sleep(5000);
    }

    return 0;
}


