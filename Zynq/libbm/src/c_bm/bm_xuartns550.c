
#include <stdio.h>
#include "xil_io.h"
#include "xuartns550.h"
#include "plt_include.h"

int XUartNs550_SetBaudRate(XUartNs550 *InstancePtr, u32 BaudRate);
extern STATUS hd_setInterInt(int intr, Xil_InterruptHandler Handler, uint8 priority,void *CallBackRef);

#if 0
XUartNs550 XUartNs550Instance;
XUartNs550 *UartNs550InstancePtr = &XUartNs550Instance;

char send_ptr[100] = "11 22 33 44 55 !";
char recv_buf[100] = {0};

extern STATUS hd_setInterInt(int intr, Xil_InterruptHandler Handler, uint8 priority,void *CallBackRef);

static volatile int TotalReceivedCount;
static volatile int TotalSentCount;
static volatile int TotalErrorCount;

void UartNs550IntrHandler(void *CallBackRef, u32 Event, unsigned int EventData)
{
	u8 Errors;
	XUartNs550 *UartNs550Ptr = (XUartNs550 *)CallBackRef;
	
	/*
	 * All of the data has been sent.
	 */
	if (Event == XUN_EVENT_SENT_DATA) {
		TotalSentCount = EventData;
	}

	/*
	 * All of the data has been received.
	 */
	if (Event == XUN_EVENT_RECV_DATA) {
		TotalReceivedCount = EventData;
		//XUartNs550_Recv(UartNs550InstancePtr, recv_buf, sizeof(recv_buf));
	}

	/*
	 * Data was received, but not the expected number of bytes, a
	 * timeout just indicates the data stopped for 4 character times.
	 */
	if (Event == XUN_EVENT_RECV_TIMEOUT) {
		TotalReceivedCount = EventData;
		//XUartNs550_Recv(UartNs550InstancePtr, recv_buf, sizeof(recv_buf));
	}

	/*
	 * Data was received with an error, keep the data but determine
	 * what kind of errors occurred.
	 */
	if (Event == XUN_EVENT_RECV_ERROR) {
		TotalReceivedCount = EventData;
		TotalErrorCount++;
		Errors = XUartNs550_GetLastErrors(UartNs550Ptr);
		//XUartNs550_Recv(UartNs550InstancePtr, recv_buf, sizeof(recv_buf));
	}
}

int xuartns550_test()
{
	int Status;
	int sendbyte;
	int recvbyte;
	u16 Options;
	
	Status = XUartNs550_Initialize(UartNs550InstancePtr, XPAR_AXI_UART16550_9_DEVICE_ID);
	if(XST_SUCCESS != Status)
	{
		return Status;
	}

	Status = XUartNs550_SetBaudRate(UartNs550InstancePtr, 230400);
	if (Status != XST_SUCCESS) {
		UartNs550InstancePtr->IsReady = 0;
		return Status;
	}

	hd_setInterInt(XPAR_FABRIC_AXI_UART16550_9_IP2INTC_IRPT_INTR, XUartNs550_InterruptHandler, 1,UartNs550InstancePtr);
	
	//5. 设置Uart中断回调函数
	XUartNs550_SetHandler(UartNs550InstancePtr, UartNs550IntrHandler ,UartNs550InstancePtr);
	
	Options = XUN_OPTION_DATA_INTR | XUN_OPTION_FIFOS_ENABLE;
	XUartNs550_SetOptions(UartNs550InstancePtr, Options);

	//XUartNs550_SetFifoThreshold(UartNs550InstancePtr,XUN_FIFO_TRIGGER_01);
	XUartNs550_GetOptions(UartNs550InstancePtr);

	XUartNs550_Recv(UartNs550InstancePtr, recv_buf, sizeof(recv_buf));
	
	while(1)
	{

		sendbyte = XUartNs550_Send(UartNs550InstancePtr,
									send_ptr,
									strlen(send_ptr));
		printf("uart 0 send %d bytes : %s\r\n", strlen(send_ptr), send_ptr);

		sleep(1);

		memset(recv_buf, 0, sizeof(recv_buf));
		recvbyte = XUartNs550_Recv(UartNs550InstancePtr,
								recv_buf,
								sizeof(recv_buf));

		if (recvbyte > 0)
		{
			printf("uart 0 recv %d bytes : %s  \r\n", recvbyte, recv_buf);
		}

	}

}

void Uart_Send(int phy_no,BYTE *buffer, int len)
{
	int sendbyte = 0;

	sendbyte = XUartNs550_Send(UartNs550InstancePtr,
									buffer,
									len);

	if(sendbyte != len)
	{
		printf("send err %d %d \n",len,sendbyte);
	}
}

void Uart_SendTest()
{
	BYTE buf[20];
	int i;
	uint32 oldtick,newtick;
	
	for(i = 0;i < 20;i++)
		buf[i] = TotalSentCount+i;

	
	oldtick = hd_get_cycles();
	Uart_Send(0,buf,20);
	newtick = hd_get_cycles();
	printf("sendtick  %d \r\n",newtick - oldtick);

}

void Uart_recv()
{
	
}

#else

#define UART_NUM    3
#define RX_BUF_NUM  16 //发送接收全是16字节以内

BYTE RXBUF[UART_NUM][RX_BUF_NUM];

XUartNs550 XUartNs550Instance[UART_NUM];
XUartNs550 *UartNs550InstancePtr = XUartNs550Instance;

static volatile DWORD TotalReceivedCount[UART_NUM];

static volatile DWORD TotalSentCount[UART_NUM];
static volatile DWORD TotalErrorCount[UART_NUM];

static DWORD cycle;

int XUartNs550_RecvBuf(XUartNs550 *InstancePtr, u8 *BufferPtr,
				unsigned int NumBytes)
{
	u32 IerRegister;

	/*
	 * Assert validates the input arguments
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(BufferPtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(((signed)NumBytes) >= 0);

	/*
	 * Enter a critical region by disabling all the UART interrupts to allow
	 * this call to stop a previous operation that may be interrupt driven
	 */
	IerRegister = XUartNs550_ReadReg(InstancePtr->BaseAddress,
						XUN_IER_OFFSET);
	XUartNs550_WriteReg(InstancePtr->BaseAddress, XUN_IER_OFFSET, 0);

	/*
	 * Setup the specified buffer to be received by setting the instance
	 * variables so it can be received with polled or interrupt mode
	 */
	InstancePtr->ReceiveBuffer.RequestedBytes = NumBytes;
	InstancePtr->ReceiveBuffer.RemainingBytes = NumBytes;
	InstancePtr->ReceiveBuffer.NextBytePtr = BufferPtr;

	/*
	 * Restore the interrupt enable register to it's previous value such
	 * that the critical region is exited
	 */
	XUartNs550_WriteReg(InstancePtr->BaseAddress, XUN_IER_OFFSET,
				IerRegister);

	return 0;
}

#define max_cnt  128
DWORD cnt = 0;
DWORD tickcnt[max_cnt][4];
extern DWORD sendtick,sendtick1,sendtick2;

void UartNs550IntrHandler(void *CallBackRef, u32 Event, unsigned int EventData)
{
	u8 Errors;
	int phy_no;
	phy_no = (int )CallBackRef;
	
	XUartNs550 *UartNs550Ptr = UartNs550InstancePtr + phy_no;

	/*
	 * All of the data has been sent.
	 */
	if (Event == XUN_EVENT_SENT_DATA) {
		TotalSentCount[phy_no] += EventData;
	}

	/*
	 * All of the data has been received.
	 */
	if (Event == XUN_EVENT_RECV_DATA) {
		TotalReceivedCount[phy_no] += EventData;
		/*RXBUF EventData cycle   add*/
		tickcnt[cnt][0] = cycle - sendtick;
		tickcnt[cnt][1] = EventData;
		tickcnt[cnt][2] = sendtick1 - sendtick;
		tickcnt[cnt][3] = sendtick2 - sendtick;
		cnt = (cnt + 1) & (max_cnt - 1);
		XUartNs550_RecvBuf(UartNs550Ptr, RXBUF[phy_no], RX_BUF_NUM);
	}

	/*
	 * Data was received, but not the expected number of bytes, a
	 * timeout just indicates the data stopped for 4 character times.
	 */
	if (Event == XUN_EVENT_RECV_TIMEOUT) {
		TotalReceivedCount[phy_no] += EventData;
		/*RXBUF EventData cycle   add*/
		tickcnt[cnt][0] = cycle - sendtick;
		tickcnt[cnt][1] = EventData;
		tickcnt[cnt][2] = sendtick1 - sendtick;
		tickcnt[cnt][3] = sendtick2 - sendtick;
		cnt = (cnt + 1) & (max_cnt - 1);
		XUartNs550_RecvBuf(UartNs550Ptr, RXBUF[phy_no], RX_BUF_NUM);
	}

	/*
	 * Data was received with an error, keep the data but determine
	 * what kind of errors occurred.
	 */
	if (Event == XUN_EVENT_RECV_ERROR) {
		TotalReceivedCount[phy_no] += EventData;
		/*RXBUF EventData cycle   add*/
	
		TotalErrorCount[phy_no]++;
		Errors = XUartNs550_GetLastErrors(UartNs550Ptr);
		XUartNs550_RecvBuf(UartNs550Ptr, RXBUF[phy_no], RX_BUF_NUM);
	}
}

void UartNs550_InterruptHandler(XUartNs550 *InstancePtr)
{
	cycle = hd_get_cycles();
	XUartNs550_InterruptHandler(InstancePtr);
}


int uartns550_int(int phy_no)
{
	int Status;
	u16 Options;
	XUartNs550 *UartNs550Ptr = UartNs550InstancePtr + phy_no;
	XUartNs550Format FormatPtr = {230400, XUN_FORMAT_8_BITS,XUN_FORMAT_ODD_PARITY ,XUN_FORMAT_1_STOP_BIT};
	
	int intr[3] = {XPAR_FABRIC_AXI_UART16550_9_IP2INTC_IRPT_INTR,XPAR_FABRIC_AXI_UART16550_10_IP2INTC_IRPT_INTR,XPAR_FABRIC_AXI_UART16550_11_IP2INTC_IRPT_INTR};
	u16 devid[3] = {XPAR_AXI_UART16550_9_DEVICE_ID,XPAR_AXI_UART16550_10_DEVICE_ID,XPAR_AXI_UART16550_11_DEVICE_ID};
	
	Status = XUartNs550_Initialize(UartNs550Ptr, devid[phy_no]);
	if(XST_SUCCESS != Status)
	{
		return Status;
	}

	Status = XUartNs550_SetBaudRate(UartNs550Ptr, 230400);
	if (Status != XST_SUCCESS) {
		UartNs550Ptr->IsReady = 0;
		return Status;
	}
 
	XUartNs550_SetDataFormat(UartNs550Ptr,&FormatPtr);
	
	hd_setInterInt(intr[phy_no], UartNs550_InterruptHandler, 1,UartNs550Ptr); //优先级设置最高 ，不然接收会进入timeout中断
	
	//5. 设置Uart中断回调函数
	XUartNs550_SetHandler(UartNs550Ptr, UartNs550IntrHandler ,phy_no);
	
	Options = XUN_OPTION_DATA_INTR | XUN_OPTION_FIFOS_ENABLE;
	XUartNs550_SetOptions(UartNs550Ptr, Options);

	//XUartNs550_SetFifoThreshold(UartNs550InstancePtr,XUN_FIFO_TRIGGER_01);

	
	XUartNs550_Recv(UartNs550Ptr, RXBUF[phy_no], sizeof(RX_BUF_NUM));
}

void Uart_Send(int phy_no,BYTE *buffer, int len)
{
	int sendbyte = 0;
	XUartNs550 *UartNs550Ptr = UartNs550InstancePtr + phy_no;

	sendbyte = XUartNs550_Send(UartNs550Ptr,
									buffer,
									len);
	
	//多了不一定相等，发送中断会有
	if(sendbyte != len)
	{
		printf("send err %d %d \n",len,sendbyte);
	}
}

void Uart_SendTest(int phy_no)
{
	BYTE buf[RX_BUF_NUM];
	int i;
	uint32 oldtick,newtick;

	TotalSentCount[phy_no] = 0;
	for(i = 0;i < RX_BUF_NUM;i++)
		buf[i] = TotalSentCount[phy_no] + i;

	oldtick = hd_get_cycles();
	Uart_Send(phy_no,buf,RX_BUF_NUM);
	newtick = hd_get_cycles();
	printf("sendtick  %ul \r\n",newtick - oldtick);

	usleep(200000);


	printf("recv  %d  send %d \r\n",TotalSentCount[phy_no],TotalReceivedCount[phy_no]);

	TotalReceivedCount[phy_no] = 0;
}

void Uart_recv()
{
	
}



#endif





