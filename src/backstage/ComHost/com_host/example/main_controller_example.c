// #include "../com_host.h"
// #include <string.h>
// #include <stdlib.h>
// #include <pthread.h>

// uint8_t bsp_init(void)
// {
//     //do something
//     return 0;
// }

// static void ping_handler( com_host_handle_t host, com_host_id_t sender_host_id, com_host_msg_t* msg , uint8_t seq )
// {
//     //接受到其他主机发送的ping消息后应答
//     if( msg->need_ack )
//     {
//         com_host_msg_ping_ack_data_t ack_data;
//         memset(&ack_data,0,sizeof(ack_data));

//         ack_data.uptime = 123456; //fill your uptime here
//         com_host_send_ack( host, sender_host_id, msg, (uint8_t*)&ack_data, sizeof(ack_data), seq );
//     }
    
//     //主动ping其他主机后接收到的对应ack包
//     if( msg->is_ack )
//     {
//         com_host_msg_ping_ack_data_t* ack_data = (com_host_msg_ping_ack_data_t*)msg->data;
//         printf( "revieve host:%d ping(seq:%d) ack uptime:%d\n", sender_host_id, seq, ack_data->uptime );
//     }
// }

// static void reboot_cmd_handler( com_host_handle_t host, com_host_id_t sender_host_id, com_host_msg_t* msg , uint8_t seq )
// {
//     if( !msg->is_ack )
//     {
//         //响应重启命令
//         com_host_msg_cmd_reboot_data_t* cmd = (com_host_msg_cmd_reboot_data_t*)msg->data;
//         //重启
//         //reboot_system_after( cmd->delay );

//         //需要ack则发送ack
//         if(  msg->need_ack )
//         {
//             com_host_msg_cmd_reboot_ack_data_t ack_data;
//             memset(&ack_data,0,sizeof(ack_data));
//             ack_data.result = COM_HOST_MSG_RET_OK; //fill your result here
//             //发送ack
//             com_host_send_ack( host, sender_host_id, msg, (uint8_t*)&ack_data, sizeof(ack_data), seq );
//         }
//     }
// }

// static void voice_effect_cmd_set_handler( com_host_handle_t host, com_host_id_t sender_host_id, com_host_msg_t* msg , uint8_t seq )
// {
//     if( !msg->is_ack )
//     {
//         //设置效果器命令
//         com_host_msg_cmd_set_voice_effect_data_t* cmd = (com_host_msg_cmd_set_voice_effect_data_t*)msg->data;
//         //设置效果器
//         //set_voice_effect( cmd->effect_type, cmd->params, cmd->params_len );

//         //需要ack则发送ack
//         if(  msg->need_ack )
//         {
//             com_host_msg_cmd_voice_effect_set_ack_data_t ack_data;
//             memset(&ack_data,0,sizeof(ack_data));
//             ack_data.result = COM_HOST_MSG_RET_OK; //fill your result here
//             //发送ack
//             com_host_send_ack( host, sender_host_id, msg, (uint8_t*)&ack_data, sizeof(ack_data), seq );
//         }
//     }
// }

// static void voice_effect_req_handler( com_host_handle_t host, com_host_id_t sender_host_id, com_host_msg_t* msg , uint8_t seq )
// {
//     if( !msg->is_ack )
//     {
//         //请求效果器状态
//         com_host_msg_req_voice_effect_data_t* req = (com_host_msg_req_voice_effect_data_t*)msg->data;
//         //获取效果器状态
//         //get_voice_effect( req->effect_type, &param, &enabled );

//         //需要ack则发送ack
//         if(  msg->need_ack )
//         {
//             com_host_msg_req_voice_effect_ack_data_t ack_data;
//             memset(&ack_data,0,sizeof(ack_data));
//             ack_data.result = COM_HOST_MSG_RET_OK; //fill your result here
//             ack_data.effect_type = cmd->effect_type;
//             //fill your effect param and enabled here
//             //ack_data.param = param;
//             //ack_data.enabled = enabled;
//             //发送ack
//             com_host_send_ack( host, sender_host_id, msg, (uint8_t*)&ack_data, sizeof(ack_data), seq );
//         }
//     }
// }

// static void host_run_thread_func(void* arg)
// {
//     com_host_handle_t host = (com_host_handle_t)arg;
//     while(1)
//     {
//         com_host_run(host);
//     }
// }

// int main(void)
// {
//     bsp_init();

//     //创建主机
//     com_host_desc_t host_desc;
//     memset(&host_desc,0,sizeof(host_desc));
//     host_desc.host_id = COM_HOST_ID_MAIN_CONTROLLER;
//     host_desc.ll_depend.malloc = malloc;
//     host_desc.ll_depend.free = free;
//     host_desc.ll_depend.get_time_ms = NULL; //use default
//     com_host_handle_t host = com_host_create(&host_desc);
//     if(host == NULL)
//     {
//         return -1;
//     }

//     //加入端口
//     com_host_port_desc_t port_desc;
//     memset(&port_desc,0,sizeof(port_desc));

//     port_desc.name = "uart1";
//     port_desc.attr = COM_HOST_PORT_ATTR_DEFAULT;
//     port_desc.port_ctx = NULL; //your uart port context
//     port_desc.write = NULL;    //your uart write function
//     port_desc.read = NULL;     //your uart read function
//     if( com_host_add_msg_port(host, &port_desc) != 0 )
//     {
//         com_host_destroy(host);
//         return -1;
//     }

//     //加入路由
//     com_host_add_route(host, COM_HOST_ID_USER_CLIENT, "uart1");
//     com_host_add_route(host, COM_HOST_ID_SUB_CONTROLLER_1, "uart1");

//     //加入消息处理函数
//     com_host_add_msg_handler( host, COM_HOST_MSG_ID_PING , ping_handler );
//     com_host_add_msg_handler( host, COM_HOST_MSG_ID_CMD_REBOOT , reboot_cmd_handler );
//     com_host_add_msg_handler( host, COM_HOST_MSG_ID_CMD_SET_VOICE_EFFECT , voice_effect_cmd_set_handler );
//     com_host_add_msg_handler( host, COM_HOST_MSG_ID_REQ_VOICE_EFFECT , voice_effect_req_handler );
//     //...add other msg handlers

//     pthread_t host_thread;
//     pthread_create( &host_thread, NULL, host_run_thread_func, (void*)host );

//     //主循环
//     while(1)
//     {
//         //ping子控制器
//         com_host_msg_t msg;
//         msg.msg_id = COM_HOST_MSG_ID_PING;
//         msg.data = NULL;
//         msg.size = 0;
//         msg.need_ack = 1;
//         msg.is_ack = 0;
//         com_host_send_msg( host, COM_HOST_ID_SUB_CONTROLLER_1, &msg, NULL );

//         sleep(1000);
//     }
// }
