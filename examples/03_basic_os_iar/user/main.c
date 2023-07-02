/*
 * eLesson Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

/* Includes ------------------------------------------------------------------*/
#include "bsp.h"
#include "basic_os.h"

/* public functions --------------------------------------------------------- */
/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
    bsp_init();
    basic_os_init();

    basic_os_run();
}

/* ----------------------------- end of file -------------------------------- */
