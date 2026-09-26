#include "ComHost.hpp"
#include "com_host/com_host.h"
#include "com_host/private/com_host_port.h"
#include "co/all.h"
#include "co/time.h"
#include <list>

#include <stdio.h>
#define INFO_LOG(...)   //do{ printf("[ComHost] "); printf("INFO: "); printf(__VA_ARGS__);printf("\r\n"); }while(0);
#define WARN_LOG(...)   //do{ printf("[ComHost] "); printf("WARN: "); printf(__VA_ARGS__);printf("\r\n"); }while(0);
#define ERROR_LOG(...)  //do{ printf("[ComHost] "); printf("ERROR: "); printf(__VA_ARGS__);printf("\r\n"); }while(0);

static uint8_t com_host_lib_is_init = 0;
static int64_t g_com_host_start_ms = 0; // 程序/库初始化时的单调时钟起点

/**
 * @brief 获取自程序启动（com_host 初始化）起经过的毫秒数
 * @return 毫秒数
*/
static uint32_t com_host_get_time_ms()
{
    return (uint32_t)(co::mono_time.ms() - g_com_host_start_ms);
}

static void com_host_lib_init()
{
    if( !com_host_lib_is_init )
    {
        g_com_host_start_ms = co::mono_time.ms();

        com_host_ll_depend_t ll_depend;
        ll_depend.get_time_ms = []()->uint32_t{ return (uint32_t)(co::mono_time.ms() - g_com_host_start_ms); };
        ll_depend.stack_buff = new uint8_t[ 4 * 1024 * 1024 ];
        ll_depend.stack_buff_size = 4 * 1024 * 1024;
        ll_depend.stack_align = 16; // x86 GCC推荐使用16字节对齐

        com_host_init( &ll_depend );
        com_host_lib_is_init = 1;
    }
}

// 用于保存端口信息的类
class ComPort{
public:
    std::function<uint32_t(uint8_t* data , uint32_t len)> writeFunc;
    std::function<uint32_t(uint8_t* data , uint32_t len)> readFunc;
    std::string name;
    com_host_port_handle_t handle = nullptr;
};

// 用于等待应答的类
class ComWaitAck{
public:
    ComWaitAck( com_host_id_t target ,
                com_host_msg_id_t msgId ,
                uint32_t timeoutMs)
    {
        this->targetId = target;
        this->msgId = msgId;
        this->seq = seq;
        this->timeoutMs = timeoutMs;
        this->isAcked = false;
        this->ackData = nullptr;
        this->ackDataLen = 0;
        memset(&msg,0,sizeof(msg));
    }
    ~ComWaitAck()
    {
        if( ackData ) delete[] ackData;
        ackData = nullptr;
        ackDataLen = 0;
    }

    com_host_id_t getTargetId() const { return targetId; }
    com_host_msg_id_t getMsgId() const { return msgId; }
    uint8_t getSeq() const { return seq; }

    void setAcked( com_host_msg_t* msg ) {
        co::mutex_guard guard(mutex);
        if( isAcked ) return; // 已收到则不处理
        if( msg )
        {
            if( ackData ) delete[] ackData;
            ackData = new uint8_t[msg->data_len];
            if( ackData )
            {
                memcpy( ackData , msg->data , msg->data_len );
                ackDataLen = msg->data_len;
            }
            this->msg.data = ackData;
            this->msg.data_len = ackDataLen;
        }
        isAcked = true;
    }

    com_host_msg_t* waitAck( uint32_t timeoutMs , uint8_t seq ) {
        this->seq = seq;
        uint32_t startTime = com_host_get_time_ms();
        while(1)
        {
            mutex.lock();
            if( isAcked ) {
                mutex.unlock();
                return &msg;
            }
            mutex.unlock();
            if( timeoutMs > 0 && com_host_get_time_ms() - startTime >= timeoutMs )
            {
                return nullptr; // 超时返回空
            }
            co::sleep(1);
        }
        return nullptr;
    }

private:
    com_host_id_t targetId = 0;
    com_host_msg_id_t msgId = 0;
    uint8_t seq = 0;
    uint32_t timeoutMs = 200; // 默认200毫秒超时

    com_host_msg_t msg;

    bool isAcked = false; // 是否已收到应答
    uint8_t* ackData = nullptr;
    uint16_t ackDataLen = 0;

    co::mutex mutex;
};

// 用于等待消息的类
class ComWaitMsg{
public:
    ComWaitMsg( com_host_msg_id_t id ){
        msgId = id;
        memset(&msg,0,sizeof(msg));
    }
    ~ComWaitMsg() {
        if( pData ) delete[] pData;
    }

    com_host_msg_id_t getMsgId() const { return msgId; }
    void setReceived( com_host_id_t senderId , com_host_msg_t* msg ) {
        co::mutex_guard guard(mutex);
        if( isReceived ) return; // 已收到则不处理
        this->senderId = senderId;
        if( msg->data_len > 0 && msg->data )
        {
            if( pData ) delete[] pData;
            pData = new uint8_t[ msg->data_len ];
            if( pData )
            {
                memcpy( pData , msg->data , msg->data_len );
                pDataLen = msg->data_len;
            }
        }
        this->msg = *msg;
        isReceived = 1;
    }

    com_host_id_t getSenderId() const { return senderId; }

    com_host_msg_t* wait( uint32_t timeoutMs ) {
        uint32_t startTime = com_host_get_time_ms();
        while(1)
        {
            {
                co::mutex_guard guard(mutex);
                if( isReceived ) return &msg;
            }
            if( timeoutMs > 0 && com_host_get_time_ms() - startTime >= timeoutMs )
            {
                return nullptr; // 超时返回空
            }
            co::sleep(1);
        }
        return nullptr;
    }

private:
    com_host_msg_id_t msgId = 0;
    com_host_id_t senderId = 0;
    com_host_msg_t msg;
    uint8_t* pData = nullptr;
    uint16_t pDataLen = 0;
    uint8_t isReceived = 0; // 是否已收到
    co::mutex mutex;
};

class ComWaits{
public:
    std::list<ComWaitMsg*> waitMsgs;
    std::list<ComWaitAck*> waitAcks;
};

class ComHost::Impl{
public:
    co::mutex mutex; // 保护成员变量

    com_host_handle_t com_host = nullptr;
    bool isRunning = false;
    bool shouldStop = false;   // 停止信号标志
    
    std::list<ComPort*> ports;

    ComWaits wait;
};

static void com_host_msg_default_handler(  com_host_handle_t host,
                                           com_host_port_handle_t port,
                                           com_host_id_t sender_host_id,
                                           com_host_msg_t* msg ,
                                           uint8_t seq,
                                           void* cb_ctx )
{
    (void)host;
    (void)port;
    auto wait = (ComWaits*)cb_ctx;
    if( !wait )
    {
        ERROR_LOG("com_host_msg_default_handler wait is null.");
        return;
    }

    INFO_LOG("ComHost received msg id 0x%04X from senderId 0x%02X.", msg->msg_id , sender_host_id);

    //遍历等待列表，看看有没有等待的消息
    for( auto waitMsg : wait->waitMsgs )
    {
        if( waitMsg->getMsgId() == msg->msg_id )
        {
            waitMsg->setReceived( sender_host_id , msg );
        }
    }

    //遍历等待应答列表，看看有没有等待的应答
    for( auto waitAck : wait->waitAcks )
    {
        if( waitAck->getMsgId() == msg->msg_id &&
            waitAck->getTargetId() == sender_host_id &&
            waitAck->getSeq() == seq )
        {
            waitAck->setAcked( msg );
        }
    }
}


ComHost::ComHost( com_host_id_t selfId )
{
    pImpl = new Impl();

    com_host_lib_init();

    //创建com_host
    com_host_desc_t desc;
    desc.host_id = selfId;
    pImpl->com_host = com_host_create( &desc );

    com_host_add_default_msg_handler( pImpl->com_host , com_host_msg_default_handler , &pImpl->wait );
}
 
ComHost::~ComHost()
{
    Stop();
    com_host_destroy( pImpl->com_host );
    for(  auto port : pImpl->ports )
    {
        INFO_LOG("Delete ComPort %s.", port->name.c_str());
        delete port;
    }
    delete pImpl;
}

void ComHost::Start(uint32_t freq)
{
    co::mutex_guard guard(pImpl->mutex);
    
    if( pImpl->isRunning )
    {
        WARN_LOG("ComHost is already running.");
        return;
    }
    
    pImpl->isRunning = true;
    pImpl->shouldStop = false;
    
    co::go(
        [this, freq]()
        {
            uint32_t cycle_ms = freq == 0 ? 0 : 1000 / freq;
            INFO_LOG("ComHost coroutine start , frq:%d hz" , freq );
            
            while (1)
            {
                co::mutex_guard guard(pImpl->mutex);

                if( pImpl->shouldStop )
                {
                    break;
                }

                com_host_run( pImpl->com_host );
                co::sleep( cycle_ms );
            }
            
            co::mutex_guard guard(pImpl->mutex);
            // 协程退出时设置运行状态
            pImpl->isRunning = false;
            pImpl->shouldStop = false;
            INFO_LOG("ComHost coroutine stopped.");
        }
    );
}

void ComHost::Stop()
{
    pImpl->mutex.lock();

    if( !pImpl->isRunning )
    {
        pImpl->mutex.unlock();
        WARN_LOG("ComHost is not running,cannot stop.");
        return;
    }

    pImpl->shouldStop = true;
    pImpl->mutex.unlock();

    // 等待协程停止
    while( true )
    {
        co::mutex_guard guard(pImpl->mutex);
        if( !pImpl->isRunning )
        {
            break;
        }
        co::sleep(1);
    }

    INFO_LOG("ComHost stopped.");
}

extern "C" uint32_t write_wrapper( void* opt ,  const uint8_t* data , uint32_t len )
{
    if( !opt )
        return 0;

    auto port = (ComPort*)opt;
    return port->writeFunc((uint8_t*)data, len);
}

extern "C" uint32_t read_wrapper( void* opt ,  uint8_t* data , uint32_t len )
{
    if( !opt )
        return 0;

    auto port = (ComPort*)opt;
    return port->readFunc(data, len);
}

void ComHost::AddComPort( std::function<uint32_t(uint8_t* data , uint32_t len)> writeFunc ,
                       std::function<uint32_t(uint8_t* data , uint32_t len)> readFunc ,
                       const char* name ,
                       uint32_t unpackBufSize ,
                       uint32_t packBufSize )
{
    co::mutex_guard guard(pImpl->mutex);

    if( pImpl->isRunning )
    {
        WARN_LOG("ComHost is running,cannot add port.");
        return;
    }

    //创建ComHost层的端口对象
    ComPort* port = new ComPort();
    port->name = name;
    port->writeFunc = writeFunc;
    port->readFunc = readFunc;

    //添加端口到ComHost
    com_host_port_desc_t port_desc;
    memset( &port_desc , 0 , sizeof(port_desc) );
    port_desc.name = port->name.c_str();
    port_desc.write = write_wrapper;
    port_desc.read = read_wrapper;
    port_desc.attr = COM_HOST_PORT_ATTR_DEFAULT;
    port_desc.opt_ctx = port; // 传递端口对象作为上下文
    port_desc.unpack_buf_size = unpackBufSize;
    port_desc.pack_buf_size = packBufSize;

    port->handle = com_host_add_port( pImpl->com_host , &port_desc );
    if( !port->handle )
    {
        delete port;
        ERROR_LOG("ComHost add port %s failed.", name);
        return;
    }

    //添加到端口列表
    pImpl->ports.push_back(port);

    INFO_LOG("ComHost add port %s success.", name);
}

void ComHost::AddRoute( com_host_id_t targetId , const char* portName )
{
    co::mutex_guard guard(pImpl->mutex);

    //查找端口
    ComPort* port = nullptr;
    for( auto p : pImpl->ports )
    {
        if( strcmp( p->name.c_str() , portName ) == 0 )
        {
            port = p;
            break;
        }
    }
    if( !port )
    {
        ERROR_LOG("ComHost add route failed, port \"%s\" not found.", portName);
        return;
    }
    if( !port->handle )
    {
        ERROR_LOG("ComHost add route failed, port \"%s\" handle is null.", portName);
        return;
    }

    //添加路由
    if( com_host_add_route( pImpl->com_host , targetId , port->handle ) != 0 )
    {
        ERROR_LOG("ComHost add route to targetId 0x%02X via port \"%s\" failed.", targetId, portName);
        return;
    }

    INFO_LOG("ComHost add route to targetId 0x%02X via port \"%s\" success.", targetId, portName);
}

void ComHost::SendMsg( com_host_id_t targetId , com_host_msg_t* msg , uint8_t msg_cnt )
{
    co::mutex_guard guard(pImpl->mutex);

    if( msg_cnt == 1 )
    {
        com_host_send_msg( pImpl->com_host , targetId , msg , nullptr );
    }
    else if( msg_cnt != 0 )
    {
        com_host_send_msgs( pImpl->com_host , targetId , msg , msg_cnt );
    }

    INFO_LOG("ComHost send msg id 0x%04X to targetId 0x%02X.", msg->msg_id , targetId);
}

void ComHost::SendData(com_host_id_t targetId , uint8_t* data , uint32_t len)
{
    co::mutex_guard guard(pImpl->mutex);

    com_host_target_direct_write( pImpl->com_host , targetId , data , len );

    INFO_LOG("ComHost send data to targetId 0x%02X, len=%d.", targetId , len);
}

ComHost::Result ComHost::SendMsgAndWaitAck(  com_host_id_t targetId,
                            com_host_msg_t* msg,
                            std::function<void(uint8_t* ackData , uint16_t dataLen , Result result)> retCb,
                            uint32_t timeoutMs,
                            uint32_t retryTimes)
{
    Result result = COM_TIMEOUT;
    //放入栈空间,自动释放
    ComWaitAck waitAck( targetId , msg->msg_id  , timeoutMs );

    pImpl->mutex.lock();
    pImpl->wait.waitAcks.push_back(&waitAck);
    pImpl->mutex.unlock();

    msg->need_ack = 1; // 设置需要应答
    for( uint32_t i = 0 ; i < retryTimes ; i++ )
    {
        //发送消息
        uint8_t seq = 0;
        INFO_LOG("ComHost send msg id 0x%04X to targetId 0x%02X, try %d/%d.", msg->msg_id , targetId , i+1 , retryTimes);
        com_host_send_msg( pImpl->com_host , targetId , msg , &seq );

        //等待应答
        auto ackMsg = waitAck.waitAck( timeoutMs , seq );
        if( ackMsg ) // 收到应答
        {
            result = COM_SUCCEEDED;
            if( retCb )
            {
                retCb( ackMsg->data , ackMsg->data_len , result);
            }
            INFO_LOG("ComHost received ack for msg id 0x%04X from targetId 0x%02X.", msg->msg_id , targetId);
            break;
        }
        else // 超时
        {
            result = COM_TIMEOUT;
            if( i == retryTimes - 1 ) // 最后一次重试仍然超时
            {
                if( retCb )
                {
                    retCb( nullptr , 0 , result);
                }
            }
            WARN_LOG("ComHost wait ack for msg id 0x%04X to targetId 0x%02X timeout, try %d/%d.", msg->msg_id , targetId , i+1 , retryTimes);
        }
    }

    //删除等待对象
    pImpl->wait.waitAcks.remove(&waitAck);

    return result;
}

ComHost::ComMsgRsp* AddMsgRsp( com_host_msg_id_t msgId ,
                               std::function<void(com_host_id_t senderId, com_host_msg_t* msg , ComHost::Result result)> cb )
{
    return nullptr;
}

void ComHost::RemoveMsgRsp( ComMsgRsp* rsp )
{
    (void)rsp;
}

ComHost::Result ComHost::WaitMsg( com_host_msg_id_t msgId ,
                  std::function<void(com_host_id_t senderId, com_host_msg_t* msg , Result result)> cb,
                  uint32_t timeoutMs)
{
    Result result = COM_TIMEOUT;

    //放入栈空间,自动释放
    ComWaitMsg waitMsg(msgId);

    pImpl->mutex.lock();
    pImpl->wait.waitMsgs.push_back(&waitMsg);
    pImpl->mutex.unlock();

    //等待消息
    com_host_msg_t* msg = waitMsg.wait( timeoutMs );

    //处理回调
    if( msg )
    {
        result = COM_SUCCEEDED;
        cb( waitMsg.getSenderId(), msg , result);
    }
    else
    {
        result = COM_TIMEOUT;
        cb(0, nullptr , result);
    }
    //删除等待对象
    pImpl->mutex.lock();
    pImpl->wait.waitMsgs.remove(&waitMsg);
    pImpl->mutex.unlock();

    return result;
}

void ComHost::SendMsgPkgByPort(const char* portName,
                          com_host_id_t senderId,
                          com_host_id_t targetId,
                          com_host_msg_id_t msgId,
                          uint8_t* data,
                          uint16_t len,
                          bool needAck,
                          bool isAck
                        )
{
    co::mutex_guard guard(pImpl->mutex);

    //查找端口
    ComPort* port = nullptr;
    for( auto p : pImpl->ports )
    {
        if( strcmp( p->name.c_str() , portName ) == 0 )
        {
            port = p;
            break;
        }
    }
    if( !port )
    {
        ERROR_LOG("ComHost send msg pkg by port failed, port \"%s\" not found.", portName);
        return;
    }
    if( !port->handle )
    {
        ERROR_LOG("ComHost send msg pkg by port failed, port \"%s\" handle is null.", portName);
        return;
    }

    //构造消息
    com_host_msg_t msg;
    memset( &msg , 0 , sizeof(msg) );
    msg.msg_id = msgId;
    msg.data = data;
    msg.data_len = len;
    msg.need_ack = needAck;
    msg.is_ack = isAck;

    //构造消息包
    msg_package_t msg_pkg;
    memset( &msg_pkg , 0 , sizeof(msg_pkg) );
    msg_pkg.msg = &msg;
    msg_pkg.sender_host_id = senderId;
    msg_pkg.target_host_id = targetId;
    msg_pkg.seq = 0; // 序列号由底层自动处理
    msg_pkg.crc16 = 0; // CRC16由底层自动处理

    //发送消息包
    com_host_port_send_msg_package( port->handle , &msg_pkg );
}

void ComHost::SendMsgPkgByPort( com_host_id_t senderId,
                                com_host_id_t targetId,
                                com_host_msg_t* msg,
                                const char* portName)
{
    co::mutex_guard guard(pImpl->mutex);

    //查找端口
    ComPort* port = nullptr;
    for( auto p : pImpl->ports )
    {
        if( strcmp( p->name.c_str() , portName ) == 0 )
        {
            port = p;
            break;
        }
    }
    if( !port )
    {
        ERROR_LOG("ComHost send msg pkg by port failed, port \"%s\" not found.", portName);
        return;
    }
    if( !port->handle )
    {
        ERROR_LOG("ComHost send msg pkg by port failed, port \"%s\" handle is null.", portName);
        return;
    }

    //构造消息包
    msg_package_t msg_pkg;
    memset( &msg_pkg , 0 , sizeof(msg_pkg) );
    msg_pkg.msg = msg;
    msg_pkg.sender_host_id = senderId;
    msg_pkg.target_host_id = targetId;
    msg_pkg.seq = 0; // 序列号由底层自动处理
    msg_pkg.crc16 = 0; // CRC16由底层自动处理

    //发送消息包
    com_host_port_send_msg_package( port->handle , &msg_pkg );
}
