  
#ifndef __CM3_H
#define __CM3_H

#define CANOPEN_SLAVE_NODE_MAX          ( 0x7F )
#define CANOPEN_SLAVE_NODE_MIN          ( 0x1 )
#define CANOPEN_SLAVE_NODE_ID_DEFAULT   ( 0x0A )

#define BSP_MLSS_ADRESS                 (uint16_t)0x7E5//LSS协议主机COBID
#define BSP_SLSS_ADRESS                 (uint16_t)0x7E4//LSS协议客户机COBID

#define BSP_LSS_CMD_MODE                0x04//模式切换
#define BSP_LSS_CMD_ID_SET              0x11//节点ID设置
#define BSP_LSS_CMD_BAND_SET            0x13//波特率设置
#define BSP_LSS_CMD_SAVE                0x17//参数保存

#define BSP_LSS_IDENT_REMOTE_NON_CONF   0x4C//总线查询
#define BSP_LSS_IDENT_NON_CONF_SLAVE    0x50//总线查询回复
#define BSP_LSS_CMD_ID_INQUIRY          0x5E//询问节点ID号


void TIM3_IRQHandler(void);


#endif 

