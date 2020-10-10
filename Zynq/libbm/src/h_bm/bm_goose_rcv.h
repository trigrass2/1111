/*
 * @Author: zhongwei
 * @Date: 2020/4/1 16:11:18
 * @Description: Goose接收处理接口代码
 * @File: bm_goose_rcv.h
 *
*/

#ifndef SRC_H_BAREMETAL_BM_GOOSE_RCV_H_
#define SRC_H_BAREMETAL_BM_GOOSE_RCV_H_

//接收到Goose报文进一步处理接口 
void on_bm_goose_rcv(const TFRAME_GOOSE_RCV * pGoose);

#endif /* SRC_H_BAREMETAL_BM_GOOSE_RCV_H_ */
