#include "deviceManagers/peripherals.h"

static uint8_t dboardDeviceId              = NEW_DEVICE_REQUEST_FAILED;
static uint8_t dboardScancodeMappings[256] = {0};

static int glfwKeyTokensForDboard[][2] = 
{
   {GLFW_KEY_SPACE,         DBOARD_SCANCODE_SPACE},
   {GLFW_KEY_APOSTROPHE,    DBOARD_SCANCODE_QUOTE},
   {GLFW_KEY_COMMA,         DBOARD_SCANCODE_COMMA},
   {GLFW_KEY_MINUS,         DBOARD_SCANCODE_MINUS},
   {GLFW_KEY_PERIOD,        DBOARD_SCANCODE_PERIOD},
   {GLFW_KEY_SLASH,         DBOARD_SCANCODE_SLASH},
   {GLFW_KEY_0,             DBOARD_SCANCODE_0},
   {GLFW_KEY_1,             DBOARD_SCANCODE_1},
   {GLFW_KEY_2,             DBOARD_SCANCODE_2},
   {GLFW_KEY_3,             DBOARD_SCANCODE_3},
   {GLFW_KEY_4,             DBOARD_SCANCODE_4},
   {GLFW_KEY_5,             DBOARD_SCANCODE_5},
   {GLFW_KEY_6,             DBOARD_SCANCODE_6},
   {GLFW_KEY_7,             DBOARD_SCANCODE_7},
   {GLFW_KEY_8,             DBOARD_SCANCODE_8},
   {GLFW_KEY_9,             DBOARD_SCANCODE_9},
   {GLFW_KEY_SEMICOLON,     DBOARD_SCANCODE_SEMICOLON},
   {GLFW_KEY_EQUAL,         DBOARD_SCANCODE_EQUALS},
   {GLFW_KEY_A,             DBOARD_SCANCODE_A},
   {GLFW_KEY_B,             DBOARD_SCANCODE_B},
   {GLFW_KEY_C,             DBOARD_SCANCODE_C},
   {GLFW_KEY_D,             DBOARD_SCANCODE_D},
   {GLFW_KEY_E,             DBOARD_SCANCODE_E},
   {GLFW_KEY_F,             DBOARD_SCANCODE_F},
   {GLFW_KEY_G,             DBOARD_SCANCODE_G},
   {GLFW_KEY_H,             DBOARD_SCANCODE_H},
   {GLFW_KEY_I,             DBOARD_SCANCODE_I},
   {GLFW_KEY_J,             DBOARD_SCANCODE_J},
   {GLFW_KEY_K,             DBOARD_SCANCODE_K},
   {GLFW_KEY_L,             DBOARD_SCANCODE_L},
   {GLFW_KEY_M,             DBOARD_SCANCODE_M},
   {GLFW_KEY_N,             DBOARD_SCANCODE_N},
   {GLFW_KEY_O,             DBOARD_SCANCODE_O},
   {GLFW_KEY_P,             DBOARD_SCANCODE_P},
   {GLFW_KEY_Q,             DBOARD_SCANCODE_Q},
   {GLFW_KEY_R,             DBOARD_SCANCODE_R},
   {GLFW_KEY_S,             DBOARD_SCANCODE_S},
   {GLFW_KEY_T,             DBOARD_SCANCODE_T},
   {GLFW_KEY_U,             DBOARD_SCANCODE_U},
   {GLFW_KEY_V,             DBOARD_SCANCODE_V},
   {GLFW_KEY_W,             DBOARD_SCANCODE_W},
   {GLFW_KEY_X,             DBOARD_SCANCODE_X},
   {GLFW_KEY_Y,             DBOARD_SCANCODE_Y},
   {GLFW_KEY_Z,             DBOARD_SCANCODE_Z},
   {GLFW_KEY_LEFT_BRACKET,  DBOARD_SCANCODE_OPEN_BRACKET},
   {GLFW_KEY_BACKSLASH,     DBOARD_SCANCODE_BACKSLASH},
   {GLFW_KEY_RIGHT_BRACKET, DBOARD_SCANCODE_CLOSE_BRACKET},
   {GLFW_KEY_GRAVE_ACCENT,  DBOARD_SCANCODE_GRAVE_ACCENT},
   {GLFW_KEY_ESCAPE,        DBOARD_SCANCODE_ESC},
   {GLFW_KEY_ENTER,         DBOARD_SCANCODE_ENTER},
   {GLFW_KEY_TAB,           DBOARD_SCANCODE_TAB},
   {GLFW_KEY_BACKSPACE,     DBOARD_SCANCODE_BACKSPACE},
   {GLFW_KEY_DELETE,        DBOARD_SCANCODE_DELETE},
   {GLFW_KEY_RIGHT,         DBOARD_SCANCODE_RIGHT},
   {GLFW_KEY_LEFT,          DBOARD_SCANCODE_LEFT},
   {GLFW_KEY_DOWN,          DBOARD_SCANCODE_DOWN},
   {GLFW_KEY_UP,            DBOARD_SCANCODE_UP},
   {GLFW_KEY_LEFT_SHIFT,    DBOARD_SCANCODE_LSHIFT},
   {GLFW_KEY_LEFT_CONTROL,  DBOARD_SCANCODE_LCTRL},
   {GLFW_KEY_LEFT_ALT,      DBOARD_SCANCODE_LALT},
   {GLFW_KEY_RIGHT_SHIFT,   DBOARD_SCANCODE_RSHIFT},
   {GLFW_KEY_RIGHT_CONTROL, DBOARD_SCANCODE_RCTRL},
   {GLFW_KEY_RIGHT_ALT,     DBOARD_SCANCODE_RALT}
};

static bool dboardCanWriteMap[] = 
{
    false,
    false
};

// Derial
static uint8_t derialDeviceId                                  = NEW_DEVICE_REQUEST_FAILED;
static uint8_t derialOutboundBuffer[DERIAL_OUTBOUND_BUF_SIZE]  = {0};
static uint8_t derialOutboundBufferHead                        = 0;
static bool    derialShouldFlush                               = false;

static bool derialConfigIntOnFlush = false;
static bool derialConfigAutoFlush  = false;
static bool derialConfigIntOnRcv   = true;

static bool derialStatusBufNotEmpty = false;
static bool derialStatusBufFull     = false;
static bool derialStatusInboudReady = false;
static bool derialStatusRcvMissed   = false;

static bool derialCanWriteMap[] =
{
    false,
    false,
    false,
    true,
    true,
    true,
    false
};

static void pr_dboardHandleKeyPress(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (GLFW_REPEAT == action ||
        scancode    >  255)
    {
        return;
    }

    if (DBOARD_SCANCODE_NONE != dboardScancodeMappings[scancode])
    {
        if (GLFW_PRESS == action)
        {
            dmi_enqueueInterrupt(dboardDeviceId, it_keyPressed, dboardScancodeMappings[scancode]);
        }
        else
        {
            dmi_enqueueInterrupt(dboardDeviceId, it_keyReleased, dboardScancodeMappings[scancode]);
        }
    }
}

static void pr_derialUpdateConfig(uint8_t configData)
{
    derialConfigIntOnFlush = 0 != (configData & DERIAL_CONFIG_INT_ON_FLUSH_MASK);
    derialConfigAutoFlush  = 0 != (configData & DERIAL_CONFIG_AUTO_FLUSH_MASK);
    derialConfigIntOnRcv   = 0 != (configData & DERIAL_CONFIG_INT_ON_RCV_MASK);
}

static bool pr_derialUpdateStatus()
{
    uint8_t buf8 = 0;

    if (derialStatusBufNotEmpty)
    {
        buf8 |= DERIAL_STATUS_BUF_NOT_EMPTY_MASK;
    }

    if (derialStatusBufFull)
    {
        buf8 |= DERIAL_STATUS_BUF_FULL_MASK;
    }

    if (derialStatusInboudReady)
    {
        buf8 |= DERIAL_STATUS_INBOUND_READY_MASK;
    }

    if (derialStatusRcvMissed)
    {
        buf8 |= DERIAL_STATUS_RCV_MISSED_MASK;
    }

    return dmi_writeDeviceData(derialDeviceId,
                               DERIAL_STATUS_ADDRESS,
                               1,
                               &buf8);
}

static void pr_derialHandleTermIn(uint8_t data)
{
    if (false == dmi_writeDeviceData(derialDeviceId, DERIAL_INBOUND_ADDRESS, 1, &data))
    {
        return;
    }

    if (derialStatusInboudReady)
    {
        derialStatusRcvMissed = true;
    }
    derialStatusInboudReady = true;

    if (false == pr_derialUpdateStatus())
    {
        usleep(10);
        return;
    }

    if (derialConfigIntOnRcv)
    {
        dmi_enqueueInterrupt(derialDeviceId, it_peripheralEvent, DERIAL_RECEIVED_INT_OVERLAY);
    }

    usleep(10);
}

static void pr_derialHandleReadWrite()
{
    if (derialShouldFlush)
    {
        derialShouldFlush = false;

        for (uint8_t i = 0; i < derialOutboundBufferHead; i++)
        {
            printf("%c", derialOutboundBuffer[i]);
        }

        derialOutboundBufferHead = 0;

        // Clear buf not empty and buf full status
        derialStatusBufNotEmpty = false;
        derialStatusBufFull     = false;

        if (false == pr_derialUpdateStatus())
        {
            return;
        }

        // Enqueue interrupt
        if (derialConfigIntOnFlush)
        {
            dmi_enqueueInterrupt(derialDeviceId, it_peripheralEvent, DERIAL_FLUSH_COMPLETE_INT_OVERLAY);
        }
    }
}

static bool pr_initDboardScancodeMappings()
{
    int systemScancode = 0;

    for (uint16_t i = 0; i < 256; i++)
    {
        dboardScancodeMappings[i] = DBOARD_SCANCODE_NONE;
    }

    for (uint8_t i = 0; i < sizeof(glfwKeyTokensForDboard) / (2 * sizeof(int)); i++)
    {
        systemScancode = glfwGetKeyScancode(glfwKeyTokensForDboard[i][0]);

        if (systemScancode < 0)
        {
            printf("Failed to get scancode\n");
            return false;
        }

        if (systemScancode > 255)
        {
            printf("Scancode out of range\n");
            return false;
        }

        dboardScancodeMappings[systemScancode] = glfwKeyTokensForDboard[i][1];
    }

    return true;
}

static void pr_handleRead(uint8_t deviceId, uint16_t deviceDataAddress, uint8_t numBytes)
{
    if (derialDeviceId == deviceId)
    {
        if (1 != numBytes)
        {
            return;
        }

        if (DERIAL_INBOUND_ADDRESS == deviceDataAddress)
        {
            derialStatusInboudReady = false;
            derialStatusRcvMissed   = false;

            (void)pr_derialUpdateStatus();
        }
    }
}

static void pr_handleWrite(uint8_t deviceId, uint16_t deviceDataAddress, uint8_t numBytes, void *data)
{
    if (dboardDeviceId == deviceId)
    {
        (void) dmi_writeDeviceData(dboardDeviceId, 0, 2, (uint8_t[]) {DBOARD_PERIPHERAL_TYPE_ID,
                                                                      DBOARD_KEYBOARD_PROT_ID});

        return;
    }
    else if (derialDeviceId == deviceId)
    {
        // All writes only make sense for 1 byte
        if (1 != numBytes)
        {
            return;
        }

        if (DERIAL_OUTBOUND_ADDRESS == deviceDataAddress)
        {
            // Skip if buffer is full
            if (derialOutboundBufferHead == DERIAL_OUTBOUND_BUF_SIZE)
            {
                return;
            }

            derialOutboundBuffer[derialOutboundBufferHead++] = *((uint8_t*) data);

            // Updating status
            derialStatusBufNotEmpty = true;
            if (DERIAL_OUTBOUND_BUF_SIZE == derialOutboundBufferHead)
            {
                derialStatusBufFull = true;
            }

            if (false == pr_derialUpdateStatus())
            {
                return;
            }

            // Auto flush
            if (derialConfigAutoFlush)
            {
                derialShouldFlush = true;
            }
        }
        else if (DERIAL_FLUSH_ADDRESS == deviceDataAddress)
        {
            derialShouldFlush = true;
        }
        else if (DERIAL_CONFIG_ADDRESS == deviceDataAddress)
        {
            pr_derialUpdateConfig(*((uint8_t*) data));
        }
    }
}

int pr_initDeviceManager(void *arg)
{
    threadArg_t myThreadData = {0};

    memcpy(&myThreadData, arg, sizeof(threadArg_t));
    free(arg);

    mtx_lock(myThreadData.mutex);

    // Request initial devices

    // Dboard
    dboardDeviceId = dmi_requestNewDevice(myThreadData.managerId,
                                          DBOARD_DDAT_SIZE,
                                          dboardCanWriteMap);

    if (NEW_DEVICE_REQUEST_FAILED == dboardDeviceId ||
        false == dmi_writeDeviceData(dboardDeviceId, 0, 2, (uint8_t[]) {DBOARD_PERIPHERAL_TYPE_ID,
                                                                        DBOARD_KEYBOARD_PROT_ID})||
        false == pr_initDboardScancodeMappings())
    {
        myThreadData.semaphore->wakeReason = dts_kill;
        cnd_signal(myThreadData.wakeCondition);
        mtx_unlock(myThreadData.mutex);
        return -1;
    }

    glfwSetKeyCallback(myThreadData.glfwInfo.window, pr_dboardHandleKeyPress);

    // Derial
    derialDeviceId = dmi_requestNewDevice(myThreadData.managerId,
                                          DERIAL_DDAT_SIZE,
                                          derialCanWriteMap);

    if (NEW_DEVICE_REQUEST_FAILED == derialDeviceId ||
        false == dmi_writeDeviceData(derialDeviceId, 0, 2, (uint8_t[]) {DERIAL_PERIPHERAL_TYPE_ID,
                                                                        DERIAL_SERIAL_PROT_ID}))
    {
        myThreadData.semaphore->wakeReason = dts_kill;
        cnd_signal(myThreadData.wakeCondition);
        mtx_unlock(myThreadData.mutex);
        return -1;
    }

    // Bind the read/write handlers
    if (false == dmi_bindHandleTermIn(myThreadData.managerId, pr_derialHandleTermIn) ||
        false == dmi_bindHandleRead  (myThreadData.managerId, pr_handleRead)         ||
        false == dmi_bindHandleWrite (myThreadData.managerId, pr_handleWrite))
    {
        myThreadData.semaphore->wakeReason = dts_kill;
        cnd_signal(myThreadData.wakeCondition);
        mtx_unlock(myThreadData.mutex);
        return -1;
    }

    cnd_signal(myThreadData.wakeCondition);

    while (true)
    {
        cnd_wait(myThreadData.wakeCondition, myThreadData.mutex);

        switch (myThreadData.semaphore->wakeReason)
        {
            case dts_kill:
                return 0;
            case dts_handleReadWrite:
                if (myThreadData.semaphore->deviceId == derialDeviceId)
                {
                    pr_derialHandleReadWrite();
                }
                break;
            case dts_continue:
                break;
        }


        myThreadData.semaphore->wakeReason = dts_continue;
    }
}