#pragma once

#include "com_host/com_host.h"
#include <functional>

class ComHost
{
public:
    typedef enum{
        COM_SUCCEEDED = 0,
        COM_TIMEOUT,
        COM_FAILED
    }Result;

public:
    ComHost( com_host_id_t selfId = COM_HOST_ID_ADMIN );
    virtual ~ComHost();

    /**
     * @brief 启动ComHost
     * @param freq 心跳频率，单位Hz，默认50Hz
    */
    void Start(uint32_t freq = 50);

    /**
     * @brief 停止ComHost
     * @note 停止后可以再次调用Start重新启动,阻塞直到停止完成
    */
    void Stop();

    /**
     * @brief 添加端口
     * @param writeFunc 写函数
     * @param readFunc 读函数
     * @param name 端口名称
     * @param unpackBufSize 解包缓冲区大小
     * @param packBufSize 打包缓冲区大小
    */
    void AddComPort( std::function<uint32_t(uint8_t* data , uint32_t len)> writeFunc ,
                     std::function<uint32_t(uint8_t* data , uint32_t len)> readFunc ,
                     const char* name ,
                     uint32_t unpackBufSize = 2048 ,
                     uint32_t packBufSize = 2048 );
    
    /**
     * @brief 添加路由
     * @param targetId 目标ID
     * @param portName 端口名称
    */
    void AddRoute( com_host_id_t targetId , const char* portName );

    /**
     * @brief 发送消息给指定目标
     * @param targetId 目标ID
     * @param msg 消息指针
     * @param msg_cnt 消息个数
    */
    void SendMsg(com_host_id_t targetId , com_host_msg_t* msg , uint8_t msg_cnt = 1);

    /**
     * @brief 发送原始数据给指定目标
     * @param targetId 目标ID
     * @param data 数据指针
     * @note 无法多级路由转发
    */
    void SendData(com_host_id_t targetId , uint8_t* data , uint32_t len);

    /**
     * @brief 发送消息并等待应答
     * @param targetId 目标ID
     * @param msg 消息指针
     * @param retCb 回调函数,用于处理收到的应答消息
     * @param timeoutMs 单次等待应答超时时间，单位毫秒，默认200ms
     * @param retryTimes 重试次数，默认3次
     * @return 发送结果 (Result)
    */
    Result SendMsgAndWaitAck( com_host_id_t targetId,
                            com_host_msg_t* msg,
                            std::function<void(uint8_t* ackData , uint16_t dataLen , Result result)> retCb = nullptr,
                            uint32_t timeoutMs = 200,
                            uint32_t retryTimes = 3);


    class ComMsgRsp;
    
    /**
     * @brief 添加消息响应回调
     * @param msgId 消息ID
     * @param cb 回调函数
     * @return 消息响应对象指针 (ComMsgRsp*)
     */
    ComMsgRsp* AddMsgRsp( com_host_msg_id_t msgId ,
                            std::function<void(com_host_id_t senderId, com_host_msg_t* msg , Result result)> cb );

    /**
     * @brief 移除消息响应回调
     * @param rsp 消息响应对象指针
     */
    void RemoveMsgRsp( ComMsgRsp* rsp );

    /**
     * @brief 等待指定消息
     * @param msgId 消息ID
     * @param cb 回调函数
     * @param timeoutMs 超时时间，单位毫秒，默认0表示无限等待
     * @return 结果 (Result)
    */
    Result WaitMsg( com_host_msg_id_t msgId ,
                  std::function<void(com_host_id_t senderId, com_host_msg_t* msg , Result result)> cb,
                  uint32_t timeoutMs = 0 ); // timeoutMs=0表示无限等待

    /**
     * @brief 指定端口发送消息包
     * @param portName 端口名称
     * @param senderId 消息包发送者ID字段
     * @param targetId 消息包目标ID字段
     * @param msgId 消息包消息ID字段
     * @param data 消息包数据指针
     * @param len 消息包数据长度
     * @param needAck 是否需要应答字段
     * @param isAck 是否为应答包字段
     * @note 该函数不会通过路由发送消息，而是直接通过指定端口发送消息包
    */
    void SendMsgPkgByPort(const char* portName,
                          com_host_id_t senderId,
                          com_host_id_t targetId,
                          com_host_msg_id_t msgId,
                          uint8_t* data = nullptr,
                          uint16_t len = 0,
                          bool needAck = false,
                          bool isAck = false
                        );
    void SendMsgPkgByPort(com_host_id_t senderId,
                          com_host_id_t targetId,
                          com_host_msg_t* msg,
                          const char* portName
                        );
private:
    class Impl;
    Impl* pImpl;
};

