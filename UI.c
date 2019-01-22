/*
 * UI.c
 *
 *  Created on: 2019年1月19日
 *      Author: yan
 */
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Event.h>
#include <ti/sysbios/knl/Queue.h>
#include <string.h>
#include <Board.h>
#include "UI.h"
#include "Application/util.h"
#include <ti/drivers/GPIO.h>
#include <stdbool.h>
#include <icall.h>

//typedef enum
//{
//    UI_ALARM_LEVEL_1,
//    UI_ALARM_LEVEL_2,
//    UI_ALARM_LEVEL_3,
//    UI_ALARM_CANCEL
//}UI_Event_e;
//typedef struct
//{
//    Queue_Elem _elem;           //!< New queue element to contain write evt info
//    UI_Event_e  event;          //!< Event that occurred
//    uint16_t connHandle;        //!< Connection event was received on
//    uint16_t len;               //!< Length of data received
//} UI_ALARM_MSG_t;

//static void UI_CreatTask();

//static bool state=false;
//static uint8_t level=0x00;

static void UI_taskFxn();
static void UI_Test();
static void UI_init();
void Clock1_Configure();
void clk1_CallBack();
void Clock2_Configure();
void clk2_CallBack();
void Clock3_Configure();
void clk3_CallBack();
//uint8_t UI_EnqueueMsg(UI_Event_e event, uint16_t connHandle, uint16_t len);
Queue_Handle UI_constructQueue(Queue_Struct *pQueue);
uint8_t UI_EnqueueMsg(Queue_Handle hUIQ,UI_Event_e event);
uint8_t UI_ProcessMSG();

// Task configuration
#define UI_TASK_PRIORITY                     5

#ifndef UI_TASK_STACK_SIZE
#define UI_TASK_STACK_SIZE                   512
#endif

static Queue_Struct UIQ;
static Queue_Handle hUIQ;

Task_Struct UITask;
#if defined __TI_COMPILER_VERSION__
#pragma DATA_ALIGN(appTaskStack, 8)
#else
#pragma data_alignment=8
#endif
uint8_t appTaskStack[UI_TASK_STACK_SIZE];
Clock_Struct clk1Struct, clk2Struct,clk3Struct;
Clock_Handle clk1Handle,clk2Handle,clk3Handle;

void UI_CreatTask()
{
    Task_Params taskParams;

    // Configure task
    Task_Params_init(&taskParams);
    taskParams.stack = appTaskStack;
    taskParams.stackSize = UI_TASK_STACK_SIZE;
    taskParams.priority = UI_TASK_PRIORITY;

    Task_construct(&UITask, UI_taskFxn, &taskParams, NULL);
}
void UI_taskFxn()
{
    UI_init();
    UI_Test();
}
void UI_init()
{
//    state=false;
//    level=0x00;
    GPIO_init();
    GPIO_setConfig(Board_UI_LED_1, GPIO_CFG_OUTPUT| GPIO_CFG_OUT_LOW);//red LED
    GPIO_setConfig(Board_UI_LED_2 , GPIO_CFG_OUTPUT| GPIO_CFG_OUT_LOW);//Green LEd
    GPIO_setConfig(Board_UI_LED_3 , GPIO_CFG_OUTPUT| GPIO_CFG_OUT_LOW);//Yellow LED
    GPIO_setConfig(Board_UI_BUZZ , GPIO_CFG_OUTPUT| GPIO_CFG_OUT_LOW);// Buzz
//    // Construct the OAD Queue
//    Queue_construct(UIQ, NULL);
//
//    // Get the handle to the newly constructed Queue
//    hUIQ = Queue_handle(UIQ);
}
void UI_Test()
{
    uint8_t states;
    /*配置clock用来定时产生中断*/
    Clock1_Configure();
    Clock2_Configure();
    Clock3_Configure();
//        Clock_start(clk1Handle);
        hUIQ=UI_constructQueue(&UIQ);
        states=UI_EnqueueMsg(hUIQ,UI_ALARM_LEVEL_2);
        if(states==UI_ALARM_SUCCESS)
        {
            UI_ProcessMSG();
        }

//        else
//        {
//
//        }
}
Queue_Handle UI_constructQueue(Queue_Struct *pQueue)
{
  // Construct a Queue instance.
  Queue_construct(pQueue, NULL);

  return Queue_handle(pQueue);
}

//uint8_t UI_EnqueueMsg(UI_Event_e event, uint16_t connHandle,uint16_t len)
uint8_t UI_EnqueueMsg(Queue_Handle hUIQ,UI_Event_e event)
{
    uint8_t status = UI_ALARM_SUCCESS;

    UI_ALARM_MSG_t *UIEvt =ICall_malloc( sizeof(UI_ALARM_MSG_t) );
//    UI_ALARM_MSG_t *UIEvt;
    if ( UIEvt != NULL )
    {
        UIEvt->event = event;
//        UIEvt->connHandle = connHandle;
//        UIEvt->len = len;

        Queue_put(hUIQ, &UIEvt->_elem);
    }
    else
    {
        status = UI_NO_RESOURCES;
    }

    return (status);
}
uint8_t UI_ProcessMSG()
{
    uint8_t state=0;
    uint8_t level=0x00;
    uint8_t states=UI_ALARM_SUCCESS;
//    Queue_Elem *elem;
//    UI_Event_e *Evt=NULL;
    UI_Event_e *Evt;
//    UI_ALARM_MSG_t *UIMSG;
    UI_ALARM_MSG_t *UIMSG = Queue_get(hUIQ);
//    while(UIMSG!=(UI_ALARM_MSG_t *)hUIQ)
//    {
//        if(UIMSG!=NULL)
//        {
    *Evt=UIMSG->event;
//            *elem=UIMSG->_elem;
    ICall_free(UIMSG);
            switch(*Evt)
            {
            case UI_ALARM_LEVEL_1:

                if(state==0)
                {
//                    state=true;
//                    level=0x01;
//                    states=UI_ALARM_SUCCESS;
                    level=0x01;
                    state=1;
                    Clock_start(clk1Handle);
//                    Queue_remove(elem);
//                    ICall_free(UIMSG->event);
                }
                else
                {
                    states=UI_ALARM_SUCCESS;
                }
                break;

            case UI_ALARM_LEVEL_2:
                if(state==0)
                {
                    Clock_start(clk2Handle);
                    state=true;
                    level=0x02;
                    states=UI_ALARM_SUCCESS;
                }
                else
                {
                    if(state==1&&level==0x01)
                    {
                        Clock_stop(clk1Handle);
                        Clock_start(clk2Handle);
                        level=0x02;
                        states=UI_ALARM_SUCCESS;
                    }
                    else
                    {
                        states=UI_ALARM_SUCCESS;
                    }
                }


                break;
            case UI_ALARM_LEVEL_3:
                if(state==0)
                {
                    Clock_start(clk3Handle);
                    state=1;
                    level=0x03;
                    states=UI_ALARM_SUCCESS;
                }

                else
                {
                    if(state==1&&level==0x01)
                    {
                        Clock_stop(clk1Handle);
                        Clock_start(clk3Handle);
                        level=0x03;
                        states=UI_ALARM_SUCCESS;
                    }
                    else if(state==1&&level==0x02)
                    {
                        Clock_stop(clk2Handle);
                        Clock_start(clk3Handle);
                        level=0x03;
                        states=UI_ALARM_SUCCESS;
                    }
                    else
                    {
                        states=UI_ALARM_SUCCESS;
                    }
                }
                break;
            case UI_ALARM_CANCEL:
                if(state==0)
                {
                    states=UI_NO_ALARM;
                }
                else if(state==1&&level==0x01)
                {
                    Clock_stop(clk1Handle);
                    state=0;
                    level=0x00;
                }
                else if(state==1&&level==0x02)
                {
                    Clock_stop(clk2Handle);
                    state=0;
                    level=0x00;
                }
                else
                {
                    Clock_stop(clk3Handle);
                    state=0;
                    level=0x00;
                }
                states=UI_CANCEL_ALARM;
                break;
            default :
                break;
//                ICall_free(UI_ALARM_MSG_t);
            }

//        }
//        else
//        {
//            states=UI_NO_MSG;
//            return(states);
//        }
//        ICall_free(UIMSG);
//    }
//    ICall_free(UIMSG);
    return(states);
}
/*
 * confige clock and set clock callback
 * clock1-------------alarm1
 * clock2-------------alarm2
 * clock3-------------alarm3
 */
void Clock1_Configure()
{
    Clock_Params clkParams;
    Clock_Params_init(&clkParams);
    clkParams.period = 500000/Clock_tickPeriod;
    clkParams.startFlag = FALSE;

//    /* Construct a periodic Clock Instance */
    Clock_construct(&clk1Struct, (Clock_FuncPtr)clk1_CallBack,
                    500000/Clock_tickPeriod, &clkParams);

    clkParams.period = 0;
//    clkParams.startFlag = FALSE;

    /* Construct a one-shot Clock Instance */
//    Clock_construct(&clk1Struct, (Clock_FuncPtr)clk1_CallBack,
//                    500000/Clock_tickPeriod, &clkParams);

    clk1Handle = Clock_handle(&clk1Struct);
}
void clk1_CallBack()
{
    GPIO_toggle(Board_UI_LED_1);
}
void Clock2_Configure()
{
    Clock_Params clkParams;
    Clock_Params_init(&clkParams);
    clkParams.period = 500000/Clock_tickPeriod;
    clkParams.startFlag = FALSE;

//    /* Construct a periodic Clock Instance */
    Clock_construct(&clk2Struct, (Clock_FuncPtr)clk2_CallBack,
                    500000/Clock_tickPeriod, &clkParams);

    clkParams.period = 0;
//    clkParams.startFlag = FALSE;

    /* Construct a one-shot Clock Instance */
//    Clock_construct(&clk2Struct, (Clock_FuncPtr)clk2_CallBack,
//                    10000000/Clock_tickPeriod, &clkParams);

    clk2Handle = Clock_handle(&clk2Struct);
}
void clk2_CallBack()
{
    GPIO_toggle(Board_UI_LED_2);
}
void Clock3_Configure()
{
    Clock_Params clkParams;
    Clock_Params_init(&clkParams);
    clkParams.period = 5000/Clock_tickPeriod;
    clkParams.startFlag = FALSE;

//    /* Construct a periodic Clock Instance */
    Clock_construct(&clk3Struct, (Clock_FuncPtr)clk3_CallBack,
                    500000/Clock_tickPeriod, &clkParams);

    clkParams.period = 0;
//    clkParams.startFlag = FALSE;

    /* Construct a one-shot Clock Instance */
//    Clock_construct(&clk3Struct, (Clock_FuncPtr)clk3_CallBack,
//                    11000/Clock_tickPeriod, &clkParams);

    clk3Handle = Clock_handle(&clk3Struct);
}
void clk3_CallBack()
{
    GPIO_toggle(Board_UI_LED_3);
}
/*
 * configure PWM
 */
