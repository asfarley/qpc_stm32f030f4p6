#include "qpc.h"
#include "repl.h"
#include "bsp.h"
#include "stm32f0xx_hal.h"

//Q_DEFINE_THIS_FILE

/*..........................................................................*/
typedef struct {     /* the Repl active object */
    QActive super;   /* inherit QActive */

    QTimeEvt timeEvt; /* private time event generator */
} Repl;

static Repl l_repl; /* the Repl active object */

QActive * const AO_Repl = &l_repl.super;

UART_HandleTypeDef huart1;
int count = 0;
unsigned char notify[20] = {0};
GPIO_InitTypeDef GPIO_InitStruct;

static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);

/* hierarchical state machine ... */
static QState Repl_initial(Repl * const me, QEvt const * const e);
static QState Blinky_off    (Repl * const me, QEvt const * const e);
static QState Blinky_on     (Repl * const me, QEvt const * const e);

/*..........................................................................*/
void Repl_ctor(void) {
    Repl * const me = &l_repl;
    QActive_ctor(&me->super, Q_STATE_CAST(&Repl_initial));
    QTimeEvt_ctorX(&me->timeEvt, &me->super, TIMEOUT_SIG, 0U);
}

/* HSM definition ----------------------------------------------------------*/
QState Repl_initial(Repl * const me, QEvt const * const e) {
    (void)e; /* avoid compiler warning about unused parameter */

    MX_GPIO_Init();
    MX_USART1_UART_Init();

    /* arm the time event to expire in half a second and every half second */
    QTimeEvt_armX(&me->timeEvt, BSP_TICKS_PER_SEC/2U, BSP_TICKS_PER_SEC/2U);
    return Q_TRAN(&Blinky_off);
}
/*..........................................................................*/
QState Blinky_off(Repl * const me, QEvt const * const e) {
    QState status;
    switch (e->sig) {
        case Q_ENTRY_SIG: {\
        	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, 0);
        	int len = sprintf(notify, "LED_OFF %d\r\n", count);
			HAL_UART_Transmit(&huart1, notify, len, 0xFFFFFFFF);
			count++;
            status = Q_HANDLED();
            break;
        }
        case TIMEOUT_SIG: {
            status = Q_TRAN(&Blinky_on);
            break;
        }
        default: {
            status = Q_SUPER(&QHsm_top);
            break;
        }
    }
    return status;
}
/*..........................................................................*/
QState Blinky_on(Repl * const me, QEvt const * const e) {
    QState status;
    switch (e->sig) {
        case Q_ENTRY_SIG: {
        	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, 1);
        	int len = sprintf(notify, "LED_ON %d\r\n", count);
			HAL_UART_Transmit(&huart1, notify, len, 0xFFFFFFFF);
			count++;
            status = Q_HANDLED();
            break;
        }
        case TIMEOUT_SIG: {
            status = Q_TRAN(&Blinky_off);
            break;
        }
        default: {
            status = Q_SUPER(&QHsm_top);
            break;
        }
    }
    return status;
}

/* USART1 init function */
static void MX_USART1_UART_Init(void)
{

  huart1.Instance = USART1;
  huart1.Init.BaudRate = 9600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/** Pinout Configuration
*/
static void MX_GPIO_Init(void)
{
  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);

  /*Configure GPIO pin : PA4 */
  GPIO_InitStruct.Pin = GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

