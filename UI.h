/*
 * UI.h
 *
 *  Created on: 2019年1月19日
 *      Author: yan
 */

#ifndef STARTUP_UI_H_
#define STARTUP_UI_H_

#define UI_ALARM_SUCCESS                0
#define UI_NO_RESOURCES                1//未分配足够空间
#define UI_NO_MSG                       2//未收到Message
#define UI_NO_ALARM                     3//当前不存在报警
#define UI_CANCEL_ALARM                 4//取消当前报警

typedef enum
{
    UI_ALARM_LEVEL_1=1,
    UI_ALARM_LEVEL_2,
    UI_ALARM_LEVEL_3,
    UI_ALARM_CANCEL
}UI_Event_e;
typedef struct
{
    Queue_Elem _elem;           //!< New queue element to contain write evt info
    UI_Event_e  event;          //!< Event that occurred
//    uint16_t connHandle;        //!< Connection event was received on
//    uint16_t len;               //!< Length of data received
} UI_ALARM_MSG_t;

extern void UI_CreatTask();


#endif /* STARTUP_UI_H_ */
