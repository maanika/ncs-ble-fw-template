#include <zephyr/kernel.h>
#include <zephyr/usb/usb_device.h>
#include <zephyr/usb/usbd.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/logging/log.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/gap.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/addr.h>
#include <zephyr/bluetooth/conn.h>
/* TEMP: Nordic LED Button Service */
/* TODO: Add custom service */
#include <bluetooth/services/lbs.h>

#include <dk_buttons_and_leds.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

BUILD_ASSERT(DT_NODE_HAS_COMPAT(DT_CHOSEN(zephyr_console), zephyr_cdc_acm_uart),
             "Console device is not ACM CDC UART device");

#define USER_BUTTON             DK_BTN1_MSK
#define RUN_STATUS_LED          DK_LED1
#define CONNECTION_STATUS_LED   DK_LED2
#define RUN_LED_BLINK_INTERVAL  1000

#define DEVICE_NAME             CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN         (sizeof(DEVICE_NAME) - 1)

static struct bt_le_adv_param *adv_param = BT_LE_ADV_PARAM((
                /* Connectable advertising and use identity address */
                BT_LE_ADV_OPT_CONNECTABLE | BT_LE_ADV_OPT_USE_IDENTITY),
                BT_GAP_ADV_FAST_INT_MIN_1, /* 0x30 units, 48 units, 30ms */
                BT_GAP_ADV_FAST_INT_MAX_1, /* 0x60 units, 96 units, 60ms */
                NULL); /* Set to NULL for undirected advertising*/

static const struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
};

struct bt_conn *my_conn = NULL;

void on_connected(struct bt_conn *conn, uint8_t err)
{
    if (err) {
        LOG_ERR("Connection error %d", err);
        return;
    }
    LOG_INF("Connected");
    my_conn = bt_conn_ref(conn);

    dk_set_led(CONNECTION_STATUS_LED, 1);
}

void on_disconnected(struct bt_conn *conn, uint8_t reason)
{
    LOG_INF("Disconnected. Reason %d", reason);
    bt_conn_unref(my_conn);

    dk_set_led(CONNECTION_STATUS_LED, 0);
}

struct bt_conn_cb connection_callbacks = {
    .connected              = on_connected,
    .disconnected           = on_disconnected,
};

static void button_changed(uint32_t button_state, uint32_t has_changed)
{
	int err;
    if (has_changed & USER_BUTTON) {
        LOG_INF("Button changed");

        /* Send a notification using the LBS characteristic. */
        err = bt_lbs_send_button_state(button_state ? true : false);
        if (err) {
            LOG_ERR("Couldn't send notification. err: %d", err);
        }
    }
}

static int init_button(void)
{
    int err;
    err = dk_buttons_init(button_changed);
    if (err) {
        LOG_INF("Cannot init buttons (err: %d)", err);
    }

    return err;
}

#define VERSION "0.1.0"

int main(void)
{
    LOG_INF("Version %s", VERSION);

    const struct device *const dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));

    if (usb_enable(NULL)) {
        return 0;
    }

    /* Poll if the DTR flag was set */
    // uint32_t dtr = 0;
    // while (!dtr) {
	//     uart_line_ctrl_get(dev, UART_LINE_CTRL_DTR, &dtr);
    //     /* Give CPU resources to low priority threads. */
    //     k_sleep(K_MSEC(100));
    // }

    LOG_INF("LEDs init");
	int err;
    err = dk_leds_init();
    if (err) {
        LOG_ERR("LEDs init failed (err %d)", err);
        return 0;
	}

    LOG_INF("Buttons init");
    err = init_button();
    if (err) {
        LOG_ERR("Button init failed (err %d)", err);
        return 0;
    }

    bt_conn_cb_register(&connection_callbacks);

    LOG_INF("BLE init");
    err = bt_enable(NULL);
    if (err) {
        LOG_ERR("Bluetooth init failed (err %d)", err);
        return 0;
    }

    LOG_INF("Bluetooth initialized");
    err = bt_le_adv_start(adv_param, ad, ARRAY_SIZE(ad), NULL, 0);
    if (err) {
        LOG_ERR("Advertising failed to start (err %d)", err);
        return 0;
    }

    LOG_INF("Advertising successfully started");

    int blink_status = 0;
    for (;;) {
        dk_set_led(RUN_STATUS_LED, (++blink_status) % 2);
        k_sleep(K_MSEC(RUN_LED_BLINK_INTERVAL));
    }
}
