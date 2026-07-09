/* Edge Impulse + Servo Control (Captain Usopp Edition)
 * Model: CAP_SORTING_inferencing
 * Device: ESP32-CAM (AI Thinker)
 * Servo Pin: GPIO 12
 *
 * Action Logic:
 *  - GREEN → Servo to 90°, then back to 45°
 *  - RED → Servo to 0°, then back to 45°
 */

#include <CAP_SORTING_inferencing.h>
#include "edge-impulse-sdk/dsp/image/image.hpp"
#include "esp_camera.h"
#include <ESP32Servo.h>
// Servo motor library

// ===== Select camera model =====
#define CAMERA_MODEL_AI_THINKER // Has PSRAM

#if defined(CAMERA_MODEL_AI_THINKER)
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22
#else
#error "Camera model not selected"
#endif

// ===== Constants =====
#define EI_CAMERA_RAW_FRAME_BUFFER_COLS   320
#define EI_CAMERA_RAW_FRAME_BUFFER_ROWS   240
#define EI_CAMERA_FRAME_BYTE_SIZE         3
#define SERVO_PIN                         12  // Servo signal pin

// ===== Globals =====
static bool debug_nn = false;
static bool is_initialised = false;
uint8_t *snapshot_buf;
Servo myServo;
int defaultPos = 45;

// ===== Camera configuration =====
static camera_config_t camera_config = {
    .pin_pwdn = PWDN_GPIO_NUM,
    .pin_reset = RESET_GPIO_NUM,
    .pin_xclk = XCLK_GPIO_NUM,
    .pin_sscb_sda = SIOD_GPIO_NUM,
    .pin_sscb_scl = SIOC_GPIO_NUM,
    .pin_d7 = Y9_GPIO_NUM,
    .pin_d6 = Y8_GPIO_NUM,
    .pin_d5 = Y7_GPIO_NUM,
    .pin_d4 = Y6_GPIO_NUM,
    .pin_d3 = Y5_GPIO_NUM,
    .pin_d2 = Y4_GPIO_NUM,
    .pin_d1 = Y3_GPIO_NUM,
    .pin_d0 = Y2_GPIO_NUM,
    .pin_vsync = VSYNC_GPIO_NUM,
    .pin_href = HREF_GPIO_NUM,
    .pin_pclk = PCLK_GPIO_NUM,
    .xclk_freq_hz = 20000000,
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,
    .pixel_format = PIXFORMAT_JPEG,
    .frame_size = FRAMESIZE_QVGA,
    .jpeg_quality = 12,
    .fb_count = 1,
    .fb_location = CAMERA_FB_IN_PSRAM,
    .grab_mode = CAMERA_GRAB_WHEN_EMPTY,
};

// ===== Function Declarations =====
bool ei_camera_init(void);
bool ei_camera_capture(uint32_t img_width, uint32_t img_height, uint8_t *out_buf);
static int ei_camera_get_data(size_t offset, size_t length, float *out_ptr);

/* -------------------- SETUP -------------------- */
void setup() {
    Serial.begin(115200);
    while (!Serial);

    Serial.println("⚙️ Edge Impulse + Servo Demo (Captain Usopp Edition)");
    if (!ei_camera_init()) {
        ei_printf("❌ Failed to initialize Camera!\r\n");
    } else {
        ei_printf("✅ Camera initialized\r\n");
    }

    myServo.attach(SERVO_PIN);
    myServo.write(defaultPos);
    ei_printf("✅ Servo initialized at 45°\r\n");

    ei_printf("\nStarting continuous inference in 2 seconds...\n");
    ei_sleep(2000);
}

/* -------------------- MAIN LOOP -------------------- */
void loop() {
    if (ei_sleep(5) != EI_IMPULSE_OK) return;

    snapshot_buf = (uint8_t*)malloc(EI_CAMERA_RAW_FRAME_BUFFER_COLS * EI_CAMERA_RAW_FRAME_BUFFER_ROWS * EI_CAMERA_FRAME_BYTE_SIZE);
    if (!snapshot_buf) {
        ei_printf("ERR: Failed to allocate snapshot buffer!\n");
        return;
    }

    ei::signal_t signal;
    signal.total_length = EI_CLASSIFIER_INPUT_WIDTH * EI_CLASSIFIER_INPUT_HEIGHT;
    signal.get_data = &ei_camera_get_data;

    if (!ei_camera_capture(EI_CLASSIFIER_INPUT_WIDTH, EI_CLASSIFIER_INPUT_HEIGHT, snapshot_buf)) {
        ei_printf("❌ Capture failed\r\n");
        free(snapshot_buf);
        return;
    }

    ei_impulse_result_t result = { 0 };
    EI_IMPULSE_ERROR err = run_classifier(&signal, &result, debug_nn);
    if (err != EI_IMPULSE_OK) {
        ei_printf("ERR: Classifier failed (%d)\n", err);
        free(snapshot_buf);
        return;
    }

    ei_printf("⏱️ DSP: %d ms, Classification: %d ms, Anomaly: %d ms\n",
        result.timing.dsp, result.timing.classification, result.timing.anomaly);

#if EI_CLASSIFIER_OBJECT_DETECTION == 0
    ei_printf("Predictions:\r\n");
    float greenScore = 0.0;
    float redScore = 0.0;

    for (uint16_t i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
        ei_printf("  %s: %.5f\r\n", ei_classifier_inferencing_categories[i], result.classification[i].value);
        if (strcmp(ei_classifier_inferencing_categories[i], "green") == 0)
            greenScore = result.classification[i].value;
        else if (strcmp(ei_classifier_inferencing_categories[i], "red") == 0)
            redScore = result.classification[i].value;
    }

    // === Servo Actions ===
    if (greenScore > 0.7) {
        ei_printf("🟢 Detected GREEN → Servo to 90°\r\n");
        myServo.write(90);
        delay(1000);
        myServo.write(defaultPos);
    }
    else if (redScore > 0.7) {
        ei_printf("🔴 Detected RED → Servo to 0°\r\n");
        myServo.write(0);
        delay(1000);
        myServo.write(defaultPos);
    } else {
        myServo.write(defaultPos);
    }

#endif

    free(snapshot_buf);
}

/* -------------------- CAMERA FUNCTIONS -------------------- */
bool ei_camera_init(void) {
    if (is_initialised) return true;

    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed: 0x%x\n", err);
        return false;
    }

    sensor_t *s = esp_camera_sensor_get();
    if (s->id.PID == OV3660_PID) {
        s->set_vflip(s, 1);
        s->set_brightness(s, 1);
        s->set_saturation(s, 0);
    }

    is_initialised = true;
    return true;
}

bool ei_camera_capture(uint32_t img_width, uint32_t img_height, uint8_t *out_buf) {
    if (!is_initialised) {
        ei_printf("ERR: Camera not initialized\r\n");
        return false;
    }

    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
        ei_printf("Camera capture failed\n");
        return false;
    }

    bool converted = fmt2rgb888(fb->buf, fb->len, PIXFORMAT_JPEG, snapshot_buf);
    esp_camera_fb_return(fb);

    if (!converted) {
        ei_printf("Conversion failed\n");
        return false;
    }

    if (img_width != EI_CAMERA_RAW_FRAME_BUFFER_COLS || img_height != EI_CAMERA_RAW_FRAME_BUFFER_ROWS) {
        ei::image::processing::crop_and_interpolate_rgb888(
            out_buf, EI_CAMERA_RAW_FRAME_BUFFER_COLS, EI_CAMERA_RAW_FRAME_BUFFER_ROWS,
            out_buf, img_width, img_height);
    }

    return true;
}

static int ei_camera_get_data(size_t offset, size_t length, float *out_ptr) {
    size_t pixel_ix = offset * 3;
    size_t out_ptr_ix = 0;

    while (length--) {
        out_ptr[out_ptr_ix++] = (snapshot_buf[pixel_ix + 2] << 16) +
                                (snapshot_buf[pixel_ix + 1] << 8) +
                                 snapshot_buf[pixel_ix];
        pixel_ix += 3;
    }
    return 0;
}
