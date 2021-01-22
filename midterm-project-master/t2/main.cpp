#include "mbed.h"
#include <cmath>
#include <string>
#include <iostream>
#include "DA7212.h"
#include "uLCD_4DGL.h"
#include "accelerometer_handler.h"
#include "config.h"
#include "magic_wand_model_data.h"
#include "tensorflow/lite/c/common.h"
#include "tensorflow/lite/micro/kernels/micro_ops.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"
#define NUM_OF_SONGS 3
#define NUM_OF_MODES 4
#define NUM_OF_FREQUNCYS 49
#define signal_Length 49
#define bufferLength 32
#define UINT14_MAX 16383
// FXOS8700CQ I2C address
#define FXOS8700CQ_SLAVE_ADDR0 (0x1E<<1) // with pins SA0=0, SA1=0
#define FXOS8700CQ_SLAVE_ADDR1 (0x1D<<1) // with pins SA0=1, SA1=0
#define FXOS8700CQ_SLAVE_ADDR2 (0x1C<<1) // with pins SA0=0, SA1=1
#define FXOS8700CQ_SLAVE_ADDR3 (0x1F<<1) // with pins SA0=1, SA1=1
// FXOS8700CQ internal register addresses
#define FXOS8700Q_STATUS 0x00
#define FXOS8700Q_OUT_X_MSB 0x01
#define FXOS8700Q_OUT_Y_MSB 0x03
#define FXOS8700Q_OUT_Z_MSB 0x05
#define FXOS8700Q_M_OUT_X_MSB 0x33
#define FXOS8700Q_M_OUT_Y_MSB 0x35
#define FXOS8700Q_M_OUT_Z_MSB 0x37
#define FXOS8700Q_WHOAMI 0x0D
#define FXOS8700Q_XYZ_DATA_CFG 0x0E
#define FXOS8700Q_CTRL_REG1 0x2A
#define FXOS8700Q_M_CTRL_REG1 0x5B
#define FXOS8700Q_M_CTRL_REG2 0x5C
#define FXOS8700Q_WHOAMI_VAL 0xC7

DA7212 audio;
Serial pc(USBTX, USBRX);
int16_t waveform[kAudioTxBufferSize];
uLCD_4DGL uLCD(D1, D0, D2);
InterruptIn sw2(SW2);
I2C i2c_( PTD9,PTD8);
int m_addr = FXOS8700CQ_SLAVE_ADDR1;
int stop_music = 1;
int this_song = 0;
int mode_iter = 0;
int choose_iter = 0;
int score = 0;
int mode;
int stop_dnn = 0;
int note_Length[3][49];
Thread thread2(osPriorityNormal);
Thread thread(osPriorityNormal);
EventQueue queue2(32 * EVENTS_EVENT_SIZE);
EventQueue queue;
DigitalOut green_led(LED2);
int song[NUM_OF_SONGS][NUM_OF_FREQUNCYS];
int beat[NUM_OF_FREQUNCYS] = {
    0, 1, 0, 1, 0, 1,
    0, 1, 0, 1, 0, 1,
    0, 1, 0, 1, 0, 1,
    0, 1, 0, 1, 0, 1,
    0, 1, 0, 1, 0, 1,
    0, 1, 0, 1, 0, 1,
    0, 1, 0, 1, 0, 1,
    0 ,1, 0, 1, 0, 1,0
};
// Accelerometer variable
uint8_t who_am_i, data[2], res[6];
int16_t acc16;
float t[3];
// DNN variable
constexpr int kTensorArenaSize = 50 * 1024;
uint8_t tensor_arena[kTensorArenaSize];
bool should_clear_buffer = false;
bool got_data = false;
int gesture_index;
static tflite::MicroErrorReporter micro_error_reporter;
tflite::ErrorReporter* error_reporter = &micro_error_reporter;
int input_length;
TfLiteTensor* model_input;
tflite::MicroInterpreter* interpreter;
char serial_buffer[bufferLength];

// Return the result of the last prediction
int PredictGesture(float* output) {
    // How many times the most recent gesture has been matched in a row
    static int continuous_count = 0;
    // The result of the last prediction
    static int last_predict = -1;

    // Find whichever output has a probability > 0.8 (they sum to 1)
    int this_predict = -1;
    for (int i = 0; i < label_num; i++) {
        if (output[i] > 0.8) this_predict = i;
    }

    // No gesture was detected above the threshold
    if (this_predict == -1) {
        continuous_count = 0;
        last_predict = label_num;
        return label_num;
    }

    if (last_predict == this_predict) {
        continuous_count += 1;
    } else {
        continuous_count = 0;
    }
    last_predict = this_predict;

    // If we haven't yet had enough consecutive matches for this gesture,
    // report a negative result
    if (continuous_count < config.consecutiveInferenceThresholds[this_predict]) {
        return label_num;
    }
    // Otherwise, we've seen a positive result, so clear all our variables
    // and report it
    continuous_count = 0;
    last_predict = -1;

    return this_predict;
}


void FXOS8700CQ_ReadRegs(int addr, uint8_t * data, int len) {
    char t = addr;
    i2c_.write(m_addr, &t, 1, true);
    i2c_.read(m_addr, (char *)data, len);
}

void FXOS8700CQ_WriteRegs(uint8_t * data, int len) {
    i2c_.write(m_addr, (char *)data, len);
}
float ReadAcc() {
    FXOS8700CQ_ReadRegs(FXOS8700Q_OUT_X_MSB, res, 6);

    acc16 = (res[0] << 6) | (res[1] >> 2);
    if (acc16 > UINT14_MAX/2)
        acc16 -= UINT14_MAX;
    t[0] = ((float)acc16) / 4096.0f;

    acc16 = (res[2] << 6) | (res[3] >> 2);
    if (acc16 > UINT14_MAX/2)
        acc16 -= UINT14_MAX;
    t[1] = ((float)acc16) / 4096.0f;

    acc16 = (res[4] << 6) | (res[5] >> 2);
    if (acc16 > UINT14_MAX/2)
        acc16 -= UINT14_MAX;
    t[2] = ((float)acc16) / 4096.0f;
    pc.printf("ACC: %1.4f\r\n",  t[1] * t[1]);
    return t[1] * t[1];
}
void loadSignal(void)
{
  green_led = 0;
  int i = 0;
  int n = 0;
  int serial_count = 0;
  audio.spk.pause();
  //pc.printf("%d\n", song_num);
  for (n = 0; n < 3; n++) {
    i = 0;
    while(i < signal_Length)
    {
      if(pc.readable()) {
          serial_buffer[serial_count] = pc.getc();
          serial_count++;
          if(serial_count == 5) {
              serial_buffer[serial_count] = '\0';
              song[n][i] = 1000 * (float)atof(serial_buffer);
              serial_count = 0;
              i++;
          }
      }
    }
  }

  for (n = 0; n < 3; n++) {
    i = 0;
    while(i < signal_Length)
    {
      if(pc.readable()) {
        serial_buffer[serial_count] = pc.getc();
        serial_count++;
        if(serial_count == 5) {
          serial_buffer[serial_count] = '\0';
          note_Length[n][i] = 1000 * (float)atof(serial_buffer);
          serial_count = 0;
          i++;
        }
      }
    }
  }
  green_led = 1;
}
void PlayNote(int freq)
{
    for(int i = 0; i < kAudioTxBufferSize; i++)
    {
        waveform[i] = (int16_t) (sin((double)i * 2. * M_PI/(double) (kAudioSampleFrequency / freq)) * ((1<<16) - 1));
    }
    audio.spk.play(waveform, kAudioTxBufferSize);
}
void PlaySong(int index)
{
    for(int i = 0; i < NUM_OF_FREQUNCYS; i++)
    {
        if(stop_music == 0) break;
        int length = note_Length[this_song][i];
        while(length--)
        {
            // the loop below will play the note for the duration of 1s
            for(int j = 0; j < 16; ++j)
            {
                queue2.call(PlayNote, song[index][i]);
            }
            float acc = ReadAcc();
            int my_beat=-1;
            if(mode==3){
            uLCD.locate(0, 3);
            uLCD.printf("%d\n", beat[i]);
            uLCD.locate(0, 5);
            uLCD.printf("%d\n", score);
            }
            if(length < 1) wait(1.);

            if(mode==3){
            if(0.5<acc){
                 my_beat = 1;
            }else if(acc < 0.5){
                my_beat = 0;
            }

            if(my_beat == beat[i]) score++;
            }
        }
    }
}
void sett()
{   
   if(mode == 0) {
        stop_music = 0;
        mode = 1;
        mode_iter = 0;
    }
    else if(mode == 1) {
        stop_dnn = 1;
        if(mode_iter == 0) {
            mode = 0;
            this_song--;
            if(this_song < 0) this_song = 3 - 1;
        }
        else if(mode_iter == 1) {
            mode = 0;
            this_song++;
            this_song %= 3;
        }
        else if(mode_iter == 2) {
            mode = 2;
            choose_iter = 0;
        }else{
            mode = 3;
        }

    }
    else if(mode == 2) {
        this_song = choose_iter;
        mode = 0;
        stop_dnn = 1;
        mode = 0;
    }else if(mode==3){
      stop_music = 0;
      mode=1;
    }
}
void dnn_main() {
    // error_reporter->Report("Set up successful...\n");
    while (!stop_dnn) {
        // Attempt to read new data from the accelerometer
        got_data = ReadAccelerometer(error_reporter, model_input->data.f,
                                    input_length, should_clear_buffer);

        // If there was no new data,
        // don't try to clear the buffer again and wait until next time
        if (!got_data) {
            should_clear_buffer = false;
            continue;
        }

        // Run inference, and report any error
        TfLiteStatus invoke_status = interpreter->Invoke();
        if (invoke_status != kTfLiteOk) {
            error_reporter->Report("Invoke failed on index: %d\n", begin_index);
            continue;
        }

        // Analyze the results to obtain a prediction
        gesture_index = PredictGesture(interpreter->output(0)->data.f);

        // Clear the buffer next time we read data
        should_clear_buffer = gesture_index < label_num;

        // Produce an output
        if (gesture_index < label_num) {
            error_reporter->Report(config.output_message[gesture_index]);
            if(mode == 1) {
                if(gesture_index == 0) {
                    mode_iter++;
                    mode_iter %= NUM_OF_MODES;
                    break;
                }
            }
            if(mode == 2) {
                if(gesture_index == 0) {
                    choose_iter++;
                    choose_iter %= NUM_OF_SONGS;
                    break;
                }
            }
        }
    }
}
void change() {
    this_song = 0;
    while(true){
    if(mode == 0) {
        audio.spk.play();
            uLCD.locate(0, 0);
            uLCD.cls();
            uLCD.printf("Song number: %d \n",this_song); 
            uLCD.locate(0, 2);
            PlaySong(this_song);
            if(stop_music == 1) {
                this_song++;
                this_song %= 3;
            }
            stop_music = 1;
        }
    else if(mode == 1) {
        audio.spk.pause();
            uLCD.locate(0, 0);
            uLCD.cls();
            uLCD.printf("0.forward\n");
            uLCD.printf("1.backward\n");
            uLCD.printf("2.choose Song\n");
            uLCD.printf("3.play game\n");
            uLCD.printf("choose: %d",mode_iter);
            dnn_main();
            stop_dnn = 0;
       }
    else if(mode == 2) {
        audio.spk.pause();
            uLCD.locate(0, 0);
            uLCD.cls();
            uLCD.printf("song number: 0-1-2\n");
            uLCD.printf("choose: %d\n",choose_iter);
            dnn_main();
            stop_dnn = 0;
           
        }
    else if(mode==3){
        audio.spk.play();
            score = 0;
            uLCD.locate(0, 0);
            uLCD.cls();
            uLCD.printf("Song nunmber: %d\n",this_song); 
            uLCD.locate(0, 2);
            uLCD.printf("Beat : \n");
            uLCD.locate(0, 4);
            uLCD.printf("Score : \n");
            PlaySong(this_song);
            if(stop_music == 1) {
                this_song++;
                this_song %= 3;
            }
            stop_music = 1;
    }
  }
}
int main(void)
{
    thread2.start(callback(&queue2, &EventQueue::dispatch_forever));
    thread.start(callback(&queue, &EventQueue::dispatch_forever));
    sw2.fall(sett);
    mode =0;
    this_song = 0;
    // set up DNN
    const tflite::Model* model = tflite::GetModel(g_magic_wand_model_data);
    if (model->version() != TFLITE_SCHEMA_VERSION) {
        error_reporter->Report(
            "Model provided is schema version %d not equal "
            "to supported version %d.",
            model->version(), TFLITE_SCHEMA_VERSION);
        return -1;
    }
    static tflite::MicroOpResolver<6> micro_op_resolver;
    micro_op_resolver.AddBuiltin(
        tflite::BuiltinOperator_DEPTHWISE_CONV_2D,
        tflite::ops::micro::Register_DEPTHWISE_CONV_2D());
    micro_op_resolver.AddBuiltin(tflite::BuiltinOperator_MAX_POOL_2D,
                                tflite::ops::micro::Register_MAX_POOL_2D());
    micro_op_resolver.AddBuiltin(tflite::BuiltinOperator_CONV_2D,
                                tflite::ops::micro::Register_CONV_2D());
    micro_op_resolver.AddBuiltin(tflite::BuiltinOperator_FULLY_CONNECTED,
                                tflite::ops::micro::Register_FULLY_CONNECTED());
    micro_op_resolver.AddBuiltin(tflite::BuiltinOperator_SOFTMAX,
                                tflite::ops::micro::Register_SOFTMAX());
    micro_op_resolver.AddBuiltin(tflite::BuiltinOperator_RESHAPE,
                                tflite::ops::micro::Register_RESHAPE(), 1);
    static tflite::MicroInterpreter static_interpreter(
        model, micro_op_resolver, tensor_arena, kTensorArenaSize, error_reporter);
    interpreter = &static_interpreter;

    interpreter->AllocateTensors();
    model_input = interpreter->input(0);
    if ((model_input->dims->size != 4) || (model_input->dims->data[0] != 1) ||
        (model_input->dims->data[1] != config.seq_length) ||
        (model_input->dims->data[2] != kChannelNumber) ||
        (model_input->type != kTfLiteFloat32)) {
        error_reporter->Report("Bad input tensor parameters in model");
        return -1;
    }

    input_length = model_input->bytes / sizeof(float);

    TfLiteStatus setup_status = SetupAccelerometer(error_reporter);
    if (setup_status != kTfLiteOk) {
        error_reporter->Report("Set up failed\n");
        return -1;
    }
    // accelerometer
    // Enable the FXOS8700Q
    FXOS8700CQ_ReadRegs( FXOS8700Q_CTRL_REG1, &data[1], 1);
    data[1] |= 0x01;
    data[0] = FXOS8700Q_CTRL_REG1;
    FXOS8700CQ_WriteRegs(data, 2);

    // Get the slave address
    FXOS8700CQ_ReadRegs(FXOS8700Q_WHOAMI, &who_am_i, 1);

    pc.printf("Here is %x\r\n", who_am_i);
    loadSignal();
    queue.call(change);    
}
