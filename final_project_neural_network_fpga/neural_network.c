    #include <stdlib.h>
    #include <stdio.h>
    #include <math.h>
    /* This files provides address values that exist in the system */

    #define SDRAM_BASE            0xC0000000
    #define FPGA_ONCHIP_BASE      0xC8000000
    #define FPGA_CHAR_BASE        0xC9000000

    /* Cyclone V FPGA devices */
    #define LEDR_BASE             0xFF200000
    #define HEX3_HEX0_BASE        0xFF200020
    #define HEX5_HEX4_BASE        0xFF200030
    #define SW_BASE               0xFF200040
    #define KEY_BASE              0xFF200050
    #define TIMER_BASE            0xFF202000
    #define PIXEL_BUF_CTRL_BASE   0xFF203020
    #define CHAR_BUF_CTRL_BASE    0xFF203030
    #define SWITCHES              ((volatile long *)0xFF200040)
    #define PUSHBUTTONS ((volatile long *) 0xFF200050)
    /* VGA colors */
    #define WHITE 0xFFFF
    #define YELLOW 0xFFE0
    #define RED 0xF800
    #define GREEN 0x07E0
    #define BLUE 0x001F
    #define CYAN 0x07FF
    #define MAGENTA 0xF81F
    #define GREY 0xC618
    #define PINK 0xFC18
    #define ORANGE 0xFC00

    #define ABS(x) (((x) > 0) ? (x) : -(x))

    /* Screen size. */
    #define RESOLUTION_X 320
    #define RESOLUTION_Y 240

    /* Constants for animation */
    #define BOX_LEN 2
    #define NUM_BOXES 8

    #define FALSE 0
    #define TRUE 1
    #define MAX(a,b) (((a)>(b))?(a):(b))
    volatile int * pixel_ctrl_ptr = (int *)0xFF203020;
    volatile int * char_ctrl_ptr = (int *)0xFF203030;
    volatile int * stat_reg_ptr = (int *)0xFF20302C;
    volatile int pixel_buffer_start; // global variable
    volatile char char_buffer_start;
    volatile int pixel_buffer_last; // global variable
    void plot_pixel(int x, int y, short int line_color);
    void write_char(int x, int y, char c);

    //patient information

    void draw_box(int x, int y, short int color){
        if (x <=0){
            x = 0;
        }
        if (y <=0){
            y = 0;
        }
        if (x >=RESOLUTION_X){
            x = RESOLUTION_X;
        }
        if (y>=RESOLUTION_Y){
            y = RESOLUTION_Y;
        }
        for (int i = x; i<(x+3); i++){
            for (int j = y; j<(y+3); j++){
                plot_pixel(i, j, color);
            }
        }
    }
    //macros

    double sigmoid(double x) { return 1 / (1 + exp(-x)); }
    double dSigmoid(double x) { return MAX(0, x); }
    double init_weight() { return ((double)rand())/((double)RAND_MAX); }
    void wait_for_vsync(){
        volatile int *pixel_ctrl_ptr = 0xFF203020;
        register int status;
        *pixel_ctrl_ptr = 1;
        status = *(pixel_ctrl_ptr+3);
        while((status & 0x01) !=0){
            status = *(pixel_ctrl_ptr+3);
        }
    }
    unsigned long createRGB(int r, int g, int b)
    {   
        return ((r & 0xff) << 16) + ((g & 0xff) << 8) + (b & 0xff);
    }
    void clear_screen(){
        for (int x = 0; x< 320; x++){
            for (int y = 0; y<240; y++){
                plot_pixel(x, y, 0x0);
            }
        }
    }
    void clear_char(){
        for (int x = 0; x<80; x++){
            for (int y = 0; y<60; y++){
                write_char(x, y, "");
            }
        }
    }
    void write_char(int x, int y, char c) {
    // VGA character buffer
    volatile char * character_buffer = (char *) (FPGA_CHAR_BASE + (y<<7) + x);
    *character_buffer = c;
    }
    void plot_pixel(int x, int y, short int line_color)
    {
        *(short int *)(pixel_buffer_start + (y << 10) + (x << 1)) = line_color;
    }
    void shuffle(int *array, size_t pp)
    {
        if (pp > 1)
        {
            size_t i;
            for (i = 0; i < pp - 1; i++)
            {
                size_t j = i + rand() / (RAND_MAX / (pp - i) + 1);
                int t = array[j];
                array[j] = array[i];
                array[i] = t;
            }
        }
    }
    int correctWrong(int trueInput[], int trueLen, double output[]){
        //printf("trueInput[%d][%d], output[%f][%f]", trueInput[0], trueInput[1], output[0], output[1]);
        int i;
        int correct = 1;
        for (i = 0; i<trueLen; i++){
            if (trueInput[i] == 1){
                if (output[i] > 0.5){
                    correct = 1;
                }
                else{
                    correct = 0;
                    //printf("case 1 pass %d, %d\n", i, correct);
                    return correct;
                }
            }
            else if (trueInput[i] == 0){
                if (output[i] > 0.5){
                    correct = 0;
                    //printf("case 2 %d\n", correct, trueInput[i]);
                    return correct; 
                }
                else{
                    correct = 1;
                }
            }

        }  
        //printf("case 3 %d\n", correct);
        return correct;
        
    }
    //data inputted in the most lazy way possible

    //basic initial parameters. numInputs and numOutputs cannot change due to the nature of the data and the desired outputs
    void draw_line(int x0, int y0, int x1, int y1, short int color){
        int is_steep = (abs(y1-y0) > abs(x1-x0)) ? TRUE:FALSE;
        int temp = 0;
        if (is_steep == TRUE){
            temp = x0;
            x0 = y0;
            y0 = temp;
            temp = x1;
            x1 = y1;
            y1 = temp;
        }
        if (x0 > x1){
            temp = x0;
            x0 = x1;
            x1 = temp;
            temp = y0;
            y0 = y1;
            y1 = temp;
        }
        int deltax = x1 - x0;
        int deltay = abs(y1-y0);
        int error = -(deltax/2);
        int y = y0;
        int y_step = (y0<y1)? 1:-1;
        if (x0<x1){
            for (int x = x0; x<=x1; x++){
                if (is_steep == TRUE){
                plot_pixel(y, x, color);  
                }
                else{
                plot_pixel(x,y, color);
                }
                error = error + deltay;
                if (error > 0){
                    y = y + y_step;
                    error = error - deltax;
                }
            }
        }
        else{
            for (int x = x0; x>=x1; x--){
                if (is_steep == TRUE){
                plot_pixel(y, x, color);  
                }
                else{
                plot_pixel(x,y, color);
                }
                error = error + deltay;
                if (error > 0){
                    y = y + y_step;
                    error = error - deltax;
                }
            }
        }
        
    }
    void printSentence(const char* arr, float number, int x, int y){
        int leng = x;
        if (arr != ""){ 
                while(*arr){
                    write_char(leng, y, *arr);
                    leng++;
                    arr++;
                }
        }

        char result[5];
            sprintf(result, "%f", (double)number);
            for (int i = 0; i<4; i++){
                write_char(leng + 1 + i, y, result[i]);
            }

    }
    int main(){
        //user input to change number of layers, number of epochs, or default
        *(pixel_ctrl_ptr + 1) = 0xC8000000; // first store the address in the 
        // back buffer
        wait_for_vsync();
        /* initialize a pointer to the pixel buffer, used by drawing functions */
        pixel_buffer_start = *pixel_ctrl_ptr;
        char_buffer_start = *char_ctrl_ptr;
        clear_screen();
        clear_char();
        
        long Swval;
        long PBval;
        while(TRUE){
            //set number of layers
            PBval = *PUSHBUTTONS;
            if (*SWITCHES>10){
                Swval = 10;
            }
            else{
                Swval = *SWITCHES;
            }
            char* cntr = "Number of Layers (Max 10): ";
            printSentence(cntr, Swval, 0, 3);
            
            if (PBval == 1){
                break;
            }
        }
        int numLayers = (int)Swval;
        while(TRUE){
            PBval = *PUSHBUTTONS;
            if (PBval == 0){
                break;
            }
        }
        while(TRUE){
            //set percent split of data
            Swval = *SWITCHES;
            PBval = *PUSHBUTTONS;
            char* cntr = "Total Epochs (Cycles): ";
            printSentence(cntr, Swval, 0, 4);
            if (PBval == 1){
                break;
            }
        }
        int epoch = (int)Swval;
        #define numInputs 6
        #define numHiddenNodes numLayers
        #define numOutputs 2
        #define numTrainingSets 119
        int testingSets = (int)(numTrainingSets /2);
        int epochs = epoch;
        int currentTotal = 0;
        double currentAccuracy = 0;
        const double lr = 0.1f;
        double hiddenLayer[numHiddenNodes];
        double outputLayer[numOutputs];
        double hiddenLayerBias[numHiddenNodes];
        double outputLayerBias[numOutputs];
        double hiddenWeights[numInputs][numHiddenNodes];
        double outputWeights[numHiddenNodes][numOutputs];
        int currentTrue[numOutputs];
        for (int i=0; i<numInputs; i++) {
            for (int j=0; j<numHiddenNodes; j++) {
                hiddenWeights[i][j] = init_weight();
            }
        }
        for (int i=0; i<numHiddenNodes; i++) {
            hiddenLayerBias[i] = init_weight();
            for (int j=0; j<numOutputs; j++) {
                outputWeights[i][j] = init_weight();
                outputLayerBias[i] = init_weight();
            }
        }
        /*for (int i=0; i<numOutputs; i++) {
            outputLayerBias[i] = init_weight();
        }*/
        //change how set order is done. not random

        int trainingSetOrder[119];
        for(int i = 0; i < 119; i++)
        {
            trainingSetOrder[i] = i;
        }

        double training_inputs[numInputs][numTrainingSets] = 
    {
    {0.8650602409638554,
    0.8650602409638554,
    0.8674698795180723,
    0.8674698795180723,
    0.8674698795180723,
    0.8722891566265061,
    0.8722891566265061,
    0.8746987951807228,
    0.8819277108433735,
    0.8819277108433735,
    0.8819277108433735,
    0.8819277108433735,
    0.8843373493975905,
    0.8843373493975905,
    0.8843373493975905,
    0.8867469879518072,
    0.8867469879518072,
    0.8891566265060241,
    0.8891566265060241,
    0.891566265060241,
    0.891566265060241,
    0.891566265060241,
    0.891566265060241,
    0.891566265060241,
    0.891566265060241,
    0.891566265060241,
    0.891566265060241,
    0.8939759036144579,
    0.8939759036144579,
    0.8939759036144579,
    0.8963855421686747,
    0.8963855421686747,
    0.8963855421686747,
    0.8987951807228916,
    0.8987951807228916,
    0.8987951807228916,
    0.9012048192771084,
    0.9012048192771084,
    0.9036144578313253,
    0.9036144578313253,
    0.9036144578313253,
    0.9036144578313253,
    0.9036144578313253,
    0.9036144578313253,
    0.9060240963855422,
    0.9060240963855422,
    0.9060240963855422,
    0.9084337349397591,
    0.9084337349397591,
    0.9084337349397591,
    0.9084337349397591,
    0.9108433734939758,
    0.9108433734939758,
    0.9108433734939758,
    0.9132530120481928,
    0.9132530120481928,
    0.9132530120481928,
    0.9132530120481928,
    0.9132530120481928,
    0.9156626506024096,
    0.9156626506024096,
    0.9180722891566265,
    0.9228915662650602,
    0.927710843373494,
    0.9325301204819277,
    0.9373493975903614,
    0.9397590361445783,
    0.9493975903614458,
    0.9566265060240965,
    0.963855421686747,
    0.963855421686747,
    0.963855421686747,
    0.963855421686747,
    0.963855421686747,
    0.963855421686747,
    0.963855421686747,
    0.963855421686747,
    0.9662650602409639,
    0.9686746987951808,
    0.9686746987951808,
    0.9686746987951808,
    0.9710843373493975,
    0.9734939759036144,
    0.9734939759036144,
    0.9734939759036144,
    0.9734939759036144,
    0.9734939759036144,
    0.9759036144578314,
    0.9783132530120482,
    0.9783132530120482,
    0.9783132530120482,
    0.9807228915662651,
    0.9807228915662651,
    0.9807228915662651,
    0.9807228915662651,
    0.9807228915662651,
    0.9831325301204819,
    0.9855421686746988,
    0.9855421686746988,
    0.9855421686746988,
    0.9879518072289156,
    0.9879518072289156,
    0.9879518072289156,
    0.9879518072289156,
    0.9903614457831326,
    0.9903614457831326,
    0.9903614457831326,
    0.9903614457831326,
    0.9903614457831326,
    0.9927710843373495,
    0.9927710843373495,
    0.9927710843373495,
    0.9927710843373495,
    0.9951807228915662,
    0.9975903614457831,
    1.0,
    1.0,
    1.0,
    1.0},
    {0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    1,
    1,
    1,
    0,
    0,
    1,
    1,
    0,
    1,
    1,
    0,
    1,
    0,
    1,
    1,
    1,
    0,
    1,
    1,
    1,
    0,
    1,
    1,
    1,
    0,
    1,
    0,
    0,
    1,
    1,
    0,
    1,
    0,
    1,
    0,
    1,
    1,
    0,
    1,
    0,
    1,
    0,
    1,
    0,
    1,
    0,
    0,
    1,
    0,
    0
    },
    {0,
    1,
    0,
    1,
    1,
    0,
    1,
    0,
    0,
    0,
    1,
    1,
    0,
    1,
    1,
    0,
    0,
    0,
    1,
    0,
    0,
    1,
    0,
    0,
    0,
    0,
    0,
    1,
    0,
    0,
    0,
    1,
    0,
    1,
    0,
    0,
    1,
    0,
    0,
    1,
    1,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    1,
    0,
    1,
    0,
    0,
    0,
    0,
    1,
    0,
    0,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    0,
    0,
    1,
    1,
    1,
    1,
    1,
    0,
    1,
    1,
    1,
    1,
    1,
    0,
    1,
    1,
    1,
    0,
    1,
    1,
    1,
    0,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    0,
    1,
    1,
    1,
    1,
    0,
    1,
    1,
    1,
    0,
    1,
    1,
    1,
    1,
    0,
    1,
    1,
    1
    },
    {1,
    0,
    1,
    0,
    0,
    1,
    0,
    1,
    1,
    1,
    0,
    0,
    1,
    0,
    0,
    1,
    1,
    1,
    0,
    1,
    1,
    0,
    1,
    1,
    1,
    1,
    1,
    0,
    1,
    1,
    1,
    0,
    1,
    0,
    1,
    1,
    0,
    1,
    1,
    0,
    0,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    0,
    1,
    0,
    1,
    1,
    1,
    1,
    0,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    0,
    0,
    0,
    0,
    1,
    1,
    1,
    0,
    0,
    1,
    1,
    1,
    1,
    0,
    0,
    1,
    1,
    0,
    0,
    1,
    1,
    0,
    0,
    1,
    1,
    1,
    1,
    1,
    1,
    0,
    0,
    1,
    1,
    1,
    0,
    0,
    1,
    1,
    0,
    0,
    1,
    1,
    1,
    0,
    0,
    1,
    1,
    },
    {1,
    0,
    1,
    0,
    0,
    1,
    0,
    1,
    1,
    1,
    0,
    0,
    1,
    0,
    0,
    1,
    1,
    1,
    0,
    1,
    1,
    0,
    1,
    1,
    1,
    1,
    0,
    0,
    1,
    0,
    1,
    0,
    0,
    0,
    1,
    0,
    0,
    0,
    1,
    0,
    0,
    1,
    0,
    0,
    1,
    1,
    1,
    1,
    1,
    0,
    0,
    0,
    1,
    0,
    1,
    1,
    0,
    1,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    1,
    1,
    1,
    0,
    0,
    1,
    1,
    0,
    1,
    1,
    0,
    1,
    0,
    1,
    1,
    1,
    0,
    1,
    1,
    1,
    0,
    1,
    1,
    1,
    0,
    1,
    0,
    0,
    1,
    1,
    0,
    1,
    0,
    1,
    0,
    1,
    1,
    0,
    1,
    0,
    1,
    0,
    1,
    0,
    1,
    0,
    0,
    1,
    0,
    0
    },
    {1,
    0,
    1,
    0,
    0,
    1,
    0,
    1,
    1,
    1,
    0,
    0,
    1,
    0,
    0,
    1,
    1,
    1,
    0,
    0,
    0,
    0,
    1,
    1,
    1,
    1,
    0,
    0,
    1,
    0,
    0,
    0,
    0,
    0,
    1,
    0,
    0,
    0,
    0,
    0,
    0,
    1,
    0,
    0,
    0,
    0,
    1,
    0,
    0,
    0,
    0,
    0,
    1,
    0,
    0,
    0,
    0,
    1,
    0,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    0,
    0,
    0,
    0,
    0,
    1,
    0,
    1,
    0,
    0,
    1,
    1,
    0,
    0,
    0,
    0,
    0,
    1,
    0,
    0,
    1,
    0,
    0,
    0,
    1,
    1,
    0,
    0,
    1,
    1,
    0,
    0,
    1,
    1,
    0,
    0,
    0,
    1,
    1,
    0,
    0,
    1,
    0,
    1,
    0,
    0,
    1,
    1
    }};
        int training_outputs[numOutputs][numTrainingSets] = 
    {
    {1,
    0,
    1,
    0,
    0,
    1,
    0,
    1,
    1,
    1,
    0,
    0,
    1,
    0,
    0,
    1,
    1,
    1,
    0,
    1,
    1,
    0,
    1,
    1,
    1,
    1,
    1,
    0,
    1,
    1,
    1,
    0,
    1,
    0,
    1,
    1,
    0,
    1,
    1,
    0,
    0,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    0,
    1,
    0,
    1,
    1,
    1,
    1,
    0,
    1,
    1,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    1,
    1,
    1,
    0,
    0,
    0,
    0,
    0,
    1,
    1,
    0,
    0,
    0,
    1,
    1,
    1,
    0,
    0,
    1,
    1,
    0,
    0,
    1,
    1,
    0,
    0,
    0,
    0,
    1,
    1,
    0,
    1,
    0,
    0,
    0,
    1,
    1,
    0,
    0,
    0,
    1,
    0,
    0,
    0,
    1,
    0,
    0,
    0,
    0,
    0
    },
    {0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    0,
    0,
    1,
    1,
    1,
    1,
    1,
    0,
    1,
    1,
    1,
    1,
    1,
    0,
    1,
    1,
    1,
    0,
    1,
    1,
    1,
    0,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    0,
    1,
    1,
    1,
    1,
    0,
    1,
    1,
    1,
    0,
    1,
    1,
    1,
    1,
    0,
    1,
    1,
    1}};
        for (int i = 0; i<numInputs; i++){
            draw_box(10, 50 + i*30, WHITE);
        }
        for (int i = 0; i<numLayers; i++){
            for (int j = 0; j<numInputs; j++){
                if (i>0){
                    for (int k = 0; k<numInputs; k++){
                        draw_line(20 + (i-1)*30, 50 + j*30, 20 +i*30, 50 + k*30,  BLUE);
                    }
                }
                draw_box(20 + i*30, 50 + j*30, WHITE);
                
            }
        }
        for (int i = 0; i<numOutputs; i++){
            draw_box(10 + 30*(numLayers), 50 + (int)(i * (30*numInputs)/numOutputs), WHITE);
        }

        for (int n=0; n < epochs; n++) {
            for (int i = 0; i<numLayers; i++){
                for (int j = 0; j<numInputs; j++){
                    if (i>0){
                        for (int k = 0; k<numInputs; k++){
                            int divisor = (int)hiddenWeights[i][j];
                            char result[5];
                            sprintf(result, "%f", hiddenWeights[i][j]);
                            for (int i = 0; i<4; i++){
                                write_char(0 + 1 + i, 40, result[i]);
                            }
                            draw_line(20 + (i-1)*30, 50 + j*30, 20 +i*30, 50 + k*30,  createRGB(255/divisor, 255/divisor, 255/divisor)); //fix this for the sake of all that is holy
                        }
                    }
                }
            }
            char* centre = "Epoch: ";
            printSentence(centre, (double)n, 0, 6);
            shuffle(trainingSetOrder,numTrainingSets);
            for (int x=0; x<testingSets; x++) {
                
                int i = trainingSetOrder[x];
                
                // Forward pass
                
                for (int j=0; j<numHiddenNodes; j++) {
                    double activation=hiddenLayerBias[j];
                    for (int k=0; k<numInputs; k++) {
                        activation+=training_inputs[k][i]*hiddenWeights[k][j];
                    }
                    hiddenLayer[j] = sigmoid(activation);
                }
                
                for (int j=0; j<numOutputs; j++) {
                    double activation=outputLayerBias[j];
                    for (int k=0; k<numHiddenNodes; k++) {
                        activation+=hiddenLayer[k]*outputWeights[k][j];
                    }
                    outputLayer[j] = sigmoid(activation);
                }
                for (int j = 0; j<numOutputs; j++){
                    currentTrue[j] = training_outputs[j][i];
                }
                        
            // Backprop
                double deltaOutput[numOutputs];
                for (int j=0; j<numOutputs; j++) {
                    double errorOutput = (training_outputs[j][i]-outputLayer[j]);
                    deltaOutput[j] = errorOutput*dSigmoid(outputLayer[j]);
                }
                
                double deltaHidden[numHiddenNodes];
                for (int j=0; j<numHiddenNodes; j++) {
                    double errorHidden = 0.0f;
                    for(int k=0; k<numOutputs; k++) {
                        errorHidden+=deltaOutput[k]*outputWeights[j][k];
                    }
                    deltaHidden[j] = errorHidden*dSigmoid(hiddenLayer[j]);
                }
                
                for (int j=0; j<numOutputs; j++) {
                    outputLayerBias[j] += deltaOutput[j]*lr;
                    for (int k=0; k<numHiddenNodes; k++) {
                        outputWeights[k][j]+=hiddenLayer[k]*deltaOutput[j]*lr;
                    }
                }
                
                for (int j=0; j<numHiddenNodes; j++) {
                    hiddenLayerBias[j] += deltaHidden[j]*lr;
                    for(int k=0; k<numInputs; k++) {
                        hiddenWeights[k][j]+=training_inputs[k][i]*deltaHidden[j]*lr;
                    }
                }
                
                currentTotal += correctWrong(currentTrue, numOutputs, outputLayer);
                currentAccuracy = (double)currentTotal/(double)x;
            }
            double leftBuffer = 105*currentAccuracy + 105;
            char* cntr = "Accuracy: ";
            
            printSentence(cntr, (currentAccuracy * 100), 0, 0);
            // for (int i = 104; i<209; i++){
            //     plot_pixel(i, 219, WHITE);
            // }
            // for (int i = 104; i<209; i++){
            //     plot_pixel(i, 224, WHITE);
                
            // }
            // for (int j = 219; j<225; j++){
            //     plot_pixel(104, j, WHITE);
            //     plot_pixel(209, j, WHITE);
            // }
            for (int i = 105; i<210; i++){
                for (int j = 220; j<224; j++){
                    if (i<leftBuffer){
                        plot_pixel(i, j, GREEN);
                    }
                    else{
                        plot_pixel(i, j, RED);
                    }
                }
            }
            //220 224
            double epochDone = (double)n/(double)epochs;
            //printf("current epoch done = %f\n", epochDone);
            double leftBufferNew = 105*epochDone + 105;
            for (int i = 105; i<208; i++){
                for (int j = 215; j<218; j++){
                    if (i<leftBufferNew){
                        plot_pixel(i, j, GREEN);
                    }
                    else{
                        plot_pixel(i, j, RED);
                    }
                }
            }
            // int leng = 9;
            // cntr = "Accuracy: ";
            //         while(*cntr){
            //             write_char(leng, 55, *cntr);
            //             leng++;
            //             cntr++;
            //         }
            



            currentTotal = 0;
            
        }
        int totalCorrect = 0;
        //(int)Swval/10
        for (int x=testingSets; x<numTrainingSets; x++) { 
                
            int i = trainingSetOrder[x];
            for (int j=0; j<numHiddenNodes; j++) {
                double activation=hiddenLayerBias[j];
                for (int k=0; k<numInputs; k++) {
                    activation+=training_inputs[k][i]*hiddenWeights[k][j];
                }
                hiddenLayer[j] = sigmoid(activation);
            }
            for (int j=0; j<numOutputs; j++) {
                double activation=outputLayerBias[j];
                for (int k=0; k<numHiddenNodes; k++) {
                    activation+=hiddenLayer[k]*outputWeights[k][j];
                }
                outputLayer[j] = sigmoid(activation);
            }
            
            for (int j = 0; j<numOutputs; j++){
                currentTrue[j] = training_outputs[j][i];
            }
            totalCorrect += correctWrong(currentTrue, numOutputs, outputLayer);
        }
        testingSets = numTrainingSets - testingSets;
        double accuracy = (double)totalCorrect/(double)testingSets * 100;
        char* cntr = "Testing Accuracy: ";
        int leng = 0;

        while(*cntr){
            write_char(leng, 0, *cntr);
            leng++;
            cntr++;
        }
        char result[5];
        sprintf(result, "%f", accuracy);
        for (int i = 0; i<4; i++){
            write_char(leng + 1 + i, 0, result[i]);
        }

        clear_screen();
        char *arr = "Temperature; Nausea(Y/N); Lumbar Pain(Y/N); Urine(Y/N); Micturation(Y/N); Burning(Y/N): ";
        printSentence(arr, 0, 0, 8);
        for(int i = 0; i<20; i++){
            printSentence("", i, 0, 10 + i);
            for(int j = 0; j<6; j++){
                wait_for_vsync();
                float input = j==0?  training_inputs[j][i] * 41.5 : training_inputs[j][i];
                printSentence("", input, 5 + j*5, 10 + i);

            }
        }
    while(TRUE){
        int index = 2;
        PBval = *PUSHBUTTONS;
        // while(TRUE){
        //     //set number of layers
        //     PBval = *PUSHBUTTONS;
        //     if (*SWITCHES>19){
        //         Swval = 19;
        //     }
        //     else{
        //         Swval = *SWITCHES;
        //     }
        //     char* cntr = "Pick a test case: ";
        //     printSentence(cntr, Swval, 0, 31);
        //     if (PBval == 1){
        //         index = (int)Swval;
        //         break;
        //     }
        // }
        for (int j=0; j<numHiddenNodes; j++) {
            double activation=hiddenLayerBias[j];
                for (int k=0; k<numInputs; k++) {
                activation+=training_inputs[k][index]*hiddenWeights[k][j];
            }
            hiddenLayer[j] = sigmoid(activation);
        }
        for (int j=0; j<numOutputs; j++) {
            double activation=outputLayerBias[j];
            for (int k=0; k<numHiddenNodes; k++) {
                activation+=hiddenLayer[k]*outputWeights[k][j];
            }
            outputLayer[j] = sigmoid(activation);
        }
        
        for (int j = 0; j<numOutputs; j++){
            currentTrue[j] = training_outputs[j][index];
        }
        for (int i = 0; i<numOutputs; i++){
            if (i == 0){
                printSentence("output activation 1: ", outputLayer[i], 0, 50);
                printSentence("actual answer 1: ", training_outputs[i][index], 0, 51);
            }
            if (i == 1){
                printSentence("output activation 2: ", outputLayer[i], 0, 52);
                printSentence("actual answer 2: ", training_outputs[i][index], 0, 53);
            }
        }
        if (PBval == 2){
            break;
        }
    }

    //print output activation, then real activation, then see if right or wrong)
        return 0;
        
    }
