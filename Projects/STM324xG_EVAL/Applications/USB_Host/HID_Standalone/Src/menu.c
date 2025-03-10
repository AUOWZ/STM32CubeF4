/**
  ******************************************************************************
  * @file    USB_Host/HID_Standalone/Src/menu.c 
  * @author  MCD Application Team
  * @version V1.3.2
  * @date    13-November-2015
  * @brief   This file implements Menu Functions
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2015 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
HID_DEMO_StateMachine hid_demo;
uint8_t               prev_select = 0;

uint8_t *DEMO_KEYBOARD_menu[] = 
{
  (uint8_t *)"      1 - Start Keyboard / Clear                                            ",
  (uint8_t *)"      2 - Return                                                            ",
}; 

uint8_t *DEMO_MOUSE_menu[] = 
{
  (uint8_t *)"      1 - Start Mouse / Re-Initialize                                       ",
  (uint8_t *)"      2 - Return                                                            ",
};

uint8_t *DEMO_HID_menu[] = 
{
  (uint8_t *)"      1 - Start HID                                                         ",
  (uint8_t *)"      2 - Re-Enumerate                                                      ",
};

/* Private function prototypes -----------------------------------------------*/
static void HID_DEMO_ProbeKey(JOYState_TypeDef state);
static void USBH_MouseDemo(USBH_HandleTypeDef *phost);
static void USBH_KeybdDemo(USBH_HandleTypeDef *phost);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Demo state machine.
  * @param  None
  * @retval None
  */
void HID_MenuInit(void)
{
  /* Start HID Interface */
  USBH_UsrLog("Starting HID Demo");
  
  BSP_LCD_SetTextColor(LCD_COLOR_GREEN);
  BSP_LCD_DisplayStringAtLine(15, (uint8_t *)"Use [Joystick Left/Right] to scroll up/down");
  BSP_LCD_DisplayStringAtLine(16, (uint8_t *)"Use [Joystick Up/Down] to scroll HID menu");
  hid_demo.state = HID_DEMO_IDLE;
  HID_MenuProcess();
}

/**
  * @brief  Manages HID Menu Process.
  * @param  None
  * @retval None
  */
void HID_MenuProcess(void)
{
  switch(hid_demo.state)
  {
  case HID_DEMO_IDLE:
    HID_SelectItem(DEMO_HID_menu, 0); 
    hid_demo.state = HID_DEMO_WAIT;
    hid_demo.select = 0;
    break;        
    
  case HID_DEMO_WAIT:
    if(hid_demo.select != prev_select)
    {
      prev_select = hid_demo.select;        
      
      HID_SelectItem(DEMO_HID_menu, hid_demo.select & 0x7F); 
      /* Handle select item */
      if(hid_demo.select & 0x80)
      {
        hid_demo.select &= 0x7F;
        switch(hid_demo.select)
        {
        case 0:
          hid_demo.state = HID_DEMO_START;  
          break;
          
        case 1:
          hid_demo.state = HID_DEMO_REENUMERATE;  
          break;
          
        default:
          break;
        }
      }
    }
    break; 
    
  case HID_DEMO_START:
    if(Appli_state == APPLICATION_READY)
    {
      if(USBH_HID_GetDeviceType(&hUSBHost) == HID_KEYBOARD)
      {
        hid_demo.keyboard_state = HID_KEYBOARD_IDLE; 
        hid_demo.state = HID_DEMO_KEYBOARD;
      }
      else if(USBH_HID_GetDeviceType(&hUSBHost) == HID_MOUSE)
      {
        hid_demo.mouse_state = HID_MOUSE_IDLE;  
        hid_demo.state = HID_DEMO_MOUSE;        
      }
    }
    else
    {
      LCD_ErrLog("No supported HID device!\n");
      hid_demo.state = HID_DEMO_WAIT;
    }
    break;
    
  case HID_DEMO_REENUMERATE:
    /* Force HID Device to re-enumerate */
    USBH_ReEnumerate(&hUSBHost); 
    hid_demo.state = HID_DEMO_WAIT;
    break;

  case HID_DEMO_MOUSE:
    if(Appli_state == APPLICATION_READY)
    {
      HID_MouseMenuProcess();
      USBH_MouseDemo(&hUSBHost);
    }
    break; 
    
  case HID_DEMO_KEYBOARD:
    if(Appli_state == APPLICATION_READY)  
    {    
      HID_KeyboardMenuProcess();
      USBH_KeybdDemo(&hUSBHost);
    }   
    break;
    
  default:
    break;
  }

  if(Appli_state == APPLICATION_DISCONNECT)
  {
    Appli_state = APPLICATION_IDLE; 
    LCD_LOG_ClearTextZone();
    LCD_ErrLog("HID device disconnected!\n");
    hid_demo.state = HID_DEMO_IDLE;
    hid_demo.select = 0;    
  }  
}

/**
  * @brief  Manages the menu on the screen.
  * @param  menu: Menu table
  * @param  item: Selected item to be highlighted
  * @retval None
  */
void HID_SelectItem(uint8_t **menu, uint8_t item)
{
  BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
  
  switch(item)
  {
  case 0: 
    BSP_LCD_SetBackColor(LCD_COLOR_MAGENTA);
    BSP_LCD_DisplayStringAtLine( 18, menu [0]);
    BSP_LCD_SetBackColor(LCD_COLOR_BLUE);    
    BSP_LCD_DisplayStringAtLine(19,  menu [1]); 
    break;
    
  case 1: 
    BSP_LCD_SetBackColor(LCD_COLOR_BLUE);
    BSP_LCD_DisplayStringAtLine( 18, menu [0]);
    BSP_LCD_SetBackColor(LCD_COLOR_MAGENTA);    
    BSP_LCD_DisplayStringAtLine( 19, menu [1]);
    BSP_LCD_SetBackColor(LCD_COLOR_BLUE);   
    break;   
  }
  BSP_LCD_SetBackColor(LCD_COLOR_BLACK); 
  
}

/**
  * @brief  Probes the HID joystick state.
  * @param  state: Joystick state
  * @retval None
  */
static void HID_DEMO_ProbeKey(JOYState_TypeDef state)
{
  /* Handle Menu inputs */
  if((state == JOY_UP) && (hid_demo.select > 0))
  {
    hid_demo.select--;
  }
  else if((state == JOY_DOWN) && (hid_demo.select < 1))
  {
    hid_demo.select++;
  }
  else if(state == JOY_SEL)
  {
    hid_demo.select |= 0x80;
  }  
}

/**
  * @brief  EXTI line detection callbacks.
  * @param  GPIO_Pin: Specifies the pins connected EXTI line
  * @retval None
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{  
  static JOYState_TypeDef JoyState = JOY_NONE;
  
  if(GPIO_Pin == GPIO_PIN_2)
  {    
    /* Get the Joystick State */
    JoyState = BSP_JOY_GetState();
    
    HID_DEMO_ProbeKey(JoyState); 
    
    switch(JoyState)
    {
    case JOY_LEFT:
      LCD_LOG_ScrollBack();
      break;     
    
    case JOY_RIGHT:
      LCD_LOG_ScrollForward();
      break;          
      
    default:
      break;           
    }
    
    /* Clear joystick interrupt pending bits */
    BSP_IO_ITClear(JOY_ALL_PINS);
  }
}

/**
  * @brief  Main routine for Mouse application
  * @param  phost: Host handle
  * @retval None
  */
static void USBH_MouseDemo(USBH_HandleTypeDef *phost)
{
  HID_MOUSE_Info_TypeDef *m_pinfo;  
  
  m_pinfo = USBH_HID_GetMouseInfo(phost);
  if(m_pinfo != NULL)
  {
    /* Handle Mouse data position */
    USR_MOUSE_ProcessData(&mouse_info);
    
    if(m_pinfo->buttons[0])
    {
      HID_MOUSE_ButtonPressed(0);
    }
    else
    {
      HID_MOUSE_ButtonReleased(0);
    }
    
    if(m_pinfo->buttons[1])
    {
      HID_MOUSE_ButtonPressed(2);
    }
    else
    {
      HID_MOUSE_ButtonReleased(2);
    }
    
    if(m_pinfo->buttons[2])
    {
      HID_MOUSE_ButtonPressed(1);
    }
    else
    {
      HID_MOUSE_ButtonReleased(1);
    }
  }
}

/**
  * @brief  Main routine for Keyboard application
  * @param  phost: Host handle
  * @retval None
  */
static void USBH_KeybdDemo(USBH_HandleTypeDef *phost)
{
  HID_KEYBD_Info_TypeDef *k_pinfo; 
  char c;
  k_pinfo = USBH_HID_GetKeybdInfo(phost);
  
  if(k_pinfo != NULL)
  {
    c = USBH_HID_GetASCIICode(k_pinfo);
    if(c != 0)
    {
      USR_KEYBRD_ProcessData(c);
    }
  }
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
