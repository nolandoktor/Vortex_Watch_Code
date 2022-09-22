#include <FreeRTOS_SAMD21.h>
#include <Arduino.h>
#include "ButtonHandler.h"
#include "../FreeRTOS_Plus/FreeRTOS_CLI.h"
#include "../Misc/Delay.h"
#include "../Misc/I2C_Helper.h"

#define CLI_STACK_SIZE 1024
#define CLI_MAX_OUTPUT_LENGTH 256
#define CLI_MAX_INPUT_LENGTH 256

TaskHandle_t xCLITask = NULL;

void cliTask(void *pvParameters);

//---------------------- CLI Function Prototypes ---------------------
static BaseType_t echoCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static BaseType_t i2c_write_byte_cmd(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static BaseType_t i2c_read_byte_cmd(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static BaseType_t i2c_bus_scan_cmd(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static BaseType_t gpio_read_cmd(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static BaseType_t gpio_write_cmd(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static BaseType_t gpio_dir_cmd(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
// TODO: 32-bit register read function
// TODO: 32-bit register write function
// TODO: Set time
// TODO: Get time
// TODO: Get watch state
// TODO: Set watch state
// TODO: Print stack usage
// TODO: Set accel ODR
// TODO: Enable/Disable datastream

static int parse_integer_param(const char *param, int len, int32_t *out);

//---------------------- CLI Struct Definitions ---------------------
static const CLI_Command_Definition_t echoCommandStruct =
    {
        "echo",
        "echo:\r\n Echoes parameter\r\n\r\n",
        echoCommand,
        2};
static const CLI_Command_Definition_t i2c_write_byte_CommandStruct =
    {
        "i2c_write_byte",
        "i2c_write_byte <device_addr> <reg_addr> <reg_val>:\r\n Writes byte from specified device and address\r\n\r\n",
        i2c_write_byte_cmd,
        3};
static const CLI_Command_Definition_t i2c_read_byte_CommandStruct =
    {
        "i2c_read_byte",
        "i2c_read_byte <device_addr> <reg_addr>:\r\n Reads byte from specified device and address\r\n\r\n",
        i2c_read_byte_cmd,
        2};
static const CLI_Command_Definition_t i2c_bus_scan_CommandStruct =
    {
        "i2c_bus_scan",
        "i2c_bus_scan:\r\n Scans I2C bus and returns list of device addresses present on bus\r\n\r\n",
        i2c_bus_scan_cmd,
        0};
static const CLI_Command_Definition_t gpio_read_CommandStruct =
    {
        "gpio_read",
        "gpio_read <pin>:\r\n Reads state of specified GPIO pin\r\n\r\n",
        gpio_read_cmd,
        1};
static const CLI_Command_Definition_t gpio_write_CommandStruct =
    {
        "gpio_write",
        "gpio_write <pin> <state>:\r\n Sets state of specified GPIO pin\r\n\r\n",
        gpio_write_cmd,
        2};
static const CLI_Command_Definition_t gpio_dir_CommandStruct =
    {
        "gpio_dir",
        "gpio_dir <pin> <mode>:\r\n Sets direction of specified GPIO pin\r\n\r\n",
        gpio_dir_cmd,
        2};

//---------------------- CLI Function Definitions ---------------------
static BaseType_t echoCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString)
{
    (void)pcCommandString;
    // Get parameter to echo
    // Get number of times to echo
    const char *echo_str;
    const char *cnt_str;
    int cnt;
    int processed = 0;
    BaseType_t xP1Len, xP2Len;
    echo_str = FreeRTOS_CLIGetParameter(
        pcCommandString, // The command string itself.
        1,               // Return the first parameter.
        &xP1Len          // Store the parameter string length.
    );
    cnt_str = FreeRTOS_CLIGetParameter(
        pcCommandString, // The command string itself.
        2,               // Return the first parameter.
        &xP2Len          // Store the parameter string length.
    );
    cnt = atoi(cnt_str);

    int string_length = ((xP1Len + 1) * cnt) + 1;
    if (string_length > xWriteBufferLen - 1)
    {
        strncpy(pcWriteBuffer, "Error: Output too large", xWriteBufferLen - 1);
    }
    else
    {
        for (int i = 0; i < cnt; i++)
        {
            memcpy(&pcWriteBuffer[processed], echo_str, xP1Len);
            processed += xP1Len;
            pcWriteBuffer[processed++] = ' ';
        }
        pcWriteBuffer[processed++] = '\n';
        pcWriteBuffer[processed++] = '\0';
        pcWriteBuffer[xWriteBufferLen - 1] = 0;
    }
    return pdFALSE;
}

static BaseType_t i2c_write_byte_cmd(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString)
{
    (void)pcCommandString;
    const char *dev_addr_str;
    const char *reg_addr_str;
    const char *reg_val_str;

    uint8_t dev_addr = 0;
    uint8_t reg_addr = 0;
    uint8_t reg_val = 0;

    BaseType_t p1_len, p2_len, p3_len;
    dev_addr_str = FreeRTOS_CLIGetParameter(
        pcCommandString, // The command string itself.
        1,               // Return the first parameter.
        &p1_len          // Store the parameter string length.
    );
    reg_addr_str = FreeRTOS_CLIGetParameter(
        pcCommandString, // The command string itself.
        2,               // Return the second parameter.
        &p2_len          // Store the parameter string length.
    );
    reg_val_str = FreeRTOS_CLIGetParameter(
        pcCommandString, // The command string itself.
        3,               // Return the third parameter.
        &p3_len          // Store the parameter string length.
    );

    if (p1_len > 4 || p2_len > 4 || p3_len > 4)
    {
        snprintf(pcWriteBuffer, xWriteBufferLen, "Error: Invalid inputs\n");
        return pdFALSE;
    }

    int32_t value;
    if (parse_integer_param(dev_addr_str, p1_len, &value) < 0) {
        snprintf(pcWriteBuffer, xWriteBufferLen, "Error: Failed to parse input\n");
        return pdFALSE;
    }
    dev_addr = (uint8_t)value;

    if (parse_integer_param(reg_addr_str, p2_len, &value) < 0) {
        snprintf(pcWriteBuffer, xWriteBufferLen, "Error: Failed to parse input\n");
        return pdFALSE;
    }
    reg_addr = (uint8_t)value;

    if (parse_integer_param(reg_val_str, p3_len, &value) < 0) {
        snprintf(pcWriteBuffer, xWriteBufferLen, "Error: Failed to parse input\n");
        return pdFALSE;
    }
    reg_val = (uint8_t)value;

    if (dev_addr > 127)
    {
        snprintf(pcWriteBuffer, xWriteBufferLen, "Error: Invalid device address (0x%02X), cannot exceed 127\n", dev_addr);
        return pdFALSE;
    }


    if (i2c_write_byte(dev_addr, reg_addr, reg_val) < 0)
    {
        snprintf(pcWriteBuffer, xWriteBufferLen, "Error: I2C write failed (Device 0x%02X Reg[0x%02X])\n", dev_addr, reg_addr);
        return pdFALSE;
    }

    snprintf(pcWriteBuffer, xWriteBufferLen, "Writing 0x%02X to Device 0x%02X Reg[0x%02X]\n", reg_val, dev_addr, reg_addr);
    return pdFALSE;
}

static BaseType_t i2c_read_byte_cmd(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString)
{
    (void)pcCommandString;
    const char *dev_addr_str;
    const char *reg_addr_str;

    uint8_t dev_addr = 0;
    uint8_t reg_addr = 0;
    uint8_t reg_val = 0;

    BaseType_t p1_len, p2_len;
    dev_addr_str = FreeRTOS_CLIGetParameter(
        pcCommandString, // The command string itself.
        1,               // Return the first parameter.
        &p1_len          // Store the parameter string length.
    );
    reg_addr_str = FreeRTOS_CLIGetParameter(
        pcCommandString, // The command string itself.
        2,               // Return the second parameter.
        &p2_len          // Store the parameter string length.
    );

    if (p1_len > 4 || p2_len > 4)
    {
        snprintf(pcWriteBuffer, xWriteBufferLen, "Error: Invalid inputs\n");
        return pdFALSE;
    }

    int32_t value;
    if (parse_integer_param(dev_addr_str, p1_len, &value) < 0) {
        snprintf(pcWriteBuffer, xWriteBufferLen, "Error: Failed to parse input\n");
        return pdFALSE;
    }
    dev_addr = (uint8_t)value;

    if (parse_integer_param(reg_addr_str, p2_len, &value) < 0) {
        snprintf(pcWriteBuffer, xWriteBufferLen, "Error: Failed to parse input\n");
        return pdFALSE;
    }
    reg_addr = (uint8_t)value;

    if (dev_addr > 127)
    {
        snprintf(pcWriteBuffer, xWriteBufferLen, "Error: Invalid device address (0x%02X), cannot exceed 127\n", dev_addr);
        return pdFALSE;
    }


    if (i2c_read_byte(dev_addr, reg_addr, &reg_val) < 0)
    {
        snprintf(pcWriteBuffer, xWriteBufferLen, "Error: I2C read failed. (Device 0x%02X Reg[0x%02X])\n", dev_addr, reg_addr);
        return pdFALSE;
    }

    snprintf(pcWriteBuffer, xWriteBufferLen, "Device 0x%02X Reg[0x%02X] = 0x%02X\n", dev_addr, reg_addr, reg_val);
    return pdFALSE;
}

static BaseType_t i2c_bus_scan_cmd(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString)
{
    (void)pcCommandString;
    static const uint8_t MAX_DEV_ADDR_CNT = 16;
    uint8_t dev_addr_list[MAX_DEV_ADDR_CNT];
    uint8_t dev_addr_cnt;

    if (i2c_bus_scan(dev_addr_list, &dev_addr_cnt, MAX_DEV_ADDR_CNT) < 0)
    {
        snprintf(pcWriteBuffer, xWriteBufferLen, "Error: I2C bus scan failed.\n");
        return pdFALSE;
    }

    int idx = snprintf(pcWriteBuffer, xWriteBufferLen, "[");
    for (int i=0; i<dev_addr_cnt-1; i++) {
        uint8_t bytes_written = snprintf(pcWriteBuffer+idx, xWriteBufferLen-idx, "0x%02X, ", dev_addr_list[i]);
        idx += bytes_written;
    }

    snprintf(pcWriteBuffer+idx, xWriteBufferLen-idx, "0x%02X]\n", dev_addr_list[dev_addr_cnt-1]);
    return pdFALSE;
}

static BaseType_t gpio_read_cmd(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString)
{
    (void)pcCommandString;
    const char *pin_str;
    
    uint8_t pin = 0;
    uint8_t state;

    BaseType_t p1_len;
    pin_str = FreeRTOS_CLIGetParameter(
        pcCommandString, // The command string itself.
        1,               // Return the first parameter.
        &p1_len          // Store the parameter string length.
    );

    if (p1_len > 2)
    {
        snprintf(pcWriteBuffer, xWriteBufferLen, "Error: Invalid pin\n");
        return pdFALSE;
    }

    int32_t value;
    if (parse_integer_param(pin_str, p1_len, &value) < 0) {
        snprintf(pcWriteBuffer, xWriteBufferLen, "Error: Failed to parse input\n");
        return pdFALSE;
    }
    pin = (uint8_t)value;
    state = digitalRead(pin);

    snprintf(pcWriteBuffer, xWriteBufferLen, "Pin[%d] = %d\n", pin, state);
    return pdFALSE;
}

static BaseType_t gpio_write_cmd(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString)
{
    (void)pcCommandString;
    const char *pin_str;
    const char *val_str;

    uint8_t pin = 0;
    uint8_t val = 0;

    BaseType_t p1_len, p2_len;
    pin_str = FreeRTOS_CLIGetParameter(
        pcCommandString, // The command string itself.
        1,               // Return the first parameter.
        &p1_len          // Store the parameter string length.
    );
    val_str = FreeRTOS_CLIGetParameter(
        pcCommandString, // The command string itself.
        2,               // Return the second parameter.
        &p2_len          // Store the parameter string length.
    );

    if (p1_len > 2 || p2_len > 1)
    {
        snprintf(pcWriteBuffer, xWriteBufferLen, "Error: Invalid inputs\n");
        return pdFALSE;
    }

    int32_t value;
    if (parse_integer_param(pin_str, p1_len, &value) < 0) {
        snprintf(pcWriteBuffer, xWriteBufferLen, "Error: Failed to parse input\n");
        return pdFALSE;
    }
    pin = (uint8_t)value;

    if (parse_integer_param(val_str, p2_len, &value) < 0) {
        snprintf(pcWriteBuffer, xWriteBufferLen, "Error: Failed to parse input\n");
        return pdFALSE;
    }
    val = (uint8_t)value;

    digitalWrite(pin, val);

    snprintf(pcWriteBuffer, xWriteBufferLen, "Setting pin[%d] to %d\n", pin, val);
    return pdFALSE;
}

static BaseType_t gpio_dir_cmd(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString)
{
    (void)pcCommandString;
    const char *pin_str;
    const char *dir_str;

    uint8_t pin = 0;
    uint8_t dir = 0;

    static const int DIR_MAP[3] = {INPUT, OUTPUT, INPUT_PULLUP};
    static const char *DIR_NAMES[3] = {"INPUT", "OUTPUT", "INPUT_PULLUP"};

    BaseType_t p1_len, p2_len;
    pin_str = FreeRTOS_CLIGetParameter(
        pcCommandString, // The command string itself.
        1,               // Return the first parameter.
        &p1_len          // Store the parameter string length.
    );
    dir_str = FreeRTOS_CLIGetParameter(
        pcCommandString, // The command string itself.
        2,               // Return the second parameter.
        &p2_len          // Store the parameter string length.
    );

    if (p1_len > 2 || p2_len > 1)
    {
        snprintf(pcWriteBuffer, xWriteBufferLen, "Error: Invalid inputs\n");
        return pdFALSE;
    }

    int32_t value;
    if (parse_integer_param(pin_str, p1_len, &value) < 0) {
        snprintf(pcWriteBuffer, xWriteBufferLen, "Error: Failed to parse input\n");
        return pdFALSE;
    }
    pin = (uint8_t)value;

    if (parse_integer_param(dir_str, p2_len, &value) < 0) {
        snprintf(pcWriteBuffer, xWriteBufferLen, "Error: Failed to parse input\n");
        return pdFALSE;
    }
    dir = (uint8_t)value;

    if (dir < 0 || dir > 2) {
        snprintf(pcWriteBuffer, xWriteBufferLen, "Error: Invalid value for dir, must be between 0 and 2\n");
        return pdFALSE;
    }

    pinMode(pin, DIR_MAP[dir]);

    snprintf(pcWriteBuffer, xWriteBufferLen, "Setting pin[%d] direction to %s\n", pin, DIR_NAMES[dir]);
    return pdFALSE;
}

static int parse_integer_param(const char *param, int len, int32_t *out)
{
    char data_buf[16] = {'\0'};
    uint32_t value;
    if (param == NULL) {
        return -1;
    }
    if (len > 15) {
        return -1;
    }

    memcpy(data_buf, param, len);

    if (data_buf[1] == 'x')
    {
        value = (uint32_t)strtol(data_buf, NULL, 0);
    }
    else
    {
        value = atoi(data_buf);
    }
    *out = value;
    return 0;
}
//---------------------- CLI Task Implementation ---------------------

void init_cli_task()
{
    xTaskCreate(
        cliTask,
        (const portCHAR *)"CLI_Task", // A name just for humans
        CLI_STACK_SIZE,               // Stack size
        NULL,                         // No Parameters
        3,                            // priority
        &xCLITask);
}

// TODO: Make dedicated Serial input thread and pass data to this thread with queue
void cliTask(void *pvParameters)
{
    (void)pvParameters;

    static const char CMD_END = '\n';
    static const char CMD_DEL = (char)127;
    char pcOutputString[CLI_MAX_OUTPUT_LENGTH];
    char pcInputString[CLI_MAX_INPUT_LENGTH];
    char char_in;
    uint8_t cInputIndex = 0;
    BaseType_t xMoreDataToFollow;

    FreeRTOS_CLIRegisterCommand(&echoCommandStruct);
    FreeRTOS_CLIRegisterCommand(&i2c_write_byte_CommandStruct);
    FreeRTOS_CLIRegisterCommand(&i2c_read_byte_CommandStruct);
    FreeRTOS_CLIRegisterCommand(&i2c_bus_scan_CommandStruct);
    FreeRTOS_CLIRegisterCommand(&gpio_read_CommandStruct);
    FreeRTOS_CLIRegisterCommand(&gpio_write_CommandStruct);
    FreeRTOS_CLIRegisterCommand(&gpio_dir_CommandStruct);

    while (1)
    {
        bool data_available = false;
        int bytes_available = Serial.available();
        if (bytes_available > 0)
        {
            data_available = true;
            char_in = Serial.read();

            if (char_in == CMD_END)
            {
                // Print newline for visibility
                Serial.println();

                /* The command interpreter is called repeatedly until it returns
                pdFALSE.  See the "Implementing a command" documentation for an
                exaplanation of why this is. */
                do
                {
                    /* Send the command string to the command interpreter.  Any
                    output generated by the command interpreter will be placed in the
                    pcOutputString buffer. */
                    xMoreDataToFollow = FreeRTOS_CLIProcessCommand(
                        pcInputString,        /* The command string.*/
                        pcOutputString,       /* The output buffer. */
                        CLI_MAX_OUTPUT_LENGTH /* The size of the output buffer. */
                    );

                    /* Write the output generated by the command interpreter to the
                    console. */
                    Serial.write(pcOutputString, strlen(pcOutputString));
                } while (xMoreDataToFollow != pdFALSE);

                /* All the strings generated by the input command have been sent.
                Processing of the command is complete.  Clear the input string ready
                to receive the next command. */
                cInputIndex = 0;
                memset(pcInputString, 0x00, CLI_MAX_INPUT_LENGTH);
            }
            else
            {
                if (char_in == CMD_DEL)
                {
                    Serial.write(&char_in, 1);

                    /* Backspace was pressed.  Erase the last character in the input
                    buffer - if there are any. */
                    if (cInputIndex > 0)
                    {
                        cInputIndex--;
                        pcInputString[cInputIndex] = '\0';
                    }
                }
                else
                {
                    /* A character was entered.  It was not a new line, backspace
                    or carriage return, so it is accepted as part of the input and
                    placed into the input buffer.  When a \n is entered the complete
                    string will be passed to the command interpreter. */
                    if (cInputIndex < CLI_MAX_INPUT_LENGTH)
                    {
                        Serial.write(&char_in, 1);
                        pcInputString[cInputIndex] = char_in;
                        cInputIndex++;
                    }
                }
            }
        }

        if (bytes_available <= 1)
        {
            k_msleep(100);
        }
    }
}