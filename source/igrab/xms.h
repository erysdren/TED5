
#ifndef _XMS_H_
#define _XMS_H_

int InitXMS(void);
int XMSAllocate(int32_t size);
unsigned XMSTotalFree(void);	// returns KB free
void XMSFreeMem(int handle);
void XMSmove(int srchandle,int32_t srcoff,int desthandle,int32_t destoff,int32_t size);
void XMSHandleInfo(int handle);

extern unsigned XMSavail;

#endif /* _XMS_H_ */
