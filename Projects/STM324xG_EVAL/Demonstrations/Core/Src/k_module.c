/**
  ******************************************************************************
  * @file    k_module.c
  * @author  MCD Application Team
  * @version V1.3.2
  * @date    13-November-2015   
  * @brief   This file provides the kernel module functions 
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
#include "k_module.h"   
#include "dialog.h"

/** @addtogroup CORE
  * @{
  */

/** @defgroup KERNEL_MODULE
  * @brief Kernel module routines
  * @{
  */

/* External variables --------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private defines -----------------------------------------------------------*/
/* Private macros ------------------------------------------------------------*/
#define MAX_MODULES_NUM                                                       15
/* Private variables ---------------------------------------------------------*/
K_ModulePropertyTypedef    module_prop [MAX_MODULES_NUM];
K_ModuleOpenTypedef        open_link   [MAX_MODULES_NUM];
uint16_t module_num = 0;
uint16_t openlink_num = 0;
/* Private function prototypes -----------------------------------------------*/
static uint8_t k_ModuleVerify(K_ModuleItem_Typedef *module);
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Module Initialization.
  * @param  None.
  * @retval None
  */
void k_ModuleInit(void) 
{
  module_num = 0;
  openlink_num = 0;
  memset(module_prop, 0, sizeof(module_prop));
  memset(open_link, 0, sizeof(module_prop));  
}

/**
  * @brief  Get the module number.
  * @param  None.
  * @retval None
  */
uint8_t k_ModuleGetNumber(void) 
{
  return module_num;
}
/**
  * @brief  Add module.
  * @param  module: pointer to data structure of type K_ModuleItem_Typedef
  * @retval module add status
  */
uint8_t k_ModuleAdd(K_ModuleItem_Typedef *module) 
{
  if((module_num < (MAX_MODULES_NUM - 1)) && (k_ModuleVerify(module) == 0))
  {
    module_prop[module_num].module = module;
    module_num++;
    return 0;
   }
  else
  {
    return 1;
  }
}

/**
  * @brief  Link module to file browser open menu
  * @param  module: pointer to data structure of type K_ModuleItem_Typedef
  * @param  ext:    pointer to file extension.  
  * @retval None
  */

uint8_t k_ModuleOpenLink(K_ModuleItem_Typedef *module, char *ext) 
{
  if(openlink_num < (MAX_MODULES_NUM - 1))
  {
    open_link[openlink_num].module = module;
    strcpy(open_link[openlink_num].ext, ext);
    openlink_num++;
    return 0;
   }
  else
  {
    return 1;
  }
}

/**
  * @brief  Link module to file browser open menu
  * @param  ext: file extension
  * @retval pointer to module
  */

K_GET_DIRECT_OPEN_FUNC *k_ModuleCheckLink(char *ext) 
{
  uint8_t i   = 0;
  
  for (i = 0; i < MAX_MODULES_NUM; i++)
  {
    if(strcmp(open_link[i].ext, ext) == 0)
    {
      return (K_GET_DIRECT_OPEN_FUNC *)(open_link[i].module->DirectOpen);
    }
  }
  return NULL;
}

/**
  * @brief  Verify module.
  * @param  module: pointer to data structure of type K_ModuleItem_Typedef
  * @retval module ID status
  */

static uint8_t k_ModuleVerify(K_ModuleItem_Typedef *module) 
{
  uint8_t i   = 0;

  for (i = 0; i < MAX_MODULES_NUM; i++)
  {
    if(module_prop[i].module != NULL)
    {
      if(module_prop[i].module->id == module->id)
      {
        return 1;
      }
    }
  }
  return 0;
}

/**
  * @brief  Remove module
  * @param  module: pointer to data structure of type K_ModuleItem_Typedef
  * @retval None
  */

void k_ModuleRemove(K_ModuleItem_Typedef *module) 
{
  uint8_t idx = 0;
  
  for (idx  = 0; idx < MAX_MODULES_NUM; idx ++)
  {
    if(module_prop[module_num].module->id == module->id)
    {
      module_prop[module_num].module = NULL;
      module_num--;
    }
  }
}
/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
